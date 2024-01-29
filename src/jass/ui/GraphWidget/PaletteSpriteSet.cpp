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
#include <jass/Debug.h>
#include <jass/Settings.hpp>
#include <jass/ui/ImageFx.h>
#include "PaletteSpriteSet.h"
#include "NodeSprite.h"

namespace jass
{
	static const QRgb COLOR_SELECTED = qRgb(0x0a, 0x84, 0xff);
	static const QRgb COLOR_HILIGHT = Blend(COLOR_SELECTED, 0xFFFFFFFF, 48);

	CPaletteSpriteSet::CPaletteSpriteSet(const std::span<const QRgb>& palette, QRgb no_color, const CSettings& settings)
		: m_Settings(settings)
	{
		m_Palette.reserve(palette.size() + 1);
		m_Palette.insert(m_Palette.begin(), palette.begin(), palette.end());
		m_Palette.push_back(no_color);

		connect(&settings, &CSettings::Changed, this, &CPaletteSpriteSet::OnSettingChanged);
	}

	void CPaletteSpriteSet::Draw(EShape shape, EStyle style, uint32_t palette_index, const QPoint& pos, QPainter& painter) const
	{
		const auto& sprite = Sprite(shape, style);
		palette_index = std::min(palette_index, (uint32_t)(m_Palette.size() - 1));
		painter.drawPixmap(pos - sprite.Origin, sprite.Pixmap, QRect(0, sprite.Size.height() * palette_index, sprite.Size.width(), sprite.Size.height()));
	}

	void CPaletteSpriteSet::OnSettingChanged(const QString& key, const QVariant& newValue)
	{
		if (key == CSettings::UI_SCALE)
		{
			for (auto& sprite : m_Sprites)
			{
				sprite.Pixmap = QPixmap();
			}
			emit Changed();
		}
	}

	float CPaletteSpriteSet::SpriteScale() const
	{
		return .01f * m_Settings.value(CSettings::UI_SCALE, 100).toInt();
	}

	void CPaletteSpriteSet::CreateSprite(EShape shape, EStyle style, const std::span<const QRgb>& palette, SSprite& out_sprite)
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
		desc.Shape = shape;

		if (EStyle::Selected == style)
		{
			desc.OutlineColor2 = COLOR_SELECTED;
			desc.OutlineWidth2 = desc.OutlineWidth;
		}
		else if (EStyle::Hilighted == style)
		{
			desc.OutlineColor2 = COLOR_HILIGHT;
			desc.OutlineWidth2 = std::round(sprite_scale * 4);
		}

		QPixmap pmTemp;

		desc.FillColor = palette.front();
		CreateNodeSprite(desc, pmTemp, out_sprite.Origin);
		out_sprite.Size = pmTemp.size();

		out_sprite.Pixmap = QPixmap(out_sprite.Size.width(), out_sprite.Size.height() * (int)palette.size());
		out_sprite.Pixmap.fill(Qt::transparent);

		QPainter painter(&out_sprite.Pixmap);
		painter.drawPixmap(0, 0, pmTemp);

		for (size_t palette_index = 1; palette_index < palette.size(); ++palette_index)
		{
			desc.FillColor = palette[palette_index];
			CreateNodeSprite(desc, pmTemp, out_sprite.Origin);
			painter.drawPixmap(0, (int)palette_index * out_sprite.Size.height(), pmTemp);
		}
	}

	void InterpolatePalette(const std::span<const QRgb>& in_colors, const std::span<QRgb>& out_colors)
	{
		const float r = (float)((int)in_colors.size() - 1) / (int)(out_colors.size() - 1);
		for (int i = 0; i < (int)out_colors.size(); ++i)
		{
			const float t = r * i;
			const auto index0 = (int)t;
			const auto color0 = in_colors[index0];
			const auto color1 = in_colors[std::min(index0 + 1, (int)in_colors.size() - 1)];
			out_colors[i] = Blend(color0, color1, (uint8_t)((t - (float)index0) * 255));
		}
	}
}

#include <moc_PaletteSpriteSet.cpp>