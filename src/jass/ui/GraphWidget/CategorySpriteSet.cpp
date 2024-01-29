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

#include <QtGui/qpainter.h>
#include <jass/ui/ImageFx.h>
#include <jass/Settings.hpp>
#include "NodeSprite.h"
#include "CategorySpriteSet.hpp"

namespace jass
{
	static const QRgb COLOR_SELECTED = qRgb(0x0a, 0x84, 0xff);
	static const QRgb COLOR_HILIGHT = Blend(COLOR_SELECTED, 0xFFFFFFFF, 48);

	CCategorySpriteSet::CCategorySpriteSet(const CCategorySet& categories, const CSettings& settings)
		: m_Categories(categories)
		, m_Settings(settings)
	{
		connect(&categories, &CCategorySet::rowsInserted, this, &CCategorySpriteSet::OnCategoriesInserted);
		connect(&categories, &CCategorySet::rowsRemoved,  this, &CCategorySpriteSet::OnCategoriesRemoved);
		connect(&categories, &CCategorySet::CategoriesRemapped, this, &CCategorySpriteSet::OnCategoriesRemapped);
		connect(&categories, &CCategorySet::dataChanged,  this, &CCategorySpriteSet::OnCategoriesChanged);
		connect(&settings,   &CSettings::Changed,         this, &CCategorySpriteSet::OnSettingChanged);

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

	void CCategorySpriteSet::OnSettingChanged(const QString& key, const QVariant& newValue)
	{
		if (key == CSettings::UI_SCALE)
		{
			UpdateSprites();
			emit Changed();
		}
	}

	float CCategorySpriteSet::SpriteScale() const
	{
		return .01f * m_Settings.value(CSettings::UI_SCALE, 100).toInt();
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

		const auto sprite_scale = SpriteScale();

		SNodeSpriteDesc desc;
		desc.Radius = (9 * sprite_scale) + .5f;
		desc.OutlineWidth = std::round(sprite_scale * 3);
		desc.OutlineWidth2 = 0;
		const auto shadow_offset = (float)std::max((int)1, (int)std::round(sprite_scale));
		desc.ShadowOffset = { shadow_offset, shadow_offset };
		desc.ShadowBlurRadius = std::round(sprite_scale * 3);
		desc.OutlineColor = qRgb(255, 255, 255);
		desc.ShadowColor = qRgba(0, 0, 0, 255);
		desc.Shape = m_Categories.Shape(category_index);

		desc.FillColor = m_Categories.Color(category_index);
		CreateNodeSprite(desc, pixmap, origin);
		UpdateSprite(category_index * SPRITE_COUNT_PER_CATEGORY + 0, std::move(pixmap), origin);

		desc.OutlineColor2 = COLOR_SELECTED;
		desc.OutlineWidth2 = desc.OutlineWidth;
		CreateNodeSprite(desc, pixmap, origin);
		UpdateSprite(category_index * SPRITE_COUNT_PER_CATEGORY + 1, std::move(pixmap), origin);

		desc.OutlineColor2 = COLOR_HILIGHT;
		desc.OutlineWidth2 = std::round(sprite_scale * 4);
		desc.FillColor = Blend(m_Categories.Color(category_index), 0xFFFFFFFF, 64);
		CreateNodeSprite(desc, pixmap, origin);
		UpdateSprite(category_index * SPRITE_COUNT_PER_CATEGORY + 2, std::move(pixmap), origin);
	}
}

#include <moc_CategorySpriteSet.cpp>