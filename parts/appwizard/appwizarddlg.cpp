/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "appwizarddlg.h"

#include <qvbox.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qgrid.h>
#include <qheader.h>
#include <qmap.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtextview.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <klistview.h>
#include <kiconview.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kfile.h>
#include <kapplication.h>
#include <kpopupmenu.h>

#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kio/netaccess.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <kmacroexpander.h>
#include <karchive.h>
#include <ktar.h>

#include <qlayout.h>

#include "kdevversioncontrol.h"
#include "kdevmakefrontend.h"
#include "kdevlicense.h"
#include "kdevcore.h"
#include "appwizardfactory.h"
#include "appwizardpart.h"
#include "filepropspage.h"
#include "misc.h"
#include "dataform.h"

AppWizardDialog::AppWizardDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : AppWizardDialogBase(parent, name,true), m_pCurrentAppInfo(0)
{
	kdDebug( 9000 ) << "  ** AppWizardDialog::AppWizardDialog()" << endl;

	m_customOptions = 0L;
    connect( this, SIGNAL( selected( const QString & ) ), this, SLOT( pageChanged() ) );
    
	helpButton()->hide();
    templates_listview->header()->hide();
	templates_listview->setColumnWidthMode(0, QListView::Maximum);	//to provide horiz scrollbar.
	
	m_templatesMenu = new KPopupMenu(templates_listview);
	m_templatesMenu->insertItem(i18n("&Add To Favorites"), this, SLOT(addTemplateToFavourites()));
	
	m_favouritesMenu = new KPopupMenu(favourites_iconview);
	m_favouritesMenu->insertItem(i18n("&Remove Favorite"), this, SLOT(removeFavourite()));
	
    m_pathIsValid=false;
    m_part = part;
    m_projectLocationWasChanged=false;
    m_appsInfo.setAutoDelete(true);
    m_tempFiles.setAutoDelete(true);

    KConfig *config = kapp->config();
	
	//config->setGroup("AppWizard");
	//templates_tabwidget->setCurrentPage(config->readNumEntry("CurrentTab", 0));

	config->setGroup("General Options");
    QString defaultProjectsDir = config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/");

    KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
    QStringList m_templateNames = dirs->findAllResources("apptemplates", QString::null, false, true);

    kdDebug(9010) << "Templates: " << endl;
    QStringList categories;

    QStringList::Iterator it;
    for (it = m_templateNames.begin(); it != m_templateNames.end(); ++it) {
        kdDebug(9010) << (*it) << endl;
		KConfig templateConfig(KGlobal::dirs()->findResource("apptemplates", *it));
        templateConfig.setGroup("General");

        ApplicationInfo *info = new ApplicationInfo;
        info->templateName = (*it);
        info->name = templateConfig.readEntry("Name");
        info->icon = templateConfig.readEntry("Icon");
        info->comment = templateConfig.readEntry("Comment");
        info->fileTemplates = templateConfig.readEntry("FileTemplates");
        info->openFilesAfterGeneration = templateConfig.readListEntry("ShowFilesAfterGeneration");
        QString destDir = templateConfig.readPathEntry("DefaultDestinatonDir", defaultProjectsDir);
        destDir.replace(QRegExp("HOMEDIR"), QDir::homeDirPath());
        info->defaultDestDir = destDir;
        QString category = templateConfig.readEntry("Category");
        // format category to a unique status
        if (category.right(1) == "/")
            category.remove(category.length()-1, 1); // remove /
        if (category.left(1) != "/")
            category.prepend("/"); // prepend /
        categories.append(category);
        info->category = category;
		// Grab properties and file/dir list
		QStringList groups = templateConfig.groupList();
		groups.remove("General");
		QStringList::Iterator group = groups.begin();
		for(  ; group != groups.end(); ++group)
		{
			templateConfig.setGroup( (*group) );
			QString type = templateConfig.readEntry("Type", "value");
			kdDebug(9010) << "Reading " <<  (*group) << " of type " << type << endl;
			if( type == "value" )  // Add value
			{
				QString name = templateConfig.readEntry( "Value" );
				QString label = templateConfig.readEntry( "Comment" );
				QString type = templateConfig.readEntry( "ValueType", "String" );
				autoKey key( name, label);
				QVariant value = templateConfig.readPropertyEntry( "Default", QVariant::nameToType( type.latin1() ) );
				value.cast( QVariant::nameToType( type.latin1() ) );  // fix this in kdelibs...
				info->subValues.insert( key, value );
			} 
			else if( type == "install" ) // copy dir
			{
				installFile file;
				file.source = templateConfig.readPathEntry("Source");
				file.dest = templateConfig.readPathEntry("Dest");
				file.process = templateConfig.readBoolEntry("Process",true);
				file.option = templateConfig.readEntry("Option");
				info->fileList.append(file);
			}
			else if( type == "install archive" )
			{
				installArchive arch;
				arch.source = templateConfig.readPathEntry("Source");
				arch.dest = templateConfig.readPathEntry("Dest");
				arch.process = templateConfig.readBoolEntry("Process",true);
				arch.option = templateConfig.readEntry("Option", "" );
				info->archList.append(arch);
			}
			else if( type == "mkdir" )
			{
				installDir dir;
				dir.dir = templateConfig.readPathEntry("Dir");
				dir.option = templateConfig.readEntry("Option", "" );
				dir.perms = templateConfig.readNumEntry("Perms", 0777 );
				info->dirList.append(dir);
			}
			else if( type == "ui")
			{
				QString name = templateConfig.readPathEntry("File");
				info->customUI = name;
			}
			else if( type == "message" )
			{
				info->message = templateConfig.readEntry( "Comment" );
			}
		}
        m_appsInfo.append(info);
    }

    // Insert categories into list view
    categories.sort();
    for (it = categories.begin(); it != categories.end(); ++it)
        insertCategoryIntoTreeView(*it);

    // Insert items into list view
    QPtrListIterator<ApplicationInfo> ait(m_appsInfo);
    for (; ait.current(); ++ait) {
        QListViewItem *item = m_categoryMap.find(ait.current()->category);
        if (item)
		{
            item = new KListViewItem(item, ait.current()->name);
			item->setPixmap(0, SmallIcon("kdevelop"));
		}
        else
            kdDebug(9010) << "Error can't find category in categoryMap: "
                          << ait.current()->category << endl;
        ait.current()->item = item;
    }
	
	//Load favourites from config
	populateFavourites();
	
	QString author, email;
    AppWizardUtil::guessAuthorAndEmail(&author, &email);
    author_edit->setText(author);
    email_edit->setText(email);
    QToolTip::add( dest_edit->button(), i18n("Choose projects directory") );
    dest_edit->setURL(defaultProjectsDir);
    dest_edit->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    loadVcs();

    //    addPage(m_sdi_fileprops_page,"Class/File Properties");

    //    licenseChanged();

    setNextEnabled(generalPage, false);

//    QRegExp appname_regexp ("[a-zA-Z][a-zA-Z0-9_]*"); //Non-Unicode version
    /* appname will start with a letter, and will contain letters,
       digits or underscores. */
    QRegExp appname_regexp ("[a-zA-Z][a-zA-Z0-9_]*");
    // How about names like "__" or "123" for project name? Are they legal?
    QRegExpValidator *appname_edit_validator;
    appname_edit_validator = new QRegExpValidator (appname_regexp,
                                                   appname_edit,
                                                   "AppNameValidator");
    appname_edit->setValidator(appname_edit_validator);
    
    // insert the licenses into the license_combo
    QDict< KDevLicense > lics( m_part->core()->licenses() );
    QDictIterator< KDevLicense > dit(lics);
    int idx=1;
    for( ; dit.current(); ++dit )
    {
        license_combo->insertItem( dit.currentKey(), idx++ );
        if( dit.currentKey() == "GPL" )
            license_combo->setCurrentItem( idx - 1 );
    }
	
	m_custom_options_layout = new QHBoxLayout( custom_options );
	m_custom_options_layout->setAutoAdd(true);
}

