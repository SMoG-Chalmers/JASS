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

#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QMainWindow>

class QDockWidget;
class QMenu;
class QSettings;
class QToolBar;

namespace qapp
{
	class CActionManager;
	class CDocumentManager;
	class CWorkbench;
	class CWorkbenchWidget;
	class IEditor;
}

namespace jass
{
	class CCategoryView;
	struct SToolViewDesc;

	class CMainWindow : public QMainWindow, public qapp::IActionTarget
	{
		Q_OBJECT
	public:
		CMainWindow(qapp::CDocumentManager& document_manager, qapp::CWorkbench& workbench, qapp::CActionManager& action_manager);
		~CMainWindow();

		void SaveLayout(QSettings& settings);

		void RestoreLayout(QSettings& settings);

		CCategoryView& CategoryView() { return *m_CategoryView; }

		QMenu* Menu(const QString& name, QAction** out_action = nullptr);

	private Q_SLOTS:
		void OnOpen();

	private:
		struct SMenu
		{
			QString  Name;
			QMenu*   Menu = nullptr;
			QAction* Action = nullptr;
		};
		void About();

		QDockWidget* AddToolView(QWidget* widget, const SToolViewDesc& desc);

		// qapp::IActionTarget
		void UpdateActions(qapp::CActionUpdateContext& ctx) override;
		bool OnAction(qapp::HAction action_handle) override;

		qapp::CDocumentManager& m_DocumentManager;
		qapp::CWorkbench& m_Workbench;
		
		QAction* m_ViewsSeparatorAction = nullptr;
		QMenu* m_ViewMenu = nullptr;
		QToolBar* m_MainToolBar = nullptr;
		std::vector<SMenu> m_Menus;
		qapp::CWorkbenchWidget* m_WorkbenchWidget = nullptr;

		CCategoryView* m_CategoryView = nullptr;
	};
}
