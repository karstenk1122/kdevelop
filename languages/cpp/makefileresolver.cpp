/*
* KDevelop C++ Language Support
*
* Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/**
 * Compatibility:
 * make/automake: Should work perfectly
 * cmake: Thanks to the path-recursion, this works with cmake(tested with version "2.4-patch 6"
 *        with kdelibs out-of-source and with kdevelop4 in-source)
 *
 * unsermake:
 *   unsermake is detected by reading the first line of the makefile. If it contains
 *   "generated by unsermake" the following things are respected:
 *   1. Since unsermake does not have the -W command (which should tell it to recompile
 *      the given file no matter whether it has been changed or not), the file-modification-time of
 *      the file is changed temporarily and the --no-real-compare option is used to force recompilation.
 *   2. The targets seem to be called *.lo instead of *.o when using unsermake, so *.lo names are used.
 *   example-(test)command: unsermake --no-real-compare -n myfile.lo
 **/

#include "makefileresolver.h"

#include <memory>
#include <cstdio>
#include <iostream>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#ifdef TEST
#include <QApplication>
#endif
#include <QDir>
#include <QFileInfo>
#include <QRegExp>

#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>

#include <language/duchain/indexedstring.h>
#include <util/pushvalue.h>

//#define VERBOSE

#if defined(TEST) || defined(VERBOSE)
#define ifTest(x) x
#else
#define ifTest(x)
#endif

const int maximumInternalResolutionDepth = 3;

using namespace std;
using namespace KDevelop;

namespace {
  ///After how many seconds should we retry?
  static const int CACHE_FAIL_FOR_SECONDS = 200;

  static const int processTimeoutSeconds = 30;

  struct CacheEntry
  {
    CacheEntry()
      : failed(false)
    { }
    ModificationRevisionSet modificationTime;
    QStringList paths;
    QString errorMessage, longErrorMessage;
    bool failed;
    QMap<QString,bool> failedFiles;
    QDateTime failTime;
  };
  typedef QMap<QString, CacheEntry> Cache;

  static Cache s_cache;
  static QMutex s_cacheMutex;
}



namespace CppTools {

  ///Helper-class used to fake file-modification times
  class FileModificationTimeWrapper
  {
  public:
    ///@param files list of files that should be fake-modified(modtime will be set to current time)
    explicit FileModificationTimeWrapper(const QStringList& files = QStringList(), const QString& workingDirectory = QString())
      : m_newTime(time(0))
    {
      for (QStringList::const_iterator it = files.constBegin(); it != files.constEnd(); ++it) {
        ifTest(cout << "touching " << it->toUtf8().constData() << endl);

        QFileInfo fileinfo(QDir(workingDirectory), *it);
        if (!fileinfo.exists()) {
          cout << "File does not exist: " << it->toUtf8().constData()
               << "in working dir " << QDir::currentPath().toUtf8().constData() << "\n";
          continue;
        }

        QString filename = fileinfo.canonicalFilePath();
        if (m_stat.find(filename) != m_stat.end()) {
          cout << "Duplicate file: " << filename.toUtf8().constData() << endl;
          continue;
        }

        QByteArray bFileName = filename.toLocal8Bit();
        const char* bFileNameC = bFileName.constData();

        struct stat s;
        if (stat(bFileNameC, &s) == 0) {
          ///Success
          m_stat[filename] = s.st_mtime;
          ///change the modification-time to m_newTime
          struct timeval times[2];
          times[0].tv_sec = m_newTime;
          times[0].tv_usec = 0;
          times[1].tv_sec = m_newTime;
          times[1].tv_usec = 0;

          if (utimes(bFileNameC, times) != 0) {
            ifTest(cout << "failed to touch " << it->toUtf8().constData() << endl);
          }
        }
      }
    }

    ///Undo changed modification-times
    void unModify()
    {
      for (QHash<QString, time_t>::const_iterator it = m_stat.constBegin(); it != m_stat.constEnd(); ++it) {
        ifTest(cout << "untouching " << it.key().toUtf8().constData() << endl);

        QByteArray bFileName = it.key().toLocal8Bit();
        const char* bFileNameC = bFileName.constData();

        struct stat s;
        if (stat(bFileNameC, &s) == 0) {
          if (s.st_mtime == m_newTime) {
            ///Still the modtime that we've set, change it back
            struct timeval times[2];
            times[0].tv_usec = 0;
            times[0].tv_sec = s.st_atime;
            times[1].tv_usec = 0;
            times[1].tv_sec = *it;
            if (utimes(bFileNameC, times) != 0) {
              perror("Resetting modification time");
              ifTest(cout << "failed to untouch " << it.key().toUtf8().constData() << endl);
            }
          } else {
            ///The file was modified since we changed the modtime
            ifTest(cout << "will not untouch " << it.key().toUtf8().constData() << " because the modification-time has changed" << endl);
          }
        } else {
          perror("File status");
        }
      }
    }

