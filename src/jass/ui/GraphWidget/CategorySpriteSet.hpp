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

#include <jass/GraphEditor/CategorySet.hpp>
#include "GraphNodeTheme.hpp"
#include "SpriteSet.h"

namespace jass
{
	class CCategorySet;

	class CCategorySpriteSet: public QObject, public CSpriteSet
	{
		Q_OBJECT
	public:
		using EStyle = CGraphNodeTheme::EStyle;

		CCategorySpriteSet(const CCategorySet& categories);

		inline size_t SpriteIndex(size_t category_index, EStyle style) const;

	Q_SIGNALS:
		void Changed();

	private Q_SLOTS:
		void OnCategoriesInserted(const QModelIndex& parent, int first, int last);
		void OnCategoriesRemoved(const QModelIndex& parent, int first, int last);
		void OnCategoriesRemapped(const std::span<const size_t>&);
		void OnCategoriesChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

	private:
		static const uint8_t SPRITE_COUNT_PER_CATEGORY = 3;

		struct SSpriteDesc;

		void UpdateSprites();
		void UpdateSpritesForCategory(size_t catgory_index);

		const CCategorySet& m_Categories;
	};

	inline size_t CCategorySpriteSet::SpriteIndex(size_t category_index, EStyle style) const
	{
		return (std::min(category_index, m_Categories.Size()) * SPRITE_COUNT_PER_CATEGORY) + (size_t)style;
	}
}