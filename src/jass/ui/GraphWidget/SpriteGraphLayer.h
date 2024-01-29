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

// DEPRECATED. TODO: Remove this file!
// DEPRECATED. TODO: Remove this file!
// DEPRECATED. TODO: Remove this file!
// DEPRECATED. TODO: Remove this file!
// DEPRECATED. TODO: Remove this file!
// DEPRECATED. TODO: Remove this file!
// DEPRECATED. TODO: Remove this file!
// DEPRECATED. TODO: Remove this file!
// DEPRECATED. TODO: Remove this file!

#pragma once

#include "ItemGraphLayer.h"

class QPainter;

namespace jass
{
	class CSpriteSet;


	class CSpriteGraphLayer : public CItemGraphLayer
	{
	public:
		CSpriteGraphLayer(CGraphWidget& graphWidget, CSpriteSet* sprites);

		virtual QPoint ItemPosition(element_t element) const = 0;
		virtual size_t ItemSpriteIndex(element_t element) const = 0;

		// CItemGraphLayer overrides
		QRect ItemRect(element_t element) const override;
		void DrawItem(element_t element, QPainter& painter, const QRect& rc) const override;

	private:
		CSpriteSet* m_Sprites = nullptr;
	};
}