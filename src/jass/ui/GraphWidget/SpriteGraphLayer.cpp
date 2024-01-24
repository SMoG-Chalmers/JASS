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

#include <QtGui/qpainter.h>  // TEMP

#include "SpriteGraphLayer.h"
#include "SpriteSet.h"

namespace jass
{
	CSpriteGraphLayer::CSpriteGraphLayer(CGraphWidget& graphWidget, CSpriteSet* sprites)
		: CItemGraphLayer(graphWidget)
		, m_Sprites(sprites)
	{
	}

	QRect CSpriteGraphLayer::ItemRect(element_t element) const
	{
		const auto sprite_index = ItemSpriteIndex(element);
		return m_Sprites->SpriteRect(sprite_index).translated(ItemPosition(element));
	}

	void CSpriteGraphLayer::DrawItem(element_t element, QPainter& painter, const QRect& rc) const
	{
		m_Sprites->DrawSprite(ItemSpriteIndex(element), painter, ItemPosition(element));
	}
}