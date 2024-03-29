/*
Copyright Ioanna Stavroulaki 2023

This file is part of JASS.

JASS is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version.

JASS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along 
with JASS. If not, see <https://www.gnu.org/licenses/>.
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
#include <jass/Settings.hpp>

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
	static const int s_UiScales[] =
	{
		100,
		150,
		200,
		300,
	};

	struct SToolViewDesc
	{
		QString m_Name;
		QIcon   m_Icon;
		bool m_InitiallyVisible = true;
		bool m_InitiallyFloating = false;
		QDockWidget::DockWidgetFeatures m_Features = QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable;
		Qt::DockWidgetArea m_Area = Qt::RightDockWidgetArea;
	};

	CMainWindow::CMainWindow(qapp::CDocumentManager& document_manager, qapp::CWorkbench& workbench, qapp::CActionManager& action_manager, CSettings& settings)
		: m_DocumentManager(document_manager)
		, m_Workbench(workbench)
		, m_Settings(settings)
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

		// UI Scale
		QMenu* uiScaleMenu = new QMenu("UI Scale", this);
		for (uint32_t i = 0; i < (uint32_t)std::size(s_UiScales); ++i)
		{
			auto* action = new QAction(QString("%1%").arg(s_UiScales[i]), this);
			action->setCheckable(true);
			connect(action, &QAction::triggered, [this, i]() { this->SetUiScalePercent(s_UiScales[i]); });
			m_UiScaleActions.push_back(action);
			uiScaleMenu->addAction(action);
		}
		SetUiScalePercent(m_Settings.value(CSettings::UI_SCALE, 100).toInt());

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
		m_MainToolBar->addAction(qapp::s_StandardActions.Export);
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
		editMenu->addAction(qapp::s_StandardActions.Undo);
		editMenu->addAction(qapp::s_StandardActions.Redo);
		editMenu->addSeparator();
		editMenu->addAction(qapp::s_StandardActions.Cut);
		editMenu->addAction(qapp::s_StandardActions.Copy);
		editMenu->addAction(qapp::s_StandardActions.Paste);
		editMenu->addAction(qapp::s_StandardActions.Duplicate);
		editMenu->addAction(qapp::s_StandardActions.Delete);

		// View menu
		m_ViewMenu = Menu("view");
		m_ViewMenu->addMenu(uiScaleMenu);
		m_ViewMenu->addSeparator();

		// Help Menu
		auto* helpMenu = Menu("help");
		helpMenu->addAction(qapp::s_StandardActions.About);

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
			desc.m_InitiallyFloating = false;
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

	QMenu* CMainWindow::Menu(const QString& name, QAction** out_action)
	{
		for (const auto& m : m_Menus)
		{
			if (name.compare(m.Name, Qt::CaseInsensitive) == 0)
			{
				if (out_action)
				{
					*out_action = m.Action;
				}
				return m.Menu;
			}
		}
		QString prettyName = QString("&%1").arg(name);
		prettyName[1] = prettyName.at(1).toUpper();
		auto* menu = new QMenu(prettyName, this);
		auto* menuAction = menuBar()->addMenu(menu);
		if (out_action)
		{
			*out_action = menuAction;
		}
		m_Menus.push_back({ name, menu, menuAction });
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
		if (!m_ViewsSeparatorAction)
			m_ViewsSeparatorAction = m_MainToolBar->insertSeparator(m_MainToolBar->actions().back());
		m_MainToolBar->insertAction(m_ViewsSeparatorAction, toggle_view_action);
		m_ViewMenu->addAction(toggle_view_action);

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
		ctx.Enable(qapp::s_StandardActionHandles.Exit);
	}

	bool CMainWindow::OnAction(qapp::HAction action_handle)
	{
		if (qapp::s_StandardActionHandles.About == action_handle)
		{
			About();
			return true;
		}
		if (qapp::s_StandardActionHandles.Exit == action_handle)
		{
			this->close();
			return true;
		}
		return false;
	}

	void CMainWindow::SetUiScalePercent(int scale_percent)
	{
		for (size_t i = 0; i < m_UiScaleActions.size(); ++i)
		{
			m_UiScaleActions[i]->setChecked(scale_percent == s_UiScales[i]);
		}
		m_Settings.setValue(CSettings::UI_SCALE, scale_percent);
	}

}

#include <moc_MainWindow.cpp>
