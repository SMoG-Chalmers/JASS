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

#include <algorithm>
#include <ranges>
#include <jass/utils/range_utils.h>
#include "Debug.h"
#include "GraphModel.hpp"
#include "GraphUtils.h"

namespace jass
{
	const CGraphModel::attribute_index_t CGraphModel::NO_ATTRIBUTE = (CGraphModel::attribute_index_t)-1;
	const CGraphModel::node_index_t CGraphModel::NO_NODE = (CGraphModel::node_index_t)-1;
	const CGraphModel::category_index_t CGraphModel::NO_CATEGORY = (CGraphModel::category_index_t)-1;

	CGraphModel::CGraphModel()
	{
		m_FirstEdgePerNode.push_back(0);  // has one extra element!
	}

	CGraphModel::node_index_t CGraphModel::AddNodes(size_t count)
	{
		const auto new_node_count = NodeCount() + count;
		m_NodeNames.resize(new_node_count);
		m_NodePositions.resize(new_node_count);
		m_NodeCategories.resize(new_node_count, NO_CATEGORY);
		for (auto& node_attribute : m_NodeAttributes)
		{
			node_attribute.second->Resize(new_node_count);
		}
		
		m_FirstEdgePerNode.resize(new_node_count + 1, m_FirstEdgePerNode.back());  // has one extra element!

		return (CGraphModel::node_index_t)(new_node_count - count);
	}

	void CGraphModel::AddAttribute(const QString& name, const QVariant& value)
	{
		if (FindAttribute(name) != NO_ATTRIBUTE)
		{
			throw TFormatException<std::runtime_error>("Trying to add graph attribute '%s' twice.", name.toStdString().c_str());
		}
		m_Attributes.push_back({ name, value });
	}

	CGraphModel::attribute_index_t CGraphModel::FindAttribute(const QString& name) const
	{
		for (size_t i = 0; i < m_Attributes.size(); ++i)
		{
			if (name == m_Attributes[i].first)
			{
				return (attribute_index_t)i;
			}
		}
		return NO_ATTRIBUTE;
	}

	void CGraphModel::SetAttribute(attribute_index_t index, const QVariant& value)
	{
		if (value == m_Attributes[index].second)
		{
			return;
		}
		m_Attributes[index].second = value;
		emit AttributeChanged(index, value);
	}

	CGraphModel::attribute_index_t CGraphModel::AttributeCount() const
	{
		return m_Attributes.size();
	}

	const QString& CGraphModel::AttributeName(CGraphModel::attribute_index_t index) const
	{
		return m_Attributes[index].first;
	}

	const QVariant& CGraphModel::AttributeValue(CGraphModel::attribute_index_t index) const
	{
		return m_Attributes[index].second;
	}

	size_t CGraphModel::NodeAttributeCount() const
	{
		return m_NodeAttributes.size();
	}

	const CNodeAttributeBase& CGraphModel::NodeAttribute(size_t index, QString* out_name) const
	{
		const auto& node_attribute = m_NodeAttributes[index];
		if (out_name)
		{
			*out_name = node_attribute.first;
		}
		return *node_attribute.second;
	}

	CNodeAttributeBase* CGraphModel::FindNodeAttribute(const QString& name)
	{
		for (auto& node_attribute : m_NodeAttributes)
		{
			if (node_attribute.first == name)
			{
				return node_attribute.second.get();
			}
		}
		return nullptr;
	}

	void CGraphModel::BeginModifyNodes()
	{
		if (0 == m_NodeModificationCounter)
		{
			m_NodeModificationMask.clear();
			m_NodeModificationMask.resize(NodeCount());
		}
		++m_NodeModificationCounter;
	}

	void CGraphModel::EndModifyNodes()
	{
		VerifyModifyingNodes();
		--m_NodeModificationCounter;
		if (0 == m_NodeModificationCounter)
		{
			emit NodesModified(m_NodeModificationMask);
			m_NodeModificationMask.clear();
		}
	}