AppWizardDialog::~AppWizardDialog()
{}

void AppWizardDialog::loadVcs()
{
    m_vcsForm = new VcsForm();

    int i=0;
    m_vcsForm->combo->insertItem( i18n("no version control system", "None"), i );
    m_vcsForm->stack->addWidget( 0, i++ );

    // We query for all vcs plugins for KDevelop
    QStringList availableVcs = m_part->registeredVersionControls();

    for(QStringList::const_iterator it( availableVcs.begin() ); it != availableVcs.end(); ++it)
    {
        KDevVersionControl *vcs = m_part->versionControlByName( (*it) );
        QString vcsName = vcs->uid();

        QWidget *newProjectWidget = vcs->newProjectWidget( m_vcsForm->stack );
        if (newProjectWidget) {
            m_vcsForm->combo->insertItem( vcsName, i );
            m_vcsForm->stack->addWidget( newProjectWidget, i++ );
        }
        else
        {
            kdDebug( 9000 ) << "  ** Warning: VCS has not widget. Skipping. " << endl;
        }
    }

    addPage( m_vcsForm, i18n("Version Control System") );
}


void AppWizardDialog::textChanged()
{
    licenseChanged();

    bool invalid = !m_pCurrentAppInfo
        || appname_edit->text().isEmpty()
        || !m_pathIsValid
        || author_edit->text().isEmpty()
        || version_edit->text().isEmpty();
    setFinishEnabled(m_lastPage, !invalid);
    nextButton()->setEnabled(!invalid);

}


