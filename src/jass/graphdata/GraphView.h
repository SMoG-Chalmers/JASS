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
#include <qapplib/utils/QVariantType.h>
#include "GraphDataCommon.h"

namespace jass
{
	class IGraphView
	{
	public:
		typedef std::pair<uint32_t, uint32_t> edge_t;

		virtual size_t AttributeCount() const = 0;
		virtual void   GetAttribute(size_t index, QString& out_name, QVariant& out_value) const = 0;
		virtual size_t NodeCount() const = 0;
		virtual size_t NodeAttributeCount() const = 0;
		virtual SNodeAttributeDesc NodeAttributeDesc(size_t index) const = 0;
		virtual void   GetNodeAttributeData(size_t index, void* buffer, size_t size) const = 0;
		virtual size_t EdgeCount() const = 0;
		virtual void   GetEdges(std::span<edge_t> out_edges) const = 0;
	
		// Helpers
		inline bool FindNodeAttributes(const QString& name, size_t& out_index, SNodeAttributeDesc& out_desc) const;
		template <typename T> inline void GetNodeAttributeData(size_t index, std::span<T> out_span) const;
		
		template <typename T>
		inline bool TryGetNodeAttributes(const QString& name, std::span<T> out_data) const;
	};

	inline bool IGraphView::FindNodeAttributes(const QString& name, size_t& out_index, SNodeAttributeDesc& out_desc) const
	{
		const auto node_attribute_count = NodeAttributeCount();
		for (size_t node_attribute_index = 0; node_attribute_index < node_attribute_count; ++node_attribute_index)
		{
			out_desc = NodeAttributeDesc(node_attribute_index);
			if (out_desc.Name == name)
			{
				out_index = node_attribute_index;
				return true;
			}
		}
		return false;
	}

	template <typename T> inline void IGraphView::GetNodeAttributeData(size_t index, std::span<T> out_span) const
	{
		return GetNodeAttributeData(index, out_span.data(), out_span.size() * sizeof(T));
	}

	template <typename T>
	inline bool IGraphView::TryGetNodeAttributes(const QString& name, std::span<T> out_data) const
	{
		size_t index;
		SNodeAttributeDesc desc;
		if (!FindNodeAttributes(name, index, desc) || qapp::QVariantType<T>() != desc.Type)
		{
			return false;
		}
		GetNodeAttributeData(index, out_data.data(), out_data.size() * sizeof(T));
		return true;
	}
}