	void CGraphModel::InsertNodes(const std::span<const SNodeDesc>& new_nodes)
	{
		decltype(m_TempIndices) temp_indices = std::move(m_TempIndices);  // Hold m_TempIndices in this scope, in case it is accessed 

		temp_indices.resize(new_nodes.size() + NodeCount());
		for (size_t i = 0; i < new_nodes.size(); ++i)
		{
			temp_indices[i] = new_nodes[i].Index;
		}
		auto new_node_indices = std::span<const node_index_t>(temp_indices.data(), new_nodes.size());
		auto remap_table = std::span<node_index_t>(temp_indices.data() + new_nodes.size(), NodeCount());
		build_index_expand_table(new_node_indices, remap_table);

		expand(m_NodeNames, new_node_indices);
		expand(m_NodePositions, new_node_indices);
		expand(m_NodeCategories, new_node_indices);
		expand(m_FirstEdgePerNode, new_node_indices, (node_index_t)-1);
		for (auto& node_attribute : m_NodeAttributes)
		{
			node_attribute.second->Expand(new_node_indices);
		}

		for (auto& new_node : new_nodes)
		{
			m_NodeNames[new_node.Index] = QString();  // TODO: Fix
			m_NodePositions[new_node.Index] = QPoint((int)std::round(new_node.PositionF.x()), (int)std::round(new_node.PositionF.y()));
			m_NodeCategories[new_node.Index] = new_node.Category;  // TODO: Fix
		}
		
		for (auto it = new_node_indices.rbegin(); new_node_indices.rend() != it; ++it)
		{
			m_FirstEdgePerNode[*it] = m_FirstEdgePerNode[*it + 1];
		}

		// Remap node indices in edge array
		for (auto& edge : m_Edges)
		{
			edge.first = remap_table[edge.first];
			edge.second = remap_table[edge.second];
		}

		RebuildEdgeMap(m_EdgeMap, m_Edges);

		remap_indices(to_span(m_NeighboursPerNode), std::span<const node_index_t>(remap_table.data(), remap_table.size()));
		emit NodesInserted(new_node_indices, remap_table);

		m_TempIndices = std::move(temp_indices);  // Release our hold on m_TempIndices
	}

	void CGraphModel::RemoveNodes(const const_node_indices_t& node_indices)
	{
		const auto old_node_count = NodeCount();
		
		// Remove edges connected to the nodes
		{
			std::vector<edge_index_t> edges;
			GetEdgesConnectedToNodes(*this, node_indices, edges);
			if (!edges.empty())
			{
				RemoveEdges(to_const_span(edges));
			}
		}

		collapse(m_NodeNames, node_indices);
		collapse(m_NodePositions, node_indices);
		collapse(m_NodeCategories, node_indices);
		collapse(m_FirstEdgePerNode, node_indices);
		for (auto& node_attribute : m_NodeAttributes)
		{
			node_attribute.second->Collapse(node_indices);
		}

		// Remap
		{
			std::vector<node_index_t> remap_table;
			build_index_collapse_table((node_index_t)old_node_count, node_indices, remap_table);
			remap_indices(to_span(m_NeighboursPerNode), to_const_span(remap_table));
			
			// Remap node indices in edge array
			for (auto& edge : m_Edges)
			{
				edge.first = remap_table[edge.first];
				edge.second = remap_table[edge.second];
			}

			// Rebuild edge map
			RebuildEdgeMap(m_EdgeMap, m_Edges);

			emit NodesRemoved(node_indices, to_const_span(remap_table));
		}
	}

	void CGraphModel::AddEdges(const std::span<const node_pair_t>& edges)
	{
		m_Edges.reserve(m_Edges.size() + edges.size());
		m_Edges.insert(m_Edges.end(), edges.begin(), edges.end());

		RebuildNeighbourTables(m_Edges);

		// Add to edge map
		for (size_t edge_index = m_Edges.size() - edges.size(); edge_index < m_Edges.size(); ++edge_index)
		{
			m_EdgeMap.insert(std::make_pair(MakeEdgeMapKey(m_Edges[edge_index]), (edge_index_t)edge_index));
		}

		emit EdgesAdded(edges.size());
	}

