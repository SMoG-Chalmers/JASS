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

#include <QtGui/qicon.h>
#include <jass/utils/range_utils.h>
#include "CategorySet.hpp"

namespace jass
{
	const size_t CCategorySet::NO_CATEGORY = (size_t)-1;

	CCategorySet::CCategorySet()
	{
		m_NoCategory.Name = "None";
		m_NoCategory.Color = qRgb(0xC0, 0xC0, 0xC0);
		m_NoCategory.Shape = EShape::Circle;
	}

	QIcon CCategorySet::Icon(size_t category_index) const
	{
		auto& category = Category(category_index);
		if (category.Icon.isNull())
		{
			category.Icon = QIcon(QPixmap::fromImage(CreateShapeImage(category.Shape, category.Color, 2, QPoint(1,1), 1, 16)));
		}
		return category.Icon;
	}

	void CCategorySet::AddCategory(QString name, QRgb color, EShape shape)
	{
		InsertCategory(m_Categories.size(), name, color, shape);
	}

	void CCategorySet::InsertCategory(size_t index, QString name, QRgb color, EShape shape)
	{
		beginInsertRows(QModelIndex(), (int)index, (int)index);

		m_Categories.insert(m_Categories.begin() + index, { name, color, shape, QIcon() });

		endInsertRows();

		if (index < m_Categories.size() - 1)
		{
			std::vector<size_t> remap_table(m_Categories.size() - 1);
			build_index_expand_table(std::span<const size_t>(&index, 1), to_span(remap_table));
			emit CategoriesRemapped(remap_table);
		}
	}

	void CCategorySet::SetDefaultCategories()
	{
		const QRgb palette[] = {
			qRgb(0xcd, 0x00, 0x1a),
			qRgb(0xef, 0x6a, 0x00),
			qRgb(0xf2, 0xcd, 0x00),
			qRgb(0x79, 0xc3, 0x00),
			qRgb(0x38, 0xaa, 0xb6),
			qRgb(0x19, 0x61, 0xae),
			qRgb(0x61, 0x00, 0x7d),
			qRgb(0xA0, 0x90, 0x80),
		};

		beginResetModel();

		SCategory category;
		for (int i = 0; i < (int)EShape::_COUNT; ++i)
		{
			category.Name = QString("Category %1").arg(i + 1);
			category.Shape = (EShape)i;
			category.Color = palette[i];
			m_Categories.push_back(category);
		}

		endResetModel();
	}

	void CCategorySet::SetCategory(size_t index, QString name, QRgb color, EShape shape)
	{
		auto& category = m_Categories[index];
		category.Name = name;
		category.Color = color;
		category.Shape = shape;
		category.Icon = QIcon();  // Will be recreated

		auto model_index = this->index((int)index, 0);
		emit dataChanged(model_index, model_index);
	}

	void CCategorySet::RemoveCategory(size_t index)
	{
		beginRemoveRows(QModelIndex(), (int)index, (int)index);

		m_Categories.erase(m_Categories.begin() + index);

		endRemoveRows();

		{
			std::vector<size_t> remap_table;
			build_index_collapse_table(m_Categories.size() + 1, std::span<const size_t>(&index, 1), remap_table);
			emit CategoriesRemapped(remap_table);
		}
	}

	void CCategorySet::Load(QIODevice& in)
	{
		beginResetModel();

		// TODO: Load

		endResetModel();
	}

	void CCategorySet::Save(QIODevice& out)
	{
		// TODO: Save
	}

	QModelIndex CCategorySet::index(int row, int column, const QModelIndex& parent) const
	{
		return createIndex(row, column);
	}

	QModelIndex CCategorySet::parent(const QModelIndex& model_index) const
	{
		return QModelIndex();
	}

	Qt::ItemFlags CCategorySet::flags(const QModelIndex& index) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant CCategorySet::data(const QModelIndex& index, int role) const
	{
		switch (role)
		{
		case Qt::DisplayRole:
			return m_Categories[index.row()].Name;
		case Qt::DecorationRole:
			return Icon(index.row());
		}
		return {};
	}

	int	CCategorySet::rowCount(const QModelIndex& parent) const
	{
		return parent.isValid() ? 0 : (int)m_Categories.size();
	}

	int CCategorySet::columnCount(const QModelIndex& parent) const
	{
		return parent.isValid() ? 0 : 1;
	}
}

#include <moc_CategorySet.cpp>