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

#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtWidgets/qstyleditemdelegate.h>
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

		//void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
		//{
		//	// Custom painting logic here
		//	// You have full control over how the items are drawn

		//	// Example: Draw text with a red background
		//	painter->fillRect(option.rect, Qt::red);
		//	painter->drawText(option.rect, index.data().toString());
		//}

		//bool eventFilter(QObject* watched, QEvent* event) override
		//{
		//	if (watched == m_ParentView && event->type() == QEvent::MouseMove)
		//	{
		//		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		//		if (mouseEvent->pos().x() > 50)
		//		{
		//			m_ParentView->setCursor(Qt::PointingHandCursor);
		//		}
		//		else
		//		{
		//			m_ParentView->setCursor(Qt::ArrowCursor);
		//		}
		//	}

		//	// Continue with the default event handling
		//	return QStyledItemDelegate::eventFilter(watched, event);
		//}

		//bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override
		//{
		//	if (event->type() == QEvent::MouseMove) 
		//	{
		//		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

		//		//// Check if the mouse press occurred within the item's rectangle
		//		//if (option.rect.contains(mouseEvent->pos())) {
		//		//	// Store the current index to use it in the paint method for highlighting
		//		//	currentIndex = index;
		//		//	// Trigger a repaint of the item
		//		//	emit QAbstractItemView::update(index);
		//		//	return true; // Event handled
		//		//}
		//	}

		//	return QStyledItemDelegate::editorEvent(event, model, option, index);
		//}

	private:
		QListView* m_ParentView;
	};

	CCategoryView::CCategoryView(QWidget* parent)
		: QListView(parent)
	{
		auto* dlgt = new CCategoryViewDelegate(this);
		setItemDelegate(dlgt);
	}
}