	void CGraphModel::InsertEdges(const std::span<const SEdgeDesc>& new_edges)
	{
		{
			// TEMPORARILY append edges to m_Edges to be able to rebuild neighbour tables
			const auto old_edge_count = EdgeCount();
			m_Edges.reserve(old_edge_count + new_edges.size());
			for (const auto& edge_desc : new_edges)
			{
				m_Edges.push_back({ edge_desc.Node0, edge_desc.Node1 });
			}
			RebuildNeighbourTables(m_Edges);
			m_Edges.resize(old_edge_count);
		}

		// Update m_Edges and m_EdgeMap
		{
			decltype(m_TempIndices) temp_indices = std::move(m_TempIndices);  // Hold m_TempIndices in this scope, in case it is accessed 

			temp_indices.resize(new_edges.size() + NodeCount());
			for (size_t i = 0; i < new_edges.size(); ++i)
			{
				temp_indices[i] = new_edges[i].Index;
			}
			auto new_edge_indices = std::span<const node_index_t>(temp_indices.data(), new_edges.size());
			auto remap_table = std::span<node_index_t>(temp_indices.data() + new_edges.size(), EdgeCount());
			build_index_expand_table(new_edge_indices, remap_table);

			// Remap edge indices in edge map
			for (auto it = m_EdgeMap.begin(); m_EdgeMap.end() != it; ++it)
			{
				it->second = remap_table[it->second];
			}

			// insert new edges into edge array and edge map
			expand(m_Edges, new_edge_indices);
			for (auto& new_edge : new_edges)
			{
				const auto node_pair = node_pair_t(
					std::min(new_edge.Node0, new_edge.Node1),
					std::max(new_edge.Node0, new_edge.Node1)
				);

				m_Edges[new_edge.Index] = node_pair;
				
				VERIFY(m_EdgeMap.insert(std::make_pair(MakeEdgeMapKey(node_pair), new_edge.Index)).second);
			}

			ASSERT(m_Edges.size() == m_NeighboursPerNode.size() / 2);
			ASSERT(m_EdgeMap.size() == m_Edges.size());

			emit EdgesInserted(new_edge_indices, remap_table);

			m_TempIndices = std::move(temp_indices);  // Release our hold on m_TempIndices
		}
	}

	void CGraphModel::RemoveEdges(const std::span<const edge_index_t>& edge_indices)
	{
		const auto INVALID_NODE_PAIR = node_pair_t(NO_NODE, NO_NODE);

		// Mark deleted edges
		for (const auto edge_index : edge_indices)
		{
			const auto& node_pair = m_Edges[edge_index];

			// Mark for deletion in m_NeighboursPerNode
			for (auto to_index = m_FirstEdgePerNode[node_pair.first]; to_index < m_FirstEdgePerNode[node_pair.first + 1]; ++to_index)
			{
				if (m_NeighboursPerNode[to_index] == node_pair.second)
				{
					m_NeighboursPerNode[to_index] = NO_NODE;
					break;
				}
			}
			for (auto from_index = m_FirstEdgePerNode[node_pair.second]; from_index < m_FirstEdgePerNode[node_pair.second + 1]; ++from_index)
			{
				if (m_NeighboursPerNode[from_index] == node_pair.first)
				{
					m_NeighboursPerNode[from_index] = NO_NODE;
					break;
				}
			}

			// Remove from edge map
			auto it = m_EdgeMap.find(MakeEdgeMapKey(node_pair));
			ASSERT(m_EdgeMap.end() != it);
			m_EdgeMap.erase(it);
		}

		std::vector<node_index_t> remap_table;
		build_index_collapse_table((edge_index_t)m_Edges.size(), edge_indices, remap_table);

		// Collapse node neighbour array
		{
			node_index_t from_index = 0;
			node_index_t to_index = 0;
			for (node_index_t node_index = 0; node_index < NodeCount(); ++node_index)
			{
				m_FirstEdgePerNode[node_index] = to_index;
				for (; from_index < m_FirstEdgePerNode[node_index + 1]; ++from_index)
				{
					if (m_NeighboursPerNode[from_index] != NO_NODE)
					{
						m_NeighboursPerNode[to_index++] = m_NeighboursPerNode[from_index];
					}
				}
			}
			m_FirstEdgePerNode.back() = to_index;
			m_NeighboursPerNode.resize(to_index);
		}

		// Collapse edges array
		collapse(m_Edges, edge_indices);

		// Remap indices edge map
		for (auto it : m_EdgeMap)
		{
			it.second = remap_table[it.second];
		}

		ASSERT(m_Edges.size() == m_NeighboursPerNode.size() / 2);
		ASSERT(m_EdgeMap.size() == m_Edges.size());

		emit EdgesRemoved(edge_indices);
	}

	void CGraphModel::RebuildEdgeMap(edge_map_t& edge_map, const const_node_pairs_t& node_pairs)
	{
		edge_map.clear();
		edge_map.reserve(node_pairs.size());
		for (edge_index_t edge_index = 0; edge_index < (edge_index_t)node_pairs.size(); edge_index++)
		{
			VERIFY(edge_map.insert(std::make_pair(MakeEdgeMapKey(node_pairs[edge_index]), edge_index)).second);
		}
	}

