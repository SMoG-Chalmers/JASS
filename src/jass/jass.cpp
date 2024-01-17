/*
Copyright XMN Software AB 2023

JASS is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version. The GNU Lesser General Public License is intended to
guarantee your freedom to share and change all versions of a program --
to make sure it remains free software for all its users.

JASS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with JASS. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtCore/qdir.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstandardpaths.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qstylefactory.h>

#include <qapplib/actions/StandardActions.h>

#include "ui/MainWindow.hpp"

#include <qapplib/actions/ActionManager.hpp>
#include <qapplib/DocumentManager.hpp>
#include <qapplib/Workbench.hpp>
#include <qapplib/utils/PagePool.h>


#include <jass/GraphEditor/JassEditor.hpp>
#include <jass_version.h>
#include "jass.h"
#include "JassDocument.hpp"

namespace jass
{
	#define SETTINGS_FILENAME "settings.ini"
	#define UI_SETTINGS_FILENAME "uisettings.ini"

	static QString AppDataPath();
	
	static QString SettingsPath();
	static QString UiSettingsPath();

	static qapp::TStdPageAllocator<4 * 1024 * 1024> s_PageAllocator;
	static qapp::CPagePool s_PagePool(s_PageAllocator);

	CJass::CJass()
	{
	}

	CJass::~CJass()
	{
	}

	int CJass::Run(int argc, char** argv)
	{
		qapp::CPagePool::SetDefaultPagePool(&s_PagePool);

		// QApplication object
		m_App = std::make_unique<QApplication>(argc, argv);
		m_App->setOrganizationName(VERC_COMPANY);
		m_App->setOrganizationDomain("www.chalmers.se");
		m_App->setApplicationName(VERC_PROJECT_NAME);
		m_App->setApplicationVersion(VERC_VERSION);

		// Apply style
		m_App->setStyle(QStyleFactory::create("Fusion"));
		
		// Make sure app data folder exists
		const auto appDataPath = AppDataPath();
		QDir appDataDir(appDataPath);
		if (!appDataDir.exists())
		{
			LOG_INFO("Creating app data path '%s'", appDataPath.toStdString().c_str());
			appDataDir.mkpath(".");
		}
		
		QSettings settings(SettingsPath(), QSettings::IniFormat);

		qapp::CActionManager action_manager;
		action_manager.Init(*m_App);
		qapp::InitStandardActions(m_App.get(), action_manager);
		qapp::CDocumentManager document_manager;
		qapp::CWorkbench workbench(document_manager, action_manager, settings);

		// Register document types
		document_manager.RegisterDocumentType(CJassDocumentTypeHandler::g_Instance.Description(), CJassDocumentTypeHandler::g_Instance);

		QSettings uiSettings(UiSettingsPath(), QSettings::IniFormat);

		// Main Window
		auto main_window = std::make_unique<CMainWindow>(document_manager, workbench);
		
		main_window->RestoreLayout(uiSettings);

		CJassEditor::InitCommon(workbench, action_manager, main_window.get());

		// New document
		//workbench.New(*document_manager.DocumentTypes().front().Handler);
		//workbench.Open("C:\\p4\\projects\\jass\\material\\Old_files\\spatrel_graph_students_upd");
		//workbench.Open("C:\\p4\\projects\\jass\\test\\9.jass");

		action_manager.UpdateActions();

		main_window->show();

		const auto exit_code = m_App->exec();

		main_window->SaveLayout(uiSettings);

		main_window.reset();

		return exit_code;
	}

	static QString AppDataPath()
	{
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	}

	static QString SettingsPath()
	{
		return AppDataPath() + "/" SETTINGS_FILENAME;
	}

	static QString UiSettingsPath()
	{
		return AppDataPath() + "/" UI_SETTINGS_FILENAME;
	}
}

// Stupid hack needed to avoid "duplicate symbol" linker error when using multiple .qrc files
bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
bool qUnregisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
namespace jass
{
	bool qRegisterResourceData(int a, const unsigned char * b, const unsigned char * c, const unsigned char * d)   { return ::qRegisterResourceData(a, b, c, d); }
	bool qUnregisterResourceData(int a, const unsigned char * b, const unsigned char * c, const unsigned char * d) { return ::qUnregisterResourceData(a, b, c, d); }
	#include <qrc_resources.cpp>
}
