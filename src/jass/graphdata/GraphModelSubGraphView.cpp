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
#include "GraphModelSubGraphView.h"

namespace jass
{
	static const SNodeAttributeDesc s_NodeAttributeDescs[] =
	{
		{ GRAPH_NODE_ATTTRIBUTE_POSITION, QVariant::PointF },
		{ GRAPH_NODE_ATTTRIBUTE_CATEGORY, QVariant::UInt },
	};

	CGraphModelSubGraphView::CGraphModelSubGraphView(const CGraphModel& data_model, const bitvec& node_mask)
		: m_DataModel(&data_model)
		, m_NodeMask(node_mask)
	{
		// Build node index map
		m_NodeIndexMap.reserve(node_mask.count_set_bits());
		uint32_t n = 0;
		node_mask.for_each_set_bit([&](size_t node_index)
			{
				m_NodeIndexMap.insert(std::make_pair((uint32_t)node_index, n++));
			});
	}

	size_t CGraphModelSubGraphView::AttributeCount() const
	{
		return 0;
	}

	void CGraphModelSubGraphView::GetAttribute(size_t index, QString& out_name, QVariant& out_value) const
	{
		ASSERT(false && "Currently no attribute support in CGraphModelSubGraphView");
	}

	size_t CGraphModelSubGraphView::NodeCount() const
	{
		return m_NodeIndexMap.size();
	}

	size_t CGraphModelSubGraphView::NodeAttributeCount() const
	{
		return std::size(s_NodeAttributeDescs);
	}

	SNodeAttributeDesc CGraphModelSubGraphView::NodeAttributeDesc(size_t index) const
	{
		ASSERT(index < std::size(s_NodeAttributeDescs));
		return s_NodeAttributeDescs[index];
	}

	void CGraphModelSubGraphView::GetNodeAttributeData(size_t index, void* buffer, size_t size) const
	{
		if (0 == index)
		{
			ASSERT(NodeCount() * sizeof(QPointF) == size);
			auto* to = (QPointF*)buffer;
			m_NodeMask.for_each_set_bit([&](size_t node_index)
				{
					*to++ = m_DataModel->NodePosition((CGraphModel::node_index_t)node_index);
				});
		}
		else if (1 == index)
		{
			ASSERT(NodeCount() * sizeof(uint32_t) == size);
			auto* to = (uint32_t*)buffer;
			m_NodeMask.for_each_set_bit([&](size_t node_index)
				{
					*to++ = m_DataModel->NodeCategory((CGraphModel::node_index_t)node_index);
				});
		}
	}

	size_t CGraphModelSubGraphView::EdgeCount() const
	{
		size_t count = 0;
		m_NodeMask.for_each_set_bit([&](size_t node_index)
		{
			for (auto neighbour_node_index : m_DataModel->NodeNeighbours((CGraphModel::node_index_t)node_index))
			{
				count += (size_t)(neighbour_node_index > node_index && m_NodeMask.get(neighbour_node_index));
			}
		});
		return count;
	}

	void CGraphModelSubGraphView::GetEdges(std::span<std::pair<uint32_t, uint32_t>> out_edges) const
	{
		size_t out_edge_index = 0;
		m_NodeMask.for_each_set_bit([&](size_t node_index)
		{
			const uint32_t out_node_index = m_NodeIndexMap.find((uint32_t)node_index)->second;
			for (auto neighbour_node_index : m_DataModel->NodeNeighbours((CGraphModel::node_index_t)node_index))
			{
				if (neighbour_node_index > node_index && m_NodeMask.get(neighbour_node_index))
				{
					out_edges[out_edge_index++] = std::make_pair(
						out_node_index,
						m_NodeIndexMap.find((uint32_t)neighbour_node_index)->second);
				}
			}
		});
		ASSERT(out_edges.size() == out_edge_index);
	}
}