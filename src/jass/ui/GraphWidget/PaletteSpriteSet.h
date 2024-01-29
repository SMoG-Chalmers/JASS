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

#include <span>
#include <jass/GraphEditor/CategorySet.hpp>
#include "GraphNodeTheme.hpp"
#include "SpriteSet.h"

namespace jass
{
	class CPaletteSpriteSet
	{
	public:
		using EStyle = CGraphNodeTheme::EStyle;

		CPaletteSpriteSet(const std::span<const QRgb>& palette, QRgb no_color);

		inline size_t PaletteSize() const { return m_Palette.size() - 1; }

		inline QRect Rect(EShape shape, EStyle style) const;
		void Draw(EShape shape, EStyle style, uint32_t palette_index, const QPoint& pos, QPainter& painter) const;

	private:
		static const uint8_t STYLE_COUNT = (uint8_t)EStyle::_COUNT;

		struct SSprite
		{
			QPoint Origin;
			QSize  Size;
			QPixmap Pixmap;
		};

		SSprite m_Sprites[(uint32_t)EShape::_COUNT * STYLE_COUNT];

		std::vector<QRgb> m_Palette;

		inline SSprite&       Sprite(EShape shape, EStyle style);
		inline const SSprite& Sprite(EShape shape, EStyle style) const { return const_cast<CPaletteSpriteSet*>(this)->Sprite(shape, style); }

		static void CreateSprite(EShape shape, EStyle style, const std::span<const QRgb>& palette, SSprite& out_sprite);
	};

	inline QRect CPaletteSpriteSet::Rect(EShape shape, EStyle style) const
	{
		const auto& sprite = Sprite(shape, style);
		return QRect(-sprite.Origin.x(), -sprite.Origin.y(), sprite.Size.width(), sprite.Size.height());
	}

	inline CPaletteSpriteSet::SSprite& CPaletteSpriteSet::Sprite(EShape shape, EStyle style)
	{
		auto& sprite = m_Sprites[(uint32_t)shape * STYLE_COUNT + (uint8_t)style];
		if (sprite.Pixmap.isNull())
		{
			CreateSprite(shape, style, m_Palette, sprite);
			ASSERT(!sprite.Pixmap.isNull());
		}
		return sprite;
	}

	void InterpolatePalette(const std::span<const QRgb>& in_colors, const std::span<QRgb>& out_colors);
}