void AppWizardDialog::licenseChanged()
{
	QValueList<AppWizardFileTemplate>::Iterator it;
	if( license_combo->currentItem() == 0 )
	{
		for (it = m_fileTemplates.begin(); it != m_fileTemplates.end(); ++it)
		{
			QMultiLineEdit *edit = (*it).edit;
			edit->setText( QString::null );
		}
	} else {
		KDevLicense* lic = m_part->core()->licenses()[ license_combo->currentText() ];
		for (it = m_fileTemplates.begin(); it != m_fileTemplates.end(); ++it) {
			QString style = (*it).style;
			QMultiLineEdit *edit = (*it).edit;
			
			KDevFile::CommentingStyle commentStyle = KDevFile::CPPStyle;
			if (style == "PStyle") {
				commentStyle = KDevFile::PascalStyle;
			} else if (style == "AdaStyle") {
				commentStyle = KDevFile::AdaStyle;
			} else if (style == "ShellStyle") {
				commentStyle = KDevFile::BashStyle;
			}
			
			QString text;
			text = lic->assemble( commentStyle, author_edit->text(), email_edit->text(), 0 );
			edit->setText(text);
		}
	}
}


void AppWizardDialog::accept()
{
    QFileInfo fi(finalLoc_label->text());
    // check /again/ whether the dir already exists; maybe users create it in the meantime
    if (fi.exists()) {
        KMessageBox::sorry(this, i18n("The directory you have chosen as the location for "
                                      "the project already exists."));
        showPage(generalPage);
        appname_edit->setFocus();
        projectLocationChanged();
        return;
    }
	
	QString source;
    QFileInfo finfo(m_pCurrentAppInfo->templateName);
    QDir dir(finfo.dir());
    dir.cdUp();
    source = dir.absPath();

	// Build KMacroExpander map
	QMap<QString,QString> subMap = m_customOptions->dataForm()->createPropertyMap(); 
	kdDebug(9010) << "subMap size " << subMap.size() << endl;
	// Add builtins
	subMap.insert("APPNAME", appname_edit->text() );
	subMap.insert("APPNAMELC", appname_edit->text().lower() );
	subMap.insert("APPNAMEUC", appname_edit->text().upper() );
	subMap.insert("AUTHOR", author_edit->text() );
	subMap.insert("EMAIL", email_edit->text() );
	subMap.insert("VERSION", version_edit->text());
	subMap.insert("src", source );
	subMap.insert("dest", finalLoc_label->text() );
	subMap.insert("LICENSE", license_combo->currentText() );

	// Add template files to the fileList
	installDir templateDir;
	templateDir.dir = "%{dest}/templates";
	m_pCurrentAppInfo->dirList.prepend(templateDir);
	
	installDir baseDir;
	baseDir.dir = "%{dest}";
	m_pCurrentAppInfo->dirList.prepend( baseDir );
	
    QValueList<AppWizardFileTemplate>::Iterator it;
    for (it = m_fileTemplates.begin(); it != m_fileTemplates.end(); ++it) {
        KTempFile *tempFile = new KTempFile();
        m_tempFiles.append(tempFile);

        QFile f;
        f.open(IO_WriteOnly, tempFile->handle());
        QTextStream temps(&f);
        temps << (*it).edit->text();
        f.flush();

		installFile file;
		file.source = tempFile->name();
		file.dest = QString( "%{dest}/templates/%1" ).arg( (*it).suffix );
		file.process = true;
		m_pCurrentAppInfo->fileList.append( file );
    }
	
	// Add license file to the file list
	QString licenseFile, licenseName = i18n("Custom");
    
    if( license_combo->currentItem() != 0 )
    {
        licenseName = license_combo->currentText();
        KDevLicense* lic = m_part->core()->licenses()[ licenseName ];
        if( lic )
        {
            QStringList files( lic->copyFiles() );
			// Not sure why this wont work
			QStringList::Iterator it = files.begin();
			for( ; it != files.end(); ++it )
			{
				installFile file;
				file.source = QString( "%{src}/template-common/%1" ).arg( *it );
				file.dest = QString("%{dest}/%1").arg( *it );
				file.process = true;
				m_pCurrentAppInfo->fileList.append( file );
			}
			
			subMap.insert("LICENSEFILE", files.first()  ); 
        }
    }
	
	// Run macro expander on both the dir map and file maps
	QValueList<installFile>::Iterator fileIt = m_pCurrentAppInfo->fileList.begin();
	for( ; fileIt != m_pCurrentAppInfo->fileList.end(); ++fileIt)
	{
		(*fileIt).source = KMacroExpander::expandMacros((*fileIt).source , subMap);
		(*fileIt).dest = KMacroExpander::expandMacros((*fileIt).dest , subMap);
	}
	
	QValueList<installArchive>::Iterator archIt = m_pCurrentAppInfo->archList.begin();
	for( ; archIt != m_pCurrentAppInfo->archList.end(); ++archIt)
	{
		(*archIt).source = KMacroExpander::expandMacros((*archIt).source , subMap);
		(*archIt).dest = KMacroExpander::expandMacros((*archIt).dest , subMap);
	}
	
	QValueList<installDir>::Iterator dirIt = m_pCurrentAppInfo->dirList.begin();
	for( ; dirIt != m_pCurrentAppInfo->dirList.end(); ++dirIt)
	{
		(*dirIt).dir = KMacroExpander::expandMacros((*dirIt).dir , subMap);
	}
	
	// Create dirs
	dirIt = m_pCurrentAppInfo->dirList.begin();
	for( ; dirIt != m_pCurrentAppInfo->dirList.end(); ++dirIt)
	{
		kdDebug( 9000 ) << "Process dir " << (*dirIt).dir  << endl;
		if( subMap[(*dirIt).option] != "false" )
		{
			
			if( ! KIO::NetAccess::mkdir( (*dirIt).dir, this ) )
			{
				KMessageBox::sorry(this, QString( i18n("The directory %1 cannot be created.")).arg( (*dirIt).dir ) );
				return;
			}
		}
	}
	// Unpack archives
	archIt = m_pCurrentAppInfo->archList.begin();
	for( ; archIt != m_pCurrentAppInfo->archList.end(); ++archIt)
	{
		if( subMap[(*archIt).option] != "false" )
		{
			KTar archive( (*archIt).source, "application/x-gzip" );
			if( archive.open( IO_ReadOnly ) )
			{
				unpackArchive( archive.directory(), (*archIt).dest, subMap, (*archIt).process );
			}
			else
			{
				KMessageBox::sorry(this, QString( i18n("The archive %1 cannot be opened.")).arg( (*archIt).source ) );
				archive.close();
				return;
			}
			archive.close();
		}
	
	}
	
	// Copy files & Process
	fileIt = m_pCurrentAppInfo->fileList.begin();
	for( ; fileIt != m_pCurrentAppInfo->fileList.end(); ++fileIt)
	{
		kdDebug( 9000 ) << "Process file " << (*fileIt).source << endl;
		if( subMap[(*fileIt).option] != "false" )
		{
			if( !copyFile( (*fileIt).source, (*fileIt).dest, subMap, (*fileIt).process ) )
			{
				KMessageBox::sorry(this, QString( i18n("The file %1 cannot be created.")).arg( (*fileIt).dest) );
				return;
			}
		}
	}
    // if dir still does not exist
    if (!fi.dir().exists()) {
      KMessageBox::sorry(this, i18n("The directory above the chosen location does not exist and cannot be created."));
      showPage(generalPage);
      dest_edit->setFocus();
      return;
    }
 
    if (m_vcsForm->stack->id(m_vcsForm->stack->visibleWidget())) {
        KDevVersionControl* pVC = m_part->versionControlByName( m_vcsForm->combo->currentText() );
        if (pVC) {
			kdDebug( 9000 ) << "Creating new project with selected VCS ..." << endl;
            pVC->createNewProject(finalLoc_label->text());
        }
		else
		{
			kdDebug( 9000 ) << "Could not grab the selected VCS: " << m_vcsForm->combo->currentText() << endl;
		}
    }
    
	KMessageBox::information(this, KMacroExpander::expandMacros(m_pCurrentAppInfo->message, subMap));
	QWizard::accept();
}

