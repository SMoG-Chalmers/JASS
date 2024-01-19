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

#include <jass/analysis/ImmutableDirectedGraph.h>
#include <jass/GraphModel.hpp>

namespace jass
{
	class CGraphModelImmutableDirectedGraphAdapter
	{
	public:
		typedef CGraphModel::node_index_t node_index_t;
		typedef CGraphModel::node_index_t node_handle_t;
		typedef CGraphModel::node_index_t edge_handle_t;

		CGraphModelImmutableDirectedGraphAdapter(const CGraphModel& graph_model)
			: m_GraphModel(graph_model) {}

		class NodeList
		{
		public:
			typedef CGraphModelImmutableDirectedGraphAdapter::node_index_t node_index_t;

			NodeList(node_index_t count) : m_Count(count) {}

			class Iterator
			{
			public:
				Iterator(node_index_t index) : m_Index(index) {}
				inline const node_index_t operator*() const { return m_Index; }
				inline Iterator& operator++() { ++m_Index; return *this; }
				inline bool operator!=(const Iterator& other) const { return m_Index != other.m_Index; }
			private:
				node_index_t m_Index;
			};

			inline Iterator begin() const { return Iterator(0); }
			inline Iterator end() const { return Iterator(m_Count); }
		private:
			node_index_t m_Count;
		};

		inline size_t        NodeCount() const { return m_GraphModel.NodeCount(); }
		inline node_handle_t NodeFromIndex(node_index_t index) const { return index; }
		inline NodeList      Nodes() const { return NodeList(m_GraphModel.NodeCount()); }
		inline node_index_t  NodeIndex(node_handle_t node) const { return node; }
		inline node_index_t  Node(const node_handle_t node) const { return node; }
		inline node_index_t  NodeEdgeCount(const node_handle_t node) const { return (node_index_t)m_GraphModel.NodeNeighbours(node).size(); }
		inline std::span<const node_index_t> NodeEdges(const node_handle_t node)  const { return m_GraphModel.NodeNeighbours(node); }
		inline node_handle_t EdgeTargetNode(const edge_handle_t edge) const { return edge; }
		inline node_handle_t EdgeTargetNodeIndex(const edge_handle_t edge) const { return edge; }

	private:
		const CGraphModel& m_GraphModel;
	};
}