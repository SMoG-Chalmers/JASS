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

#include <jass/Debug.h>
#include "GraphData.h"

namespace jass
{
	size_t CGraphData::AttributeCount() const
	{
		return m_Attributes.size();
	}

	void CGraphData::GetAttribute(size_t index, QString& out_name, QVariant& out_value) const
	{
		out_name = m_Attributes[index].first;
		out_value = m_Attributes[index].second;
	}

	size_t CGraphData::NodeCount() const
	{
		return m_NodeCount;
	}

	size_t CGraphData::NodeAttributeCount() const
	{
		return m_NodeAttributes.size();
	}

	SNodeAttributeDesc CGraphData::NodeAttributeDesc(size_t index) const
	{
		return m_NodeAttributes[index].Desc;
	}

	void CGraphData::GetNodeAttributeData(size_t index, void* buffer, size_t size) const
	{
		const auto& data = m_NodeAttributes[index].Data;
		ASSERT(data.size() == size);
		memcpy(buffer, data.data(), size);
	}

	size_t CGraphData::EdgeCount() const
	{
		return m_Edges.size();
	}
	
	void CGraphData::GetEdges(std::span<edge_t> out_edges) const
	{
		ASSERT(out_edges.size() == m_Edges.size());
		memcpy(out_edges.data(), m_Edges.data(), m_Edges.size() * sizeof(edge_t));
	}

	void CGraphData::SetAttribute(const QString& name, const QVariant& value)
	{
		for (auto& a : m_Attributes)
		{
			if (a.first == name)
			{
				a.second = value;
				return;
			}
		}
		m_Attributes.push_back({ name, value });
	}

	void CGraphData::SetNodeCount(size_t count)
	{
		ASSERT(m_NodeAttributes.empty());
		m_NodeCount = count;
	}

	void CGraphData::AddNodeAttribute(const SNodeAttributeDesc& desc, const void* data, size_t size)
	{
		SNodeAttribute attr;
		attr.Desc = desc;
		attr.Data = QByteArray((int)size, 0);
		memcpy(attr.Data.data(), data, size);
		m_NodeAttributes.push_back(std::move(attr));
	}

	void CGraphData::SetEdges(std::span<const edge_t> edges)
	{
		m_Edges.resize(edges.size());
		memcpy(m_Edges.data(), edges.data(), edges.size() * sizeof(edge_t));
	}
}