bool AppWizardDialog::copyFile( const QString &source, const QString &dest, const QMap<QString,QString> &subMap, bool process )
{
	kdDebug() << "Copy: " << source << " to " << dest << endl;
	if( process )
	{
		// Process the file and save it at the destFile location
		QFile inputFile( source);
		QFile outputFile( dest );
		if( inputFile.open( IO_ReadOnly ) && outputFile.open(IO_WriteOnly) )
		{
			QTextStream input( &inputFile );
			QTextStream output( &outputFile );
			while( !input.atEnd() )
				output << KMacroExpander::expandMacros(input.readLine(), subMap) << "\n";
		}
		else
		{
			inputFile.close();
			outputFile.close();
			return false;
		}
		inputFile.close();
		outputFile.close();
	}
	else
	{
		// Copy the source file to the destFile.
		return KIO::NetAccess::copy( source, dest, this );
	}
	return true;
}

void AppWizardDialog::unpackArchive( const KArchiveDirectory *dir, const QString &dest, const QMap<QString,QString> &subMap, bool process )
{
	KIO::NetAccess::mkdir( dest , this );
	kdDebug() << "Dir : " << dir->name() << " at " << dest << endl;
	QStringList entries = dir->entries();
	kdDebug() << "Entries : " << entries.join(",") << endl;
	
	QStringList::Iterator entry = entries.begin();
	for( ; entry != entries.end(); ++entry )
	{
		
		if( dir->entry( (*entry) )->isDirectory()  )
		{
			const KArchiveDirectory *file = (KArchiveDirectory *)dir->entry( (*entry) );
			unpackArchive( file , dest + "/" + file->name(),  subMap, process);
		}
		else if( dir->entry( (*entry) )->isFile()  )
		{
			const KArchiveFile *file = (KArchiveFile *) dir->entry( (*entry) );
			KTempFile temp;
			file->copyTo( temp.name() );
			if ( !copyFile( temp.name(), dest + "//" + file->name() , subMap,  process ) )
			{
				KMessageBox::sorry(this, QString( i18n("The file %1 cannot be created.")).arg( dest) );
				temp.unlink();
				return;
			}
			temp.unlink();
		}
	}
}

