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

#include <QtCore/qsettings.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmenubar.h>

#include <qapplib/Editor.hpp>
#include <qapplib/Document.hpp>

#include <qapplib/actions/StandardActions.h>
#include <qapplib/DocumentManager.hpp>
#include <qapplib/RecentFilesMenu.hpp>
#include <qapplib/UiUtils.h>
#include <qapplib/Workbench.hpp>
#include <qapplib/WorkbenchWidget.hpp>

#include "../Debug.h"

#include "MainWindow.hpp"

#define JASS_UI_VERSION 1

namespace jass
{
	CMainWindow::CMainWindow(qapp::CDocumentManager& document_manager, qapp::CWorkbench& workbench)
		: m_DocumentManager(document_manager)
		, m_Workbench(workbench)
	{
		auto* qapp = QApplication::instance();

		// Default size
		resize(QSize(1024, 768));

		// Icon
		setWindowIcon(QIcon(":/logo/logo_64x64.png"));

		// Title
		setWindowTitle(qapp->applicationName() + " (" + qapp->applicationVersion() + ")");

		// Recent Files Menu
		auto* recent_files_menu = new qapp::CRecentFilesMenu(this, document_manager, workbench);

		// Toolbar
		m_MainToolBar = addToolBar("Main");
		m_MainToolBar->setObjectName("MainToolBar");
		m_MainToolBar->addAction(qapp::s_StandardActions.New);
		auto* open_button = new QToolButton(this);
		open_button->setDefaultAction(qapp::s_StandardActions.Open);
		open_button->setMenu(recent_files_menu);
		open_button->setPopupMode(QToolButton::MenuButtonPopup);
		m_MainToolBar->addWidget(open_button);
		m_MainToolBar->addSeparator();
		m_MainToolBar->addAction(qapp::s_StandardActions.Save);
		m_MainToolBar->addSeparator();
		m_MainToolBar->addAction(qapp::s_StandardActions.Undo);
		m_MainToolBar->addAction(qapp::s_StandardActions.Redo);
		m_MainToolBar->addSeparator();
		m_MainToolBar->addAction(qapp::s_StandardActions.Cut);
		m_MainToolBar->addAction(qapp::s_StandardActions.Copy);
		m_MainToolBar->addAction(qapp::s_StandardActions.Paste);
		m_MainToolBar->addAction(qapp::s_StandardActions.Delete);
		m_MainToolBar->addSeparator();
		m_MainToolBar->addAction(qapp::s_StandardActions.About);

		// Actions
		VERIFY(connect(qapp::s_StandardActions.New,    &QAction::triggered, this, &CMainWindow::OnNew));
		VERIFY(connect(qapp::s_StandardActions.Open,   SIGNAL(triggered(bool)), SLOT(OnOpen())));

		// Status Bar
		auto* status_bar = statusBar();
	
		// Menu
		auto* fileMenu = Menu("file");
		fileMenu->addAction(qapp::s_StandardActions.New);
		fileMenu->addSeparator();
		fileMenu->addAction(qapp::s_StandardActions.Open);
		fileMenu->addMenu(recent_files_menu);
		fileMenu->addSeparator();
		fileMenu->addAction(qapp::s_StandardActions.Save);
		fileMenu->addAction(qapp::s_StandardActions.SaveAs);
		fileMenu->addSeparator();
		fileMenu->addAction(qapp::s_StandardActions.Exit);

		// Workbench widget
		m_WorkbenchWidget = new qapp::CWorkbenchWidget(this, workbench);
		m_WorkbenchWidget->show();
		setCentralWidget(m_WorkbenchWidget);
	}

	CMainWindow::~CMainWindow()
	{
	}

	void CMainWindow::SaveLayout(QSettings& settings)
	{
		settings.setValue("geometry", saveGeometry());
		settings.setValue("windowState", saveState(JASS_UI_VERSION));
	}

	void CMainWindow::RestoreLayout(QSettings& settings)
	{
		restoreGeometry(settings.value("geometry").toByteArray());
		restoreState(settings.value("windowState").toByteArray(), JASS_UI_VERSION);
	}

	QMenu* CMainWindow::Menu(const QString& name)
	{
		for (const auto& m : m_Menus)
			if (name.compare(m.first, Qt::CaseInsensitive) == 0)
				return m.second;
		QString prettyName = QString("&%1").arg(name);
		prettyName[1] = prettyName.at(1).toUpper();
		auto* menu = menuBar()->addMenu(prettyName);
		m_Menus.push_back(std::make_pair(name, menu));
		return menu;
	}

	void CMainWindow::OnNew()
	{
		m_Workbench.New(*m_DocumentManager.DocumentTypes().front().Handler);
	}

	void CMainWindow::OnOpen()
	{
		auto path = qapp::GetOpenFileName(this, m_DocumentManager, m_Workbench.LastDirectory());
		if (path.isEmpty())
		{
			return;
		}
		m_Workbench.Open(path);
	}
}

#include <moc_MainWindow.cpp>
