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

#include <QtCore/qabstractitemmodel.h>
#include <QtWidgets/qwidget.h>
#include <jass/GraphEditor/CategorySet.hpp>
#include <qapplib/actions/ActionTarget.h>

class QAction;
class QTooplBar;
class QListView;

namespace qapp
{
	class CActionManager;
}

namespace jass
{
	class CCategoryDialog;

	class CCategoryView : public QWidget, public qapp::IActionTarget
	{
		Q_OBJECT
	public:
		CCategoryView(QWidget* parent, qapp::CActionManager& action_manager);

		void SetCategories(CCategorySet* categories);

		QToolBar& ToolBar() { return *m_ToolBar; }

		QListView& ListView() { return *m_ListView; }

		bool eventFilter(QObject* watched, QEvent* event) override;

		// qapp::IActionTarget
		void UpdateActions(qapp::CActionUpdateContext& ctx) override;
		bool OnAction(qapp::HAction action_handle) override;

	Q_SIGNALS:
		void RemoveCategories(const QModelIndexList& indexes);
		void AddCategory(const QString& name, QRgb color, EShape shape);
		void ModifyCategory(int index, const QString& name, QRgb color, EShape shape);

	private Q_SLOTS:
		void UpdateMyActions();
		void OnAdd();
		void OnRemove();
		void EditCategory(const QModelIndex& index);

	private:
		CCategoryDialog* CategoryDialog();

		qapp::CActionManager& m_ActionManager;
		QAction* m_AddAction = nullptr;
		QAction* m_RemoveAction = nullptr;
		QToolBar* m_ToolBar = nullptr;
		QListView* m_ListView = nullptr;
		CCategorySet* m_Categories = nullptr;
		CCategoryDialog* m_CategoryDialog = nullptr;
	};
}