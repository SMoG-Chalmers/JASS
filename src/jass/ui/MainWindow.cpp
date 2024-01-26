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
#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmenubar.h>

#include <qapplib/actions/ActionManager.hpp>
#include <qapplib/Editor.hpp>
#include <qapplib/Document.hpp>

#include <qapplib/actions/StandardActions.h>
#include <qapplib/DocumentManager.hpp>
#include <qapplib/RecentFilesMenu.hpp>
#include <qapplib/UiUtils.h>
#include <qapplib/Workbench.hpp>
#include <qapplib/WorkbenchWidget.hpp>

#include "../Debug.h"

#include "AboutDialog.h"
#include "MainWindow.hpp"
#include "CategoryView.hpp"

#define JASS_UI_VERSION 1

// Disable recent files on Mac, since we haven't yet implemented this for the sandbox environment
// https://developer.apple.com/documentation/security/app_sandbox/accessing_files_from_the_macos_app_sandbox
#ifndef __APPLE__
	#define ENABLE_RECENT_FILES
#endif

namespace jass
{
	struct SToolViewDesc
	{
		QString m_Name;
		QIcon   m_Icon;
		bool m_InitiallyVisible = true;
		bool m_InitiallyFloating = false;
		QDockWidget::DockWidgetFeatures m_Features = QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable;
		Qt::DockWidgetArea m_Area = Qt::RightDockWidgetArea;
	};

	CMainWindow::CMainWindow(qapp::CDocumentManager& document_manager, qapp::CWorkbench& workbench, qapp::CActionManager& action_manager)
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
#ifdef ENABLE_RECENT_FILES
		auto* open_button = new QToolButton(this);
		open_button->setDefaultAction(qapp::s_StandardActions.Open);
		open_button->setMenu(recent_files_menu);
		open_button->setPopupMode(QToolButton::MenuButtonPopup);
		m_MainToolBar->addWidget(open_button);
#else
		m_MainToolBar->addAction(qapp::s_StandardActions.Open);
#endif
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
		VERIFY(connect(qapp::s_StandardActions.Open,  &QAction::triggered, this, &CMainWindow::OnOpen));

		// Status Bar
		auto* status_bar = statusBar();
	
		// File Menu
		auto* fileMenu = Menu("file");
		fileMenu->addAction(qapp::s_StandardActions.New);
		fileMenu->addSeparator();
		fileMenu->addAction(qapp::s_StandardActions.Open);
#ifdef ENABLE_RECENT_FILES
		fileMenu->addMenu(recent_files_menu);
#endif
		fileMenu->addSeparator();
		fileMenu->addAction(qapp::s_StandardActions.Save);
		fileMenu->addAction(qapp::s_StandardActions.SaveAs);
		fileMenu->addAction(qapp::s_StandardActions.Export);
		fileMenu->addSeparator();
		fileMenu->addAction(qapp::s_StandardActions.Exit);

		// Edit Menu
		auto* editMenu = Menu("edit");
		fileMenu->addAction(qapp::s_StandardActions.Cut);
		fileMenu->addAction(qapp::s_StandardActions.Copy);
		fileMenu->addAction(qapp::s_StandardActions.Paste);
		fileMenu->addAction(qapp::s_StandardActions.Duplicate);
		fileMenu->addAction(qapp::s_StandardActions.Delete);

		// Workbench widget
		m_WorkbenchWidget = new qapp::CWorkbenchWidget(this, workbench);
		m_WorkbenchWidget->show();
		setCentralWidget(m_WorkbenchWidget);

		// Category View
		{
			m_CategoryView = new CCategoryView(this, action_manager);
			SToolViewDesc desc;
			desc.m_Name = "Categories";
			desc.m_Icon = QIcon(":/categories.png");
			desc.m_InitiallyVisible = true;
			desc.m_InitiallyFloating = true;
			desc.m_Features = QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable;
			desc.m_Area = Qt::RightDockWidgetArea;
			AddToolView(m_CategoryView, desc);
		}

		action_manager.AddActionTarget(this, qapp::EActionTargetPrio::MainWindow);
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

	void CMainWindow::About()
	{
		CAboutDialog dialog(this);
		dialog.exec();
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

	QDockWidget* CMainWindow::AddToolView(QWidget* widget, const SToolViewDesc& desc)
	{
		auto* dock_widget = new QDockWidget(desc.m_Name, this);

		dock_widget->setObjectName(desc.m_Name);

		auto* toggle_view_action = dock_widget->toggleViewAction();
		toggle_view_action->setToolTip(QString("Show/hide %1 view").arg(desc.m_Name.toLower()));
		toggle_view_action->setIcon(desc.m_Icon);
		//if (!m_ViewsSeparatorAction)
		//	m_ViewsSeparatorAction = m_MainToolBar->insertSeparator(m_MainToolBar->actions().back());
		//m_MainToolBar->insertAction(m_ViewsSeparatorAction, toggle_view_action);
		//m_ViewMenu->addAction(toggle_view_action);

		dock_widget->setWindowFlags(Qt::Tool);
		dock_widget->setWidget(widget);
		dock_widget->setFeatures(desc.m_Features);
		addDockWidget(desc.m_Area, dock_widget);
		//if (!desc.m_InitiallyVisible)
		//	SetInitiallyHidden(dock_widget);
		dock_widget->setFloating(desc.m_InitiallyFloating);
		return dock_widget;
	}


	void CMainWindow::UpdateActions(qapp::CActionUpdateContext& ctx)
	{
		ctx.Enable(qapp::s_StandardActionHandles.About);
	}

	bool CMainWindow::OnAction(qapp::HAction action_handle)
	{
		if (qapp::s_StandardActionHandles.About == action_handle)
		{
			About();
			return true;
		}
		return false;
	}
}

#include <moc_MainWindow.cpp>