    ~FileModificationTimeWrapper()
    {
      unModify();
    }

  private:
    QHash<QString, time_t>  m_stat;
    time_t                  m_newTime;
  };

  class SourcePathInformation
  {
  public:
    SourcePathInformation(const QString& path)
      : m_path(path)
      , m_isUnsermake(false)
      , m_shouldTouchFiles(false)
    {
      m_isUnsermake = isUnsermakePrivate(path);

      ifTest(if (m_isUnsermake) cout << "unsermake detected" << endl);
    }

    bool isUnsermake() const
    {
      return m_isUnsermake;
    }

    ///When this is set, the file-modification times are changed no matter whether it is unsermake or make
    void setShouldTouchFiles(bool b)
    {
      m_shouldTouchFiles = b;
    }

    QString getCommand(const QString& absoluteFile, const QString& workingDirectory, const QString& makeParameters) const
    {
      if (isUnsermake()) {
        return "unsermake -k --no-real-compare -n " + makeParameters;
      } else {
        QString relativeFile = KUrl::relativePath(workingDirectory, absoluteFile);
        return "make -k --no-print-directory -W \'" + absoluteFile + "\' -W \'" + relativeFile + "\' -n " + makeParameters;
      }
    }

    bool hasMakefile() const
    {
        QFileInfo makeFile(m_path, "Makefile");
        return makeFile.exists();
    }

    bool shouldTouchFiles() const
    {
      return isUnsermake() || m_shouldTouchFiles;
    }

    QStringList possibleTargets(const QString& targetBaseName) const
    {
      QStringList ret;
      ///@todo open the make-file, and read the target-names from there.
      if (isUnsermake()) {
        //unsermake breaks if the first given target does not exist, so in worst-case 2 calls are necessary
        ret << targetBaseName + ".lo";
        ret << targetBaseName + ".o";
      } else {
        //It would be nice if both targets could be processed in one call, the problem is the exit-status of make, so for now make has to be called twice.
        ret << targetBaseName + ".o";
        ret << targetBaseName + ".lo";
        //ret << targetBaseName + ".lo " + targetBaseName + ".o";
      }
      ret << targetBaseName + ".ko";
      return ret;
    }

  private:
      bool isUnsermakePrivate(const QString& path)
      {
        bool ret = false;
        QFileInfo makeFile(path, "Makefile");
        QFile f(makeFile.absoluteFilePath());
        if (f.open(QIODevice::ReadOnly)) {
          QString firstLine = f.readLine(128);
          if (firstLine.indexOf("generated by unsermake") != -1) {
            ret = true;
          }
          f.close();
        }
        return ret;
      }

      QString m_path;
      bool m_isUnsermake;
      bool m_shouldTouchFiles;
  };

void PathResolutionResult::addPathsUnique(const PathResolutionResult& rhs)
{
    foreach(const QString& path, rhs.paths) {
        if(!paths.contains(path))
            paths.append(path);
    }
    includePathDependency += rhs.includePathDependency;
}

PathResolutionResult::PathResolutionResult(bool success, const QString& errorMessage, const QString& longErrorMessage)
    : success(success)
    , errorMessage(errorMessage)
    , longErrorMessage(longErrorMessage)
{}

PathResolutionResult::operator bool() const
{
    return success;
}
}

using namespace CppTools;

ModificationRevisionSet MakeFileResolver::findIncludePathDependency(const QString& file)
{
  QString oldSourceDir = m_source;
  QString oldBuildDir = m_build;

  KUrl currentWd = mapToBuild(KUrl(file));

  ModificationRevisionSet rev;
  while (!currentWd.path().isEmpty()) {
    if (currentWd == currentWd.upUrl())
      break;

    currentWd = currentWd.upUrl();
    QString path = currentWd.toLocalFile();
    QFileInfo makeFile(QDir(path), "Makefile");
    if (makeFile.exists()) {
      IndexedString makeFileStr(makeFile.filePath());
      rev.addModificationRevision(makeFileStr, ModificationRevision::revisionForFile(makeFileStr));
      break;
    }
  }

  setOutOfSourceBuildSystem(oldSourceDir, oldBuildDir);

  return rev;
}