void AppWizardDialog::templatesTreeViewClicked(QListViewItem *item)
{
	if( m_customOptions )
		delete m_customOptions;
		
    // Delete old file template pages
    while (!m_fileTemplates.isEmpty()) {
        QMultiLineEdit *edit = m_fileTemplates.first().edit;
        removePage(edit);
        delete edit;
        m_fileTemplates.remove(m_fileTemplates.begin());
    }
    m_lastPage = 0;

    ApplicationInfo *info = templateForItem(item);
    if (info) {
        m_pCurrentAppInfo = info;
        if (!info->icon.isEmpty()) {
            QFileInfo fi(info->templateName);
            QDir dir(fi.dir());
            dir.cdUp();
            QPixmap pm;
            pm.load(dir.filePath("template-" + fi.fileName() + "/" + info->icon));
            icon_label->setPixmap(pm);
        } else {
            icon_label->clear();
        }
        desc_textview->setText(info->comment);
//        dest_edit->setURL(info->defaultDestDir);
        m_projectLocationWasChanged = false;
        //projectNameChanged(); // set the dest new

		// Populate new custom options form
		m_customOptions = new AutoForm( &m_pCurrentAppInfo->subValues, custom_options );
		
		custom_options->adjustSize();
		
        // Create new file template pages
        QStringList l = QStringList::split(",", info->fileTemplates);
        QStringList::ConstIterator it = l.begin();
        while (it != l.end()) {
            AppWizardFileTemplate fileTemplate;
            fileTemplate.suffix = *it;
            ++it;
            if (it != l.end()) {
                fileTemplate.style = *it;
                ++it;
            } else
                fileTemplate.style = "";

            QMultiLineEdit *edit = new QMultiLineEdit(this);
            edit->setWordWrap(QTextEdit::NoWrap);
            edit->setFont(KGlobalSettings::fixedFont());
            if (it == l.end())
                m_lastPage = edit;
            fileTemplate.edit = edit;
            addPage(edit, i18n("Template for .%1 Files").arg(fileTemplate.suffix));
            m_fileTemplates.append(fileTemplate);
        }
        // licenseChanged(); // update template editors
        textChanged(); // calls licenseChanged() && update Next button state
    } else {
        m_pCurrentAppInfo=0;
        icon_label->clear();
        desc_textview->clear();
        nextButton()->setEnabled(false);
    }
}


