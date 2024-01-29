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

#include <vector>
#include "GraphWidget.hpp"

class QPainter;

namespace jass
{
	class CItemGraphLayer: public CGraphLayer
	{
	public:
		CItemGraphLayer(CGraphWidget& graphWidget);

		virtual QRect ItemRect(element_t element) const = 0;
		virtual void DrawItem(element_t element, QPainter& painter, const QRect& rc) const = 0;

		inline const QRect& LastItemRect(element_t element) const;

		// CGraphLayer overrides
		void      Paint(QPainter& painter, const QRect& rc) override;
		element_t HitTest(const QPoint& pt) override;
		bool      RangedHitTest(const QRect& rc, bitvec& out_hit_elements) const override;

	protected:
		void ClearItems();
		void InsertItems(size_t index, size_t count);

	private:
		struct SItem
		{
			QRect LastRect;  // Rect of last time it was drawn
		};

		std::vector<SItem> m_Items;
	};

	inline const QRect& CItemGraphLayer::LastItemRect(element_t element) const
	{
		return m_Items[element].LastRect;
	}
}