bool MakeFileResolver::executeCommand(const QString& command, const QString& workingDirectory, QString& result) const
{
  ifTest(cout << "executing " << command.toUtf8().constData() << endl);
  ifTest(cout << "in " << workingDirectory.toUtf8().constData() << endl);

  KProcess proc;
  proc.setWorkingDirectory(workingDirectory);
  proc.setOutputChannelMode(KProcess::MergedChannels);

  QStringList args(command.split(' '));
  QString prog = args.takeFirst();
  proc.setProgram(prog, args);

  int status = proc.execute(processTimeoutSeconds * 1000);
  result = proc.readAll();

  return status == 0;
}

MakeFileResolver::MakeFileResolver()
  : m_isResolving(false)
  , m_outOfSource(false)
{
}

///More efficient solution: Only do exactly one call for each directory. During that call, mark all source-files as changed, and make all targets for those files.
PathResolutionResult MakeFileResolver::resolveIncludePath(const QString& file)
{
  if (file.isEmpty()) {
    // for unit tests with temporary files
    return PathResolutionResult();
  }
  QFileInfo fi(file);
  return resolveIncludePath(fi.fileName(), fi.absolutePath());
}

KUrl MakeFileResolver::mapToBuild(const KUrl& url) const
{
  KUrl wdUrl = url;
  wdUrl.cleanPath();
  QString wd = wdUrl.toLocalFile(KUrl::RemoveTrailingSlash);
  if (m_outOfSource) {
    if (wd.startsWith(m_source) && !wd.startsWith(m_build)) {
        //Move the current working-directory out of source, into the build-system
        wd = m_build + '/' + wd.mid(m_source.length());
        KUrl u(wd);
        u.cleanPath();
        wd = u.toLocalFile();
      }
  }
  return KUrl(wd);
}

void CppTools::MakeFileResolver::clearCache()
{
  QMutexLocker l(&s_cacheMutex);
  s_cache.clear();
}

