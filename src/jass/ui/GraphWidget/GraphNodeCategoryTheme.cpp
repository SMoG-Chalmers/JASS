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

#include <jass/GraphModel.hpp>
#include "GraphNodeCategoryTheme.hpp"
#include "CategorySpriteSet.hpp"

namespace jass
{
	CGraphNodeCategoryTheme::CGraphNodeCategoryTheme(CGraphModel& graph_model, std::shared_ptr<CCategorySpriteSet> sprites)
		: m_GraphModel(graph_model)
		, m_Sprites(std::move(sprites))
	{
		VERIFY(connect(m_Sprites.get(), &CCategorySpriteSet::Changed, this, &CGraphNodeCategoryTheme::OnSpritesChanged));
	}

	CGraphNodeCategoryTheme::~CGraphNodeCategoryTheme()
	{
	}

	QRect CGraphNodeCategoryTheme::ElementLocalRect(element_t element, EStyle style) const
	{
		const auto category_index = m_GraphModel.NodeCategory((CGraphModel::node_index_t)element);
		const auto sprite_index = m_Sprites->SpriteIndex(category_index, style);
		return m_Sprites->SpriteRect(sprite_index);
	}

	void CGraphNodeCategoryTheme::DrawElement(element_t element, EStyle style, const QPoint& pos, QPainter& painter) const
	{
		const auto category_index = m_GraphModel.NodeCategory((CGraphModel::node_index_t)element);
		const auto sprite_index = m_Sprites->SpriteIndex(category_index, style);
		m_Sprites->DrawSprite(sprite_index, painter, pos);
	}

	void CGraphNodeCategoryTheme::OnSpritesChanged()
	{
		emit Updated();
	}
}

#include <moc_GraphNodeCategoryTheme.cpp>