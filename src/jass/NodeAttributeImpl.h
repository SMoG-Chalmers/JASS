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

#include <qapplib/utils/QVariantType.h>
#include <jass/utils/range_utils.h>
#include "NodeAttribute.h"

namespace jass
{
	// CNodeAttributeBase

	inline const QVariant::Type CNodeAttributeBase::Type() const 
	{
		return m_Type; 
	}

	inline void CNodeAttributeBase::BeginModify()
	{
		m_GraphModel.BeginModifyNodes();
	}

	inline void CNodeAttributeBase::EndModify()
	{
		m_GraphModel.EndModifyNodes();
	}


	// CNodeAttribute<T>

	template <class T>
	inline CNodeAttribute<T>::CNodeAttribute(CGraphModel& graph_model)
		: CNodeAttributeBase(graph_model, qapp::QVariantType<T>())
	{
	}

	template <class T>
	const T& CNodeAttribute<T>::Value(size_t node_index)
	{
		return m_Values[node_index];
	}

	template <class T>
	void CNodeAttribute<T>::SetValue(size_t node_index, const T& value)
	{
		m_Values[node_index] = value;
		m_GraphModel.SetNodeModified((CGraphModel::node_index_t)node_index);
	}

	template <class T>
	void CNodeAttribute<T>::Init(const void* data, size_t data_size_bytes)
	{
		ASSERT(m_Values.size() * sizeof(T) == data_size_bytes);
		std::copy((const T*)data, (const T*)data + m_Values.size(), m_Values.begin());
	}

	template <class T>
	void CNodeAttribute<T>::Copy(void* buffer, size_t buffer_size_bytes) const
	{
		ASSERT(m_Values.size() * sizeof(T) == buffer_size_bytes);
		std::copy(m_Values.begin(), m_Values.end(), (T*)buffer);
	}

	template <class T>
	void CNodeAttribute<T>::Resize(size_t count)
	{
		m_Values.resize(count);
	}

	template <class T>
	void CNodeAttribute<T>::Expand(const std::span<const node_index_t>& hole_indices)
	{
		jass::expand(m_Values, hole_indices);
	}

	template <class T>
	void CNodeAttribute<T>::Collapse(const std::span<const node_index_t>& remove_indices)
	{
		jass::collapse(m_Values, remove_indices);
	}
}