PathResolutionResult MakeFileResolver::resolveIncludePath(const QString& file, const QString& _workingDirectory, int maxStepsUp)
{
  //Prefer this result when returning a "fail". The include-paths of this result will always be added.
  PathResolutionResult resultOnFail;

  if (m_isResolving)
    return PathResolutionResult(false, i18n("Tried include path resolution while another resolution process was still running"));

  //Make the working-directory absolute
  QString workingDirectory = _workingDirectory;

  if (KUrl(workingDirectory).isRelative()) {
    KUrl u(QDir::currentPath());

    if (workingDirectory == ".")
      workingDirectory = QString();
    else if (workingDirectory.startsWith("./"))
      workingDirectory = workingDirectory.mid(2);

    if (!workingDirectory.isEmpty())
      u.addPath(workingDirectory);
    workingDirectory = u.toLocalFile();
  } else
    workingDirectory = _workingDirectory;

  ifTest(cout << "working-directory: " <<  workingDirectory.toLocal8Bit().data() << "  file: " << file.toLocal8Bit().data() << std::endl;)

  QDir sourceDir(workingDirectory);
  QDir dir = QDir(mapToBuild(sourceDir.absolutePath()).toLocalFile());

  QFileInfo makeFile(dir, "Makefile");
  if (!makeFile.exists()) {
    if (maxStepsUp > 0) {
      //If there is no makefile in this directory, go one up and re-try from there
      QFileInfo fileName(file);
      QString localName = sourceDir.dirName();

      if (sourceDir.cdUp() && !fileName.isAbsolute()) {
        QString checkFor = localName + "/" + file;
        PathResolutionResult oneUp = resolveIncludePath(checkFor, sourceDir.path(), maxStepsUp-1);
        if (oneUp.success) {
          oneUp.addPathsUnique(resultOnFail);
          return oneUp;
        }
      }
    }

    if (!resultOnFail.errorMessage.isEmpty() || !resultOnFail.paths.isEmpty())
      return resultOnFail;
    else
      return PathResolutionResult(false, i18n("Makefile is missing in folder \"%1\"", dir.absolutePath()), i18n("Problem while trying to resolve include paths for %1", file));
  }

  PushValue<bool> e(m_isResolving, true);

  QStringList cachedPaths; //If the call doesn't succeed, use the cached not up-to-date version
  ModificationRevisionSet dependency;
  dependency.addModificationRevision(IndexedString(makeFile.filePath()), ModificationRevision::revisionForFile(IndexedString(makeFile.filePath())));
  dependency += resultOnFail.includePathDependency;
  Cache::iterator it;
  {
    QMutexLocker l(&s_cacheMutex);
    it = s_cache.find(dir.path());
    if (it != s_cache.end()) {
      cachedPaths = (*it).paths;
      if (dependency == (*it).modificationTime) {
        if (!(*it).failed) {
          //We have a valid cached result
          PathResolutionResult ret(true);
          ret.paths = (*it).paths;
          ret.addPathsUnique(resultOnFail);
          return ret;
        } else {
          //We have a cached failed result. We should use that for some time but then try again. Return the failed result if: (there were too many tries within this folder OR this file was already tried) AND The last tries have not expired yet
          if (/*((*it).failedFiles.size() > 3 || (*it).failedFiles.find(file) != (*it).failedFiles.end()) &&*/ (*it).failTime.secsTo(QDateTime::currentDateTime()) < CACHE_FAIL_FOR_SECONDS) {
            PathResolutionResult ret(false); //Fake that the result is ok
            ret.errorMessage = i18n("Cached: %1", (*it).errorMessage);
            ret.longErrorMessage = (*it).longErrorMessage;
            ret.paths = (*it).paths;
            ret.addPathsUnique(resultOnFail);
            return ret;
          } else {
            //Try getting a correct result again
          }
        }
      }
    }
  }

  ///STEP 1: Prepare paths
  QString targetName;
  QFileInfo fi(file);

  QString absoluteFile = file;
  if (KUrl(file).isRelative())
    absoluteFile = workingDirectory + '/' + file;
  KUrl u(absoluteFile);
  u.cleanPath();
  absoluteFile = u.toLocalFile();

  int dot;
  if ((dot = file.lastIndexOf('.')) == -1) {
    if (!resultOnFail.errorMessage.isEmpty() || !resultOnFail.paths.isEmpty())
      return resultOnFail;
    else
      return PathResolutionResult(false, i18n("Filename %1 seems to be malformed", file));
  }

  targetName = file.left(dot);

  QString wd = dir.path();
  if (KUrl(wd).isRelative()) {
    wd = QDir::currentPath() + '/' + wd;
    KUrl u(wd);
    u.cleanPath();
    wd = u.toLocalFile();
  }

  wd = mapToBuild(wd).toLocalFile();

  SourcePathInformation source(wd);
  QStringList possibleTargets = source.possibleTargets(targetName);

  source.setShouldTouchFiles(true); //Think about whether this should be always enabled. I've enabled it for now so there's an even bigger chance that everything works.

  ///STEP 3: Try resolving the paths, by using once the absolute and once the relative file-path. Which kind is required differs from setup to setup.

  ///STEP 3.1: Try resolution using the absolute path
  PathResolutionResult res;
  //Try for each possible target
  res = resolveIncludePathInternal(absoluteFile, wd, possibleTargets.join(" "), source, maximumInternalResolutionDepth);
  if (!res) {
    ifTest(cout << "Try for absolute file " << absoluteFile.toLocal8Bit().data() << " and targets " << possibleTargets.join(", ").toLocal8Bit().data()
                 << " failed: " << res.longErrorMessage.toLocal8Bit().data() << endl;)
  }

  res.includePathDependency = dependency;

  if (res.paths.isEmpty())
      res.paths = cachedPaths; //We failed, maybe there is an old cached result, use that.

  {
    QMutexLocker l(&s_cacheMutex);
    if (it == s_cache.end())
      it = s_cache.insert(dir.path(), CacheEntry());

    CacheEntry& ce(*it);
    ce.paths = res.paths;
    ce.modificationTime = dependency;

    if (!res) {
      ce.failed = true;
      ce.errorMessage = res.errorMessage;
      ce.longErrorMessage = res.longErrorMessage;
      ce.failTime = QDateTime::currentDateTime();
      ce.failedFiles[file] = true;
    } else {
      ce.failed = false;
      ce.failedFiles.clear();
    }
  }


  if (!res && (!resultOnFail.errorMessage.isEmpty() || !resultOnFail.paths.isEmpty()))
    return resultOnFail;

  return res;
}