void AppWizardDialog::destButtonClicked(const QString& dir)
{
    if(!dir.isEmpty()) {

        // set new location as default project dir?
        KConfig *config = kapp->config();
        config->setGroup("General Options");
        QDir defPrjDir( config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()) );
        QDir newDir (dir);
        kdDebug(9010) << "DevPrjDir == newdir?: " << defPrjDir.absPath() << " == " << newDir.absPath() << endl;
        if (defPrjDir != newDir) {
            if (KMessageBox::questionYesNo(this, i18n("Set default project location to: ") + newDir.absPath() + "?",
                                           i18n("New Project")) == KMessageBox::Yes)
            {
                config->writePathEntry("DefaultProjectsDir", newDir.absPath() + "/");
                config->sync();
            }
        }
    }
}


void AppWizardDialog::projectNameChanged()
{
    // Location was already edited by hand => don't change
}


void AppWizardDialog::projectLocationChanged()
{
  // Jakob Simon-Gaarde: Got tired of the anoying bug with the appname/location confussion.
  // This version insures WYSIWYG and checks pathvalidity
  finalLoc_label->setText(dest_edit->url() + (dest_edit->url().right(1)=="/" ? "":"/") + appname_edit->text().lower());
  QDir qd(dest_edit->url());
  QFileInfo fi(dest_edit->url() + "/" + appname_edit->text().lower());
  if (!qd.exists() || appname_edit->displayText().isEmpty()||fi.exists())
  {
    if (!fi.exists() || appname_edit->displayText().isEmpty()) {
      finalLoc_label->setText(finalLoc_label->text() + i18n("invalid location", " (invalid)"));
    } else {
      finalLoc_label->setText(finalLoc_label->text() + i18n(" (dir/file already exist)"));
    }
    m_pathIsValid=false;
  } else {
    m_pathIsValid=true;
  }
    bool invalid = !m_pCurrentAppInfo
       || appname_edit->text().isEmpty()
       || !m_pathIsValid
       || author_edit->text().isEmpty()
       || version_edit->text().isEmpty();
    setFinishEnabled(m_lastPage, !invalid);
    nextButton()->setEnabled(!invalid);
}


void AppWizardDialog::insertCategoryIntoTreeView(const QString &completeCategoryPath)
{
    kdDebug(9010) << "TemplateCategory: " << completeCategoryPath << endl;
    QStringList categories = QStringList::split("/", completeCategoryPath);
    QString category ="";
    QListViewItem* pParentItem=0;

    QStringList::ConstIterator it;
    for (it = categories.begin(); it != categories.end(); ++it) {
        category = category + "/" + *it;
        QListViewItem *item = m_categoryMap.find(category);
        if (!item) { // not found, create it
            if (!pParentItem)
                pParentItem = new KListViewItem(templates_listview,*it);
            else
                pParentItem = new KListViewItem(pParentItem,*it);

            pParentItem->setPixmap(0, SmallIcon("folder"));
            //pParentItem->setOpen(true);
            kdDebug(9010) << "Category: " << category << endl;
            m_categoryMap.insert(category,pParentItem);
        } else {
            pParentItem = item;
        }
    }
}


ApplicationInfo *AppWizardDialog::templateForItem(QListViewItem *item)
{
    QPtrListIterator<ApplicationInfo> it(m_appsInfo);
    for (; it.current(); ++it)
        if (it.current()->item == item)
            return it.current();

    return 0;
}

