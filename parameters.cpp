/* This file is part of KDevelop

   Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "parameters.h"

#include "globalsettings.h"
#include "projectsettings.h"

#include <interfaces/iproject.h>
#include <KShell>

namespace cppcheck
{

Parameters::Parameters(KDevelop::IProject* project)
    : executablePath(defaults::executablePath)
    , hideOutputView(defaults::hideOutputView)
    , showXmlOutput(defaults::showXmlOutput)
    , checkStyle(defaults::checkStyle)
    , checkPerformance(defaults::checkPerformance)
    , checkPortability(defaults::checkPortability)
    , checkInformation(defaults::checkInformation)
    , checkUnusedFunction(defaults::checkUnusedFunction)
    , checkMissingInclude(defaults::checkMissingInclude)
{
    executablePath = KDevelop::Path(GlobalSettings::executablePath()).toLocalFile();
    hideOutputView = GlobalSettings::hideOutputView();
    showXmlOutput  = GlobalSettings::showXmlOutput();

    if (!project)
        return;

    ProjectSettings projectSettings;
    projectSettings.setSharedConfig(project->projectConfiguration());
    projectSettings.load();

    checkStyle           = projectSettings.checkStyle();
    checkPerformance     = projectSettings.checkPerformance();
    checkPortability     = projectSettings.checkPortability();
    checkInformation     = projectSettings.checkInformation();
    checkUnusedFunction  = projectSettings.checkUnusedFunction();
    checkMissingInclude  = projectSettings.checkMissingInclude();

    extraParameters      = projectSettings.extraParameters();
}

QStringList Parameters::commandLine() const
{
    QStringList result;

    result << executablePath;
    result << QStringLiteral("--xml-version=2");

    if (checkStyle)
        result << QStringLiteral("--enable=style");

    if (checkPerformance)
        result << QStringLiteral("--enable=performance");

    if (checkPortability)
        result << QStringLiteral("--enable=portability");

    if (checkInformation)
        result << QStringLiteral("--enable=information");

    if (checkUnusedFunction)
        result << QStringLiteral("--enable=unusedFunction");

    if (checkMissingInclude)
        result << QStringLiteral("--enable=missingInclude");

    if (!extraParameters.isEmpty())
        result << KShell::splitArgs(extraParameters);

    result << checkPath;

    return result;
}

}
