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

#include "GraphNodeTheme.hpp"
#include "SpriteSet.h"

namespace jass
{
	class CGraphModel;
	class CCategorySpriteSet;

	class CGraphNodeCategoryTheme: public CGraphNodeTheme
	{
		Q_OBJECT
	public:
		CGraphNodeCategoryTheme(CGraphModel& graph_model, std::shared_ptr<CCategorySpriteSet> sprites);
		~CGraphNodeCategoryTheme();

		// CGraphNodeTheme overrides
		QRect ElementLocalRect(element_t element, EStyle style) const override;
		void  DrawElement(element_t element, EStyle style, const QPoint& pos, QPainter& painter) const override;

	private Q_SLOTS:
		void OnSpritesChanged();

	private:
		static const uint8_t SPRITE_COUNT_PER_CATEGORY = 3;

		struct SSpriteDesc;

		inline size_t SpriteIndex(size_t category_index, EStyle style) const;
		
		void UpdateSprites();
		void UpdateSpritesForCategory(size_t catgory_index);
		void CreateSprite(const SSpriteDesc& desc, QPixmap& out_pixmap, QPoint& out_origin);

		CGraphModel& m_GraphModel;
		std::shared_ptr<CCategorySpriteSet> m_Sprites;
	};
}