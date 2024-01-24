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

#include <QtGui/qpainter.h>
#include "ItemGraphLayer.h"

namespace jass
{
	CItemGraphLayer::CItemGraphLayer(CGraphWidget& graphWidget)
		: CGraphLayer(graphWidget)
	{
	}

	void CItemGraphLayer::Paint(QPainter& painter, const QRect& rcClip)
	{
		for (size_t item_index = 0; item_index < m_Items.size(); ++item_index)
		{
			const auto itemRect = ItemRect(item_index);
			const auto rcVis = itemRect.intersected(rcClip);
			if (!rcVis.isEmpty())
			{
				DrawItem(item_index, painter, itemRect);
			}
			m_Items[item_index].LastRect = itemRect;
		}
	}

	CItemGraphLayer::element_t CItemGraphLayer::HitTest(const QPoint& pt)
	{
		for (size_t item_index = m_Items.size() - 1; item_index < m_Items.size(); --item_index)
		{
			if (m_Items[item_index].LastRect.contains(pt))
			{
				return item_index;
			}
		}
		return NO_ELEMENT;
	}

	bool CItemGraphLayer::RangedHitTest(const QRect& rc, bitvec& out_hit_elements) const
	{
		bool any_hit = false;
		out_hit_elements.clear();
		out_hit_elements.resize(m_Items.size());
		for (size_t item_index = 0; item_index < m_Items.size(); ++item_index)
		{
			if (rc.intersects(m_Items[item_index].LastRect))
			{
				out_hit_elements.set(item_index);
				any_hit = true;
			}
		}
		return any_hit;
	}

	void CItemGraphLayer::ClearItems()
	{
		if (m_Items.empty())
		{
			return;
		}

		m_Items.clear();

		Update();
	}

	void CItemGraphLayer::InsertItems(size_t index, size_t count)
	{
		if (0 == count)
		{
			return;
		}

		m_Items.insert(m_Items.begin() + index, count, {});

		Update();
	}
}
