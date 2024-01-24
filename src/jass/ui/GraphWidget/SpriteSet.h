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

		void DrawSprite(size_t index, QPainter& painter, const QPoint& at);

	private:
		struct SSprite
		{
			QPixmap Pixmap;
			QPoint  Origin;
		};

		std::vector<SSprite> m_Sprites;
	};
}