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
#include <QtCore/qpoint.h>
#include <QtGui/qpixmap.h>

namespace jass
{
	class CSpriteSet
	{
	public:
		CSpriteSet();

		void Clear();

		inline size_t Count() const { return m_Sprites.size(); }

		inline void Resize(size_t count) { m_Sprites.resize(count); }

		size_t AddSprite(QPixmap&& pixmap, const QPoint& origin);

		void UpdateSprite(size_t index, QPixmap&& pixmap, const QPoint& origin);

		void RemoveSprites(size_t first, size_t count);

		void InsertSprite(size_t index, QPixmap&& pixmap, const QPoint& origin);

		void InsertSprites(size_t index, size_t count);

		QRect SpriteRect(size_t index) const;

		void DrawSprite(size_t index, QPainter& painter, const QPoint& at) const;

	private:
		struct SSprite
		{
			QPixmap Pixmap;
			QPoint  Origin;
		};

		std::vector<SSprite> m_Sprites;
	};
}