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

#include <jass/Debug.h>
#include "GraphModelGraphView.h"

namespace jass
{
	static const SNodeAttributeDesc s_NodeAttributeDescs[] =
	{
		{ GRAPH_NODE_ATTTRIBUTE_POSITION, QVariant::PointF },
		{ GRAPH_NODE_ATTTRIBUTE_CATEGORY, QVariant::UInt },
	};

	CGraphModelGraphView::CGraphModelGraphView(const CGraphModel& data_model)
		: m_DataModel(&data_model)
	{
	}

	size_t CGraphModelGraphView::AttributeCount() const
	{
		return m_DataModel->AttributeCount();
	}

	void CGraphModelGraphView::GetAttribute(size_t index, QString& out_name, QVariant& out_value) const
	{
		out_name = m_DataModel->AttributeName((CGraphModel::attribute_index_t)index);
		out_value = m_DataModel->AttributeValue((CGraphModel::attribute_index_t)index);
	}

	size_t CGraphModelGraphView::NodeCount() const
	{
		return m_DataModel->NodeCount();
	}

	size_t CGraphModelGraphView::NodeAttributeCount() const
	{
		return std::size(s_NodeAttributeDescs) + m_DataModel->NodeAttributeCount();
	}

	SNodeAttributeDesc CGraphModelGraphView::NodeAttributeDesc(size_t index) const
	{
		if (index < std::size(s_NodeAttributeDescs))
		{
			return s_NodeAttributeDescs[index];
		}
		QString name;
		auto& node_attribute = m_DataModel->NodeAttribute(index - std::size(s_NodeAttributeDescs), &name);
		return { name, node_attribute.Type() };
	}

	void CGraphModelGraphView::GetNodeAttributeData(size_t index, void* buffer, size_t size) const
	{
		if (index < std::size(s_NodeAttributeDescs))
		{
			if (0 == index)
			{
				ASSERT(NodeCount() * sizeof(QPointF) == size);
				auto* to = (QPointF*)buffer;
				for (size_t node_index = 0; node_index < m_DataModel->NodeCount(); ++node_index)
				{
					*to++ = m_DataModel->NodePosition((CGraphModel::node_index_t)node_index);
				}
			}
			else if (1 == index)
			{
				ASSERT(NodeCount() * sizeof(uint32_t) == size);
				auto* to = (uint32_t*)buffer;
				for (size_t node_index = 0; node_index < m_DataModel->NodeCount(); ++node_index)
				{
					*to++ = m_DataModel->NodeCategory((CGraphModel::node_index_t)node_index);
				}
			}
		}
		else
		{
			auto& node_attribute = m_DataModel->NodeAttribute(index - std::size(s_NodeAttributeDescs));
			node_attribute.Copy(buffer, size);
		}
	}

	size_t CGraphModelGraphView::EdgeCount() const
	{
		return m_DataModel->EdgeCount();
	}

	void CGraphModelGraphView::GetEdges(std::span<std::pair<uint32_t, uint32_t>> out_edges) const
	{
		size_t out_edge_index = 0;
		for (CGraphModel::node_index_t node_index = 0; node_index < (CGraphModel::node_index_t)m_DataModel->NodeCount(); ++node_index)
		{
			for (auto neighbour_node_index : m_DataModel->NodeNeighbours(node_index))
			{
				if (neighbour_node_index > node_index)
				{
					out_edges[out_edge_index++] = std::make_pair(node_index, neighbour_node_index);
				}
			}
		}
		ASSERT(out_edges.size() == out_edge_index);
	}
}