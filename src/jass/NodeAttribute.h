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

#include <QtCore/qvariant.h>

namespace jass
{
	class CGraphModel;

	class CNodeAttributeBase
	{
	public:
		typedef uint32_t node_index_t;

		CNodeAttributeBase(CGraphModel& graph_model, QVariant::Type type);
		virtual ~CNodeAttributeBase();

		inline const QVariant::Type Type() const;

		inline void BeginModify();

		inline void EndModify();

		virtual void Init(const void* data, size_t data_size_bytes) = 0;

		virtual void Copy(void* buffer, size_t buffer_size_bytes) const = 0;

		template <class T>
		inline const T& Value(size_t node_index) const;

		template <class T>
		inline void SetValue(size_t node_index, const T& value);

	protected:
		virtual void Resize(size_t count) = 0;

		virtual void Expand(const std::span<const node_index_t>& hole_indices) = 0;

		virtual void Collapse(const std::span<const node_index_t>& hole_indices) = 0;

		friend CGraphModel;

		CGraphModel& m_GraphModel;
		const QVariant::Type m_Type;
	};

	template <class T>
	class CNodeAttribute : public CNodeAttributeBase
	{
	public:
		typedef T value_t;

		inline CNodeAttribute(CGraphModel& graph_model, const T& default_value = T());

		inline const T& Value(size_t node_index) const;
		inline void     SetValue(size_t node_index, const T& value);

		void Init(const void* data, size_t data_size_bytes) override;

		void Copy(void* buffer, size_t buffer_size_bytes) const override;

	protected:
		void Resize(size_t count) override;
		void Expand(const std::span<const node_index_t>& hole_indices) override;
		void Collapse(const std::span<const node_index_t>& remove_indices) override;

	private:
		std::vector<T> m_Values;
		const T m_DefaultValue;
	};
}
