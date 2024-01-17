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

#include <span>
#include <vector>

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qstring.h>
#include <QtGui/qicon.h>
#include <QtGui/qrgb.h>

#include <jass/Shape.h>

class QIODevice;

namespace jass
{
	class CCategorySet : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		inline size_t Size() const { return m_Categories.size(); }

		inline const QString& Name(size_t category_index) const { return m_Categories[category_index].Name; }

		inline const QRgb Color(size_t category_index) const { return m_Categories[category_index].Color; }

		inline const EShape Shape(size_t category_index) const { return m_Categories[category_index].Shape; }

		inline QIcon Icon(size_t category_index) const { return QIcon(); }

		void AddCategory(QString name, QRgb color, EShape shape);

		void SetDefaultCategories();

		void SetCategory(size_t index, QString name, QRgb color, EShape shape);

		void RemoveCategory(size_t index);

		void Load(QIODevice& in);

		void Save(QIODevice& out);

		// QAbstractItemModel
		QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex& model_index) const override;
		Qt::ItemFlags flags(const QModelIndex& index) const override;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		int	rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	Q_SIGNALS:
		void OnCategoriesRemapped(const std::span<const int>& remap_table);

	private:
		struct SCategory
		{
			QString Name;
			QRgb    Color;
			EShape  Shape;
			QIcon   Icon;
		};

		std::vector<SCategory> m_Categories;
	};
}