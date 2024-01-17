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

#include <QtGui/qicon.h>
#include "CategorySet.hpp"

namespace jass
{
	void CCategorySet::AddCategory(QString name, QRgb color, EShape shape)
	{
		beginInsertRows(QModelIndex(), (int)m_Categories.size(), (int)m_Categories.size());
		
		m_Categories.push_back({name, color, shape, QIcon()});
		
		endInsertRows();
	}

	void CCategorySet::SetDefaultCategories()
	{
		const QRgb palette[] = {
			qRgb(0xcd, 0x00, 0x1a),
			qRgb(0xef, 0x6a, 0x00),
			qRgb(0xf2, 0xcd, 0x00),
			qRgb(0x79, 0xc3, 0x00),
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
		//category.Icon

		auto model_index = this->index((int)index, 0);
		emit dataChanged(model_index, model_index);
	}

	void CCategorySet::RemoveCategory(size_t index)
	{
		beginRemoveRows(QModelIndex(), (int)index, (int)index);

		m_Categories.erase(m_Categories.begin() + index);

		endRemoveRows();
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
			return m_Categories[index.row()].Icon;
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