PathResolutionResult MakeFileResolver::resolveIncludePathInternal(const QString& file, const QString& workingDirectory,
                                                                      const QString& makeParameters, const SourcePathInformation& source,
                                                                      int maxDepth) const
{
  --maxDepth;
  if (maxDepth < 0)
    return PathResolutionResult(false);

  QString processStdout;

  QStringList touchFiles;
  if (source.shouldTouchFiles()) {
    touchFiles << file;
  }

  FileModificationTimeWrapper touch(touchFiles, workingDirectory);

  QString fullOutput;
  executeCommand(source.getCommand(file, workingDirectory, makeParameters), workingDirectory, fullOutput);

  QString includeParameterRx("\\s(-I|--include-dir=|-I\\s)");
  QString quotedRx("(\\').*(\\')|(\\\").*(\\\")"); //Matches "hello", 'hello', 'hello"hallo"', etc.
  QString escapedPathRx("(([^)(\"'\\s]*)(\\\\\\s)?)*"); //Matches /usr/I\ am \ a\ strange\ path/include

  QRegExp includeRx(QString("%1(%2|%3)(?=\\s)").arg(includeParameterRx).arg(quotedRx).arg(escapedPathRx));
  includeRx.setMinimal(true);
  includeRx.setCaseSensitivity(Qt::CaseSensitive);

  QRegExp newLineRx("\\\\\\n");
  fullOutput.replace(newLineRx, "");
  ///@todo collect multiple outputs at the same time for performance-reasons
  QString firstLine = fullOutput;
  int lineEnd;
  if ((lineEnd = fullOutput.indexOf('\n')) != -1)
    firstLine.truncate(lineEnd); //Only look at the first line of output

  /**
   * There's two possible cases this can currently handle.
   * 1.: gcc is called, with the parameters we are searching for (so we parse the parameters)
   * 2.: A recursive make is called, within another directory(so we follow the recursion and try again) "cd /foo/bar && make -f pi/pa/build.make pi/pa/po.o
   * */

  ///STEP 1: Test if it is a recursive make-call
  // Do not search for recursive make-calls if we already have include-paths available. Happens in kernel modules.
  if (!fullOutput.contains(includeRx)) {
    QRegExp makeRx("\\bmake\\s");
    int offset = 0;
    while ((offset = makeRx.indexIn(firstLine, offset)) != -1) {
      QString prefix = firstLine.left(offset).trimmed();
      if (prefix.endsWith("&&") || prefix.endsWith(';') || prefix.isEmpty()) {
        QString newWorkingDirectory = workingDirectory;
        ///Extract the new working-directory
        if (!prefix.isEmpty()) {
          if (prefix.endsWith("&&"))
            prefix.truncate(prefix.length() - 2);
          else if (prefix.endsWith(';'))
            prefix.truncate(prefix.length() - 1);

          ///Now test if what we have as prefix is a simple "cd /foo/bar" call.

          //In cases like "cd /media/data/kdedev/4.0/build/kdevelop && cd /media/data/kdedev/4.0/build/kdevelop"
          //We use the second directory. For t hat reason we search for the last index of "cd "
          int cdIndex = prefix.lastIndexOf("cd ");
          if (cdIndex != -1) {
            newWorkingDirectory = prefix.right(prefix.length() - 3 - cdIndex).trimmed();
            if (KUrl(newWorkingDirectory).isRelative())
              newWorkingDirectory = workingDirectory + '/' + newWorkingDirectory;
            KUrl u(newWorkingDirectory);
            u.cleanPath();
            newWorkingDirectory = u.toLocalFile();
          }
        }

        if (newWorkingDirectory == workingDirectory) {
          return PathResolutionResult(false, i18n("Failed to extract new working directory"), i18n("Output was: %1", fullOutput));
        }

        QFileInfo d(newWorkingDirectory);
        if (d.exists()) {
          ///The recursive working-directory exists.
          QString makeParams = firstLine.mid(offset+5);
          if (!makeParams.contains(';') && !makeParams.contains("&&")) {
            ///Looks like valid parameters
            ///Make the file-name absolute, so it can be referenced from any directory
            QString absoluteFile = file;
            if (KUrl(absoluteFile).isRelative())
              absoluteFile = workingDirectory +  '/' + file;
            KUrl u(absoluteFile);
            u.cleanPath();
            ///Try once with absolute, and if that fails with relative path of the file
            SourcePathInformation newSource(newWorkingDirectory);
            PathResolutionResult res = resolveIncludePathInternal(u.toLocalFile(), newWorkingDirectory, makeParams, newSource, maxDepth);
            if (res)
              return res;
            return resolveIncludePathInternal(KUrl::relativePath(newWorkingDirectory, u.toLocalFile()), newWorkingDirectory, makeParams , newSource, maxDepth);
          }else{
            return PathResolutionResult(false, i18n("Recursive make call failed"), i18n("The parameter string \"%1\" does not seem to be valid. Output was: %2.", makeParams, fullOutput));
          }
        } else {
          return PathResolutionResult(false, i18n("Recursive make call failed"), i18n("The directory \"%1\" does not exist. Output was: %2.", newWorkingDirectory, fullOutput));
        }

      } else {
        return PathResolutionResult(false, i18n("Malformed recursive make call"), i18n("Output was: %1", fullOutput));
      }

      ++offset;
      if (offset >= firstLine.length()) break;
    }
  }

  ///STEP 2: Search the output for include-paths

  PathResolutionResult ret(true);
  ret.longErrorMessage = fullOutput;
  ifTest(cout << "full output" << fullOutput.toAscii().data() << endl);

  int offset = 0;

  while ((offset = includeRx.indexIn(fullOutput, offset)) != -1) {
    offset += 1; ///The previous white space
    int pathOffset = 2;
    if (fullOutput[offset+1] == '-') {
      ///Must be --include-dir=, with a length of 14 characters
      pathOffset = 14;
    }
    if (fullOutput.length() <= offset + pathOffset)
      break;

    if (fullOutput[offset+pathOffset].isSpace())
      pathOffset++;

    int start = offset + pathOffset;
    int end = offset + includeRx.matchedLength();

    QString path = fullOutput.mid(start, end-start).trimmed();
    if (path.startsWith('"') || (path.startsWith('\'') && path.length() > 2)) {
      //probable a quoted path
      if (path.endsWith(path.left(1))) {
        //Quotation is ok, remove it
        path = path.mid(1, path.length() - 2);
      }
    }
    if (KUrl(path).isRelative())
      path = workingDirectory + '/' + path;

    KUrl u(path);
    u.cleanPath();

    ret.paths << u.toLocalFile();

    offset = end-1;
  }

  if (ret.paths.isEmpty())
    return PathResolutionResult(false, i18n("Could not extract include paths from make output"),
                                 i18n("Folder: \"%1\"  Command: \"%2\"  Output: \"%3\"", workingDirectory,
                                      source.getCommand(file, workingDirectory, makeParameters), fullOutput));

  return ret;
}

