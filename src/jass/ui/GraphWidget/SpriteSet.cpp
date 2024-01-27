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
#include "SpriteSet.h"

namespace jass
{
	CSpriteSet::CSpriteSet()
	{
	}

	void CSpriteSet::Clear()
	{
		m_Sprites.clear();
	}

	size_t CSpriteSet::AddSprite(QPixmap&& pixmap, const QPoint& origin)
	{
		InsertSprite(m_Sprites.size(), std::move(pixmap), origin);
		return m_Sprites.size() - 1;
	}

	void CSpriteSet::UpdateSprite(size_t index, QPixmap&& pixmap, const QPoint& origin)
	{
		m_Sprites[index] = { std::move(pixmap), origin };
	}

	void CSpriteSet::RemoveSprites(size_t first, size_t count)
	{
		auto it = m_Sprites.begin() + first;
		m_Sprites.erase(it, it + count);
	}

	void CSpriteSet::InsertSprite(size_t index, QPixmap&& pixmap, const QPoint& origin)
	{
		m_Sprites.insert(m_Sprites.begin() + index, { std::move(pixmap), origin });
	}

	void CSpriteSet::InsertSprites(size_t index, size_t count)
	{
		m_Sprites.insert(m_Sprites.begin() + index, count, {});
	}

	QRect CSpriteSet::SpriteRect(size_t index) const
	{
		const auto& sprite = m_Sprites[index];
		const auto size = sprite.Pixmap.size();
		return QRect(-sprite.Origin.x(), -sprite.Origin.y(), sprite.Pixmap.width(), sprite.Pixmap.height());
	}

	void CSpriteSet::DrawSprite(size_t index, QPainter& painter, const QPoint& at) const
	{
		const auto& sprite = m_Sprites[index];
		painter.drawPixmap(at - sprite.Origin, sprite.Pixmap);
	}
}