	void CGraphModel::OnCatagoriesRemapped(const std::span<const size_t>& remap_table)
	{
		BeginModifyNodes();
		for (CGraphModel::node_index_t node_index = 0; node_index < NodeCount(); ++node_index)
		{
			const auto category = NodeCategory(node_index);
			if (NO_CATEGORY != category)
			{
				SetNodeCategory(node_index, remap_table[category]);
			}
		}
		EndModifyNodes();
	}

	void CGraphModel::RebuildNeighbourTables(const std::span<const node_pair_t>& edges)
	{
		m_TempIndices.resize(edges.size() * 2 * 2);
		typedef decltype(m_TempIndices)::value_type temp_index_t;
		typedef std::pair<temp_index_t, temp_index_t> temp_pair_t;
		std::span<temp_pair_t> temp_pairs((temp_pair_t*)m_TempIndices.data(), edges.size() * 2);

		{
			size_t n = 0;
			for (const auto& edge : edges)
			{
				temp_pairs[n++] = { edge.first, edge.second };
				temp_pairs[n++] = { edge.second, edge.first };
			}
			ASSERT(n == edges.size() * 2);
		}

		std::sort(temp_pairs.begin(), temp_pairs.end());

		m_NeighboursPerNode.resize(temp_pairs.size());

		{
			size_t n = 0;
			edge_index_t edge_count = 0;
			for (node_index_t node_index = 0; node_index < NodeCount(); ++node_index)
			{
				m_FirstEdgePerNode[node_index] = (node_index_t)n;
				for (; n < temp_pairs.size() && temp_pairs[n].first == node_index; ++n)
				{
					m_NeighboursPerNode[n] = temp_pairs[n].second;
				}
			}
			m_FirstEdgePerNode.back() = (node_index_t)n;  // has one extra element!
		}
	}


	// CGraphSelectionModel

	CGraphSelectionModel::CGraphSelectionModel(CGraphModel& data_model)
		: m_DataModel(data_model)
	{
		m_NodeMask.resize(data_model.NodeCount());
		m_EdgeMask.resize(data_model.EdgeCount());
		connect(&m_DataModel, &CGraphModel::NodesInserted, this, &CGraphSelectionModel::OnNodesInserted);
		connect(&m_DataModel, &CGraphModel::NodesRemoved,  this, &CGraphSelectionModel::OnNodesRemoved);
		connect(&m_DataModel, &CGraphModel::EdgesInserted, this, &CGraphSelectionModel::OnEdgesInserted);
		connect(&m_DataModel, &CGraphModel::EdgesRemoved,  this, &CGraphSelectionModel::OnEdgesRemoved);
	}

	CGraphSelectionModel::node_index_t CGraphSelectionModel::FirstSelected() const
	{
		CGraphModel::node_index_t first_selected_index = CGraphModel::NO_NODE;
		NodeMask().for_each_set_bit([&](auto node_index)
			{
				first_selected_index = (CGraphModel::node_index_t)node_index;
			});
		return first_selected_index;
	}

	void CGraphSelectionModel::SetNodeMask(const bitvec& mask) 
	{
		VerifyModifying(); 
		m_NodeMask = mask; 
		m_NodeMask.resize(m_DataModel.NodeCount()); 
	}

	void CGraphSelectionModel::SetEdgeMask(const bitvec& mask)
	{
		VerifyModifying();
		m_EdgeMask = mask;
		m_EdgeMask.resize(m_DataModel.EdgeCount());
	}

	void CGraphSelectionModel::OnNodesInserted(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table)
	{
		m_NodeMask.resize(m_DataModel.NodeCount());
	}

	void CGraphSelectionModel::OnNodesRemoved(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table)
	{
		m_NodeMask.resize(m_DataModel.NodeCount());
	}

	void CGraphSelectionModel::OnEdgesInserted(const CGraphModel::const_edge_indices_t& edge_indices, const CGraphModel::node_remap_table_t& remap_table)
	{
		m_EdgeMask.resize(m_DataModel.EdgeCount());
	}

	void CGraphSelectionModel::OnEdgesRemoved(const CGraphModel::const_edge_indices_t& edge_indices)
	{
		m_EdgeMask.resize(m_DataModel.EdgeCount());
	}
}

#include <moc_GraphModel.cpp>
