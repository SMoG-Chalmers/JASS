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

#include <QtWidgets/qapplication.h>
#include <QtCore/qitemselectionmodel.h>
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlistview.h>
#include <QtWidgets/qstyleditemdelegate.h>
#include <QtWidgets/qtoolbar.h>

#include <qapplib/actions/ActionManager.hpp>
#include <qapplib/actions/StandardActions.h>
#include <jass/Debug.h>
#include "CategoryDialog.hpp"
#include "CategoryView.hpp"

namespace jass
{
	class CCategoryViewDelegate : public QStyledItemDelegate
	{
	public:
		CCategoryViewDelegate(QListView* parent = nullptr)
			: QStyledItemDelegate(parent)
			, m_ParentView(parent)
		{
		}

		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
		{
			// Customize the size of each item
			QSize size = QStyledItemDelegate::sizeHint(option, index);
			size.setHeight(24);
			return size;
		}

	private:
		QListView* m_ParentView;
	};

	CCategoryView::CCategoryView(QWidget* parent, qapp::CActionManager& action_manager)
		: QWidget(parent)
		, m_ActionManager(action_manager)
	{
		auto* vlayout = new QVBoxLayout(this);
		vlayout->setMargin(0);
		vlayout->setSpacing(0);

		m_ListView = new QListView(this);
		m_ListView->installEventFilter(this);
		auto* dlgt = new CCategoryViewDelegate(m_ListView);
		m_ListView->setItemDelegate(dlgt);
		m_ListView->setUniformItemSizes(true);
		m_ListView->setContextMenuPolicy(Qt::CustomContextMenu);
		m_ListView->setEditTriggers(QListView::NoEditTriggers);  // We don't want item labels to be edited when double-clicked
		m_ListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
		vlayout->addWidget(m_ListView);

		auto hlayout = new QHBoxLayout(this);
		hlayout->setMargin(0);
		hlayout->setSpacing(0);
		hlayout->addStretch();
		m_ToolBar = new QToolBar(this);
		m_ToolBar->setIconSize(QSize(16, 16));
		m_AddAction = new QAction(QIcon(":/add.png"), "Add category...", this);
		m_AddAction->setEnabled(false);
		connect(m_AddAction, &QAction::triggered, this, &CCategoryView::OnAdd);
		m_ToolBar->addAction(m_AddAction);
		m_RemoveAction = new QAction(QIcon(":/remove.png"), "Remove category", this);
		connect(m_RemoveAction, &QAction::triggered, this, &CCategoryView::OnRemove);
		m_RemoveAction->setEnabled(false);
		m_ToolBar->addAction(m_RemoveAction);
		hlayout->addWidget(m_ToolBar);
		vlayout->addLayout(hlayout);

		setLayout(vlayout);

		connect(m_ListView, &QListView::doubleClicked, this, &CCategoryView::EditCategory);
	}

	void CCategoryView::SetCategories(CCategorySet* categories)
	{
		m_Categories = categories;
		m_ListView->setModel(categories);
		connect(m_ListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CCategoryView::UpdateMyActions);
		UpdateMyActions();
	}

	bool CCategoryView::eventFilter(QObject* watched, QEvent* event)
	{
		if (watched == m_ListView)
		{
			switch (event->type())
			{
			case QEvent::FocusIn:
				m_ActionManager.AddActionTarget(this, qapp::EActionTargetPrio::View);
				break;
			case QEvent::FocusOut:
				m_ActionManager.RemoveActionTarget(this);
				break;
			}
		}
		return QWidget::eventFilter(watched, event);
	}

	void CCategoryView::UpdateActions(qapp::CActionUpdateContext& ctx)
	{
		if (m_ListView->selectionModel() && m_ListView->selectionModel()->hasSelection())
		{
			ctx.Enable(qapp::s_StandardActionHandles.Delete);
		}
	}

	bool CCategoryView::OnAction(qapp::HAction action_handle)
	{
		if (action_handle == qapp::s_StandardActionHandles.Delete)
		{
			OnRemove();
			return true;
		}
		return false;
	}

	void CCategoryView::UpdateMyActions()
	{
		m_AddAction->setEnabled(m_ListView->model() != nullptr);
		m_RemoveAction->setEnabled(m_ListView->selectionModel()->hasSelection());
	}

	void CCategoryView::OnAdd()
	{
		if (!m_Categories)
		{
			return;
		}
		auto* dialog = CategoryDialog();
		dialog->setWindowTitle("New Category");
		dialog->SetName("");
		if (dialog->exec() != QDialog::Accepted)
		{
			return;
		}
		emit AddCategory(dialog->Name(), dialog->Color(), dialog->Shape());
	}

	void CCategoryView::OnRemove()
	{
		if (!m_ListView->selectionModel())
		{
			return;
		}
		
		auto selected_model_indexes = m_ListView->selectionModel()->selectedIndexes();
		if (selected_model_indexes.isEmpty())
		{
			return;
		}

		emit RemoveCategories(selected_model_indexes);
	}

	void CCategoryView::EditCategory(const QModelIndex& index)
	{
		if (!m_Categories)
		{
			return;
		}
		const auto category_index = (size_t)index.row();
		auto* dialog = CategoryDialog();
		dialog->setWindowTitle("Edit Category");
		dialog->SetName(m_Categories->Name(category_index));
		dialog->SetColor(m_Categories->Color(category_index));
		dialog->SetShape(m_Categories->Shape(category_index));
		if (dialog->exec() != QDialog::Accepted)
		{
			return;
		}
		emit ModifyCategory((int)category_index, dialog->Name(), dialog->Color(), dialog->Shape());
	}

	CCategoryDialog* CCategoryView::CategoryDialog()
	{
		if (!m_CategoryDialog)
		{
			m_CategoryDialog = new CCategoryDialog(this);
			const auto center = QApplication::activeWindow()->geometry().center();
			const auto size = m_CategoryDialog->sizeHint();
			m_CategoryDialog->setGeometry(QRect(center.x() - size.width() / 2, center.y() - size.height() / 2, size.width(), size.height()));
		}
		return m_CategoryDialog;
	}
}

#include <moc_CategoryView.cpp>