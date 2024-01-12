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

#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QMainWindow>

class QMenu;
class QSettings;
class QToolBar;

namespace qapp
{
	class CDocumentManager;
	class CWorkbench;
	class CWorkbenchWidget;
	class IEditor;
}

namespace jass
{
	class CMainWindow : public QMainWindow
	{
		Q_OBJECT
	public:
		CMainWindow(qapp::CDocumentManager& document_manager, qapp::CWorkbench& workbench);
		~CMainWindow();

		void SaveLayout(QSettings& settings);

		void RestoreLayout(QSettings& settings);

	private Q_SLOTS:
		void OnOpen();

	private:
		QMenu* Menu(const QString& name);

		bool Save(qapp::IEditor& editor);
		bool SaveAs(qapp::IEditor& editor);

		qapp::CDocumentManager& m_DocumentManager;
		qapp::CWorkbench& m_Workbench;

		QToolBar* m_MainToolBar = nullptr;
		std::vector<std::pair<QString, QMenu*>> m_Menus;
		qapp::CWorkbenchWidget* m_WorkbenchWidget = nullptr;
	};
}