QStringList AppWizardDialog::getFilesToOpenAfterGeneration()
{
    for ( QStringList::Iterator it = m_pCurrentAppInfo->openFilesAfterGeneration.begin();
          it != m_pCurrentAppInfo->openFilesAfterGeneration.end(); ++it ) {
        (*it).replace("APPNAMEUC", getProjectName().upper());
        (*it).replace("APPNAMELC", getProjectName().lower());
        (*it).replace("APPNAME", getProjectName());
    }
    return m_pCurrentAppInfo->openFilesAfterGeneration;
}

void AppWizardDialog::pageChanged()
{
	kdDebug(9010) << "AppWizardDialog::pageChanged()" << endl;
	projectLocationChanged();
}

void AppWizardDialog::addTemplateToFavourites()
{
	addFavourite(templates_listview->currentItem());
}

void AppWizardDialog::addFavourite(QListViewItem* item, QString favouriteName)
{
	if(item->childCount())	
		return;
		
	ApplicationInfo* info = templateForItem(item);
	
	if(!info->favourite)
	{
		info->favourite = new KIconViewItem(favourites_iconview, 
											((favouriteName=="")?info->name:favouriteName), 
											DesktopIcon("kdevelop"));
											
		info->favourite->setRenameEnabled(true);
	}
}

ApplicationInfo* AppWizardDialog::findFavouriteInfo(QIconViewItem* item)
{
    QPtrListIterator<ApplicationInfo> info(m_appsInfo);
    for (; info.current(); ++info)
        if (info.current()->favourite == item)
            return info.current();

	return 0;
}

void AppWizardDialog::favouritesIconViewClicked( QIconViewItem* item)
{
	ApplicationInfo* info = findFavouriteInfo(item);
	templatesTreeViewClicked(info->item);
}

void AppWizardDialog::removeFavourite()
{
	QIconViewItem* curFavourite = favourites_iconview->currentItem();
	
	//remove reference to favourite from associated appinfo
	QPtrListIterator<ApplicationInfo> info(m_appsInfo);
	for (; info.current(); ++info)
	{
        if(info.current()->favourite && info.current()->favourite == curFavourite)
		{
			info.current()->favourite = 0;
		}
	}
	
	//remove favourite from iconview
	delete curFavourite;
	curFavourite=0;
	favourites_iconview->sort();	//re-arrange all items.
}

void AppWizardDialog::populateFavourites()
{
	KConfig* config = kapp->config();
	config->setGroup("AppWizard");
	
	//favourites are stored in config as a list of templates and a seperate
	//list of icon names.  
	QStringList templatesList = config->readPathListEntry("FavTemplates");
	QStringList iconNamesList = config->readListEntry("FavNames");
    
	QStringList::Iterator curTemplate = templatesList.begin();
	QStringList::Iterator curIconName = iconNamesList.begin();
	while(curTemplate != templatesList.end())
	{
		QPtrListIterator<ApplicationInfo> info(m_appsInfo);
		for (; info.current(); ++info) 
		{
			if(info.current()->templateName == *curTemplate)
			{
				addFavourite(info.current()->item, *curIconName);
				break;
			}
		}
		curTemplate++;
		curIconName++;
	}
}

void AppWizardDialog::done(int r)
{
	//need to save the template for each favourite and
	//it's icon name.  We have a one list for the templates
	//and one for the names.
	
	QStringList templatesList;
	QStringList iconNamesList;
	
	//Built the stringlists for each template that has a favourite.
	QPtrListIterator<ApplicationInfo> it(m_appsInfo);
	for (; it.current(); ++it)
	{
        if(it.current()->favourite)
		{
			templatesList.append(it.current()->templateName);
			iconNamesList.append(it.current()->favourite->text());
		}
	}
	
	KConfig* config = kapp->config();
	config->setGroup("AppWizard");
	config->writePathEntry("FavTemplates", templatesList);
	config->writeEntry("FavNames", iconNamesList);
	//config->writeEntry("CurrentTab", templates_tabwidget->currentPageIndex());
	config->sync();

	QDialog::done(r);
}

void AppWizardDialog::templatesContextMenu(QListViewItem* item, const QPoint& point, int)
{
	if(item && !item->childCount())
		m_templatesMenu->popup(point);
}

void AppWizardDialog::favouritesContextMenu(QIconViewItem* item, const QPoint& point)
{
	if(item)
		m_favouritesMenu->popup(point);
}

#include "appwizarddlg.moc"

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
