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
#include <jass/ui/ImageFx.h>
#include "NodeSprite.h"
#include "CategorySpriteSet.hpp"

namespace jass
{
	static const QRgb COLOR_SELECTED = qRgb(0x0a, 0x84, 0xff);
	static const QRgb COLOR_HILIGHT = Blend(COLOR_SELECTED, 0xFFFFFFFF, 48);

	CCategorySpriteSet::CCategorySpriteSet(const CCategorySet& categories)
		: m_Categories(categories)
	{
		connect(&categories, &CCategorySet::rowsInserted, this, &CCategorySpriteSet::OnCategoriesInserted);
		connect(&categories, &CCategorySet::rowsRemoved, this, &CCategorySpriteSet::OnCategoriesRemoved);
		connect(&categories, &CCategorySet::CategoriesRemapped, this, &CCategorySpriteSet::OnCategoriesRemapped);
		connect(&categories, &CCategorySet::dataChanged, this, &CCategorySpriteSet::OnCategoriesChanged);

		UpdateSprites();
	}

	void CCategorySpriteSet::OnCategoriesInserted(const QModelIndex& parent, int first, int last)
	{
		const auto inserted_count = last - first + 1;
		InsertSprites(first * SPRITE_COUNT_PER_CATEGORY, inserted_count * SPRITE_COUNT_PER_CATEGORY);
		for (int category_index = first; category_index <= last; ++category_index)
		{
			UpdateSpritesForCategory((size_t)category_index);
		}
	}

	void CCategorySpriteSet::OnCategoriesRemoved(const QModelIndex& parent, int first, int last)
	{
		const auto removed_count = last - first + 1;
		RemoveSprites(first * SPRITE_COUNT_PER_CATEGORY, removed_count * SPRITE_COUNT_PER_CATEGORY);
	}

	void CCategorySpriteSet::OnCategoriesRemapped(const std::span<const size_t>&)
	{
		UpdateSprites();
	}

	void CCategorySpriteSet::OnCategoriesChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& /*roles*/)
	{
		for (auto category_index = (size_t)topLeft.row(); category_index <= (size_t)bottomRight.row(); ++category_index)
		{
			UpdateSpritesForCategory(category_index);
		}
		emit Changed();
	}

	void CCategorySpriteSet::UpdateSprites()
	{
		// NOTE: Last one will be "None" (with same index as number of categories)
		Resize((m_Categories.Size() + 1) * SPRITE_COUNT_PER_CATEGORY);

		for (size_t category_index = 0; category_index <= m_Categories.Size(); ++category_index)
		{
			UpdateSpritesForCategory(category_index);
		}
	}

	void CCategorySpriteSet::UpdateSpritesForCategory(size_t category_index)
	{
		QPixmap pixmap;
		QPoint origin;

		SNodeSpriteDesc sprite_desc;
		sprite_desc.Radius = 9.5f;
		sprite_desc.OutlineWidth = 3.0f;
		sprite_desc.OutlineWidth2 = 0;
		sprite_desc.ShadowOffset = { 1, 1 };
		sprite_desc.ShadowBlurRadius = 3.0f;
		sprite_desc.OutlineColor = qRgb(255, 255, 255);
		sprite_desc.ShadowColor = qRgba(0, 0, 0, 255);
		sprite_desc.Shape = m_Categories.Shape(category_index);

		sprite_desc.FillColor = m_Categories.Color(category_index);
		CreateNodeSprite(sprite_desc, pixmap, origin);
		UpdateSprite(category_index * SPRITE_COUNT_PER_CATEGORY + 0, std::move(pixmap), origin);

		sprite_desc.OutlineColor2 = COLOR_SELECTED;
		sprite_desc.OutlineWidth2 = sprite_desc.OutlineWidth;
		CreateNodeSprite(sprite_desc, pixmap, origin);
		UpdateSprite(category_index * SPRITE_COUNT_PER_CATEGORY + 1, std::move(pixmap), origin);

		sprite_desc.OutlineColor2 = COLOR_HILIGHT;
		sprite_desc.OutlineWidth2 = 4;
		sprite_desc.FillColor = Blend(m_Categories.Color(category_index), 0xFFFFFFFF, 64);
		CreateNodeSprite(sprite_desc, pixmap, origin);
		UpdateSprite(category_index * SPRITE_COUNT_PER_CATEGORY + 2, std::move(pixmap), origin);
	}
}

#include <moc_CategorySpriteSet.cpp>