void MakeFileResolver::resetOutOfSourceBuild()
{
  m_outOfSource = false;
}

void MakeFileResolver::setOutOfSourceBuildSystem(const QString& source, const QString& build)
{
  if (source == build) {
    resetOutOfSourceBuild();
    return;
  }
  m_outOfSource = true;
  KUrl sourceUrl(source);
  sourceUrl.cleanPath();
  m_source = sourceUrl.toLocalFile(KUrl::RemoveTrailingSlash);
  KUrl buildUrl(build);
  buildUrl.cleanPath();
  m_build = buildUrl.toLocalFile(KUrl::RemoveTrailingSlash);
}

#ifdef TEST

/** This can be used for testing and debugging the system. To compile it use
 * gcc includepathresolver.cpp -I /usr/share/qt3/include -I /usr/include/kde -I ../../lib/util -DTEST -lkdecore -g -o includepathresolver
 * */

int main(int argc, char **argv)
{
  QApplication app(argc,argv);
  IncludePathResolver resolver;
  if (argc < 3) {
    cout << "params: 1. file-name, 2. working-directory [3. source-directory 4. build-directory]" << endl;
    return 1;
  }
  if (argc >= 5) {
    cout << "mapping" << argv[3] << "->" << argv[4] << endl;
    resolver.setOutOfSourceBuildSystem(argv[3], argv[4]);
  }
  PathResolutionResult res = resolver.resolveIncludePath(argv[1], argv[2]);
  cout << "success:" << res.success << "\n";
  if (!res.success) {
    cout << "error-message: \n" << res.errorMessage.toLocal8Bit().data() << "\n";
    cout << "long error-message: \n" << res.longErrorMessage.toLocal8Bit().data() << "\n";
  }
  cout << "path: \n" << res.paths.join("\n").toLocal8Bit().data() << "\n";
  return res.success;
}
#endif
