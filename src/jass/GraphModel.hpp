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

#include <algorithm>
#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include <QtCore/qobject.h>
#include <QtGui/qrgb.h>
#include <QtCore/qpoint.h>
#include <QtCore/qstring.h>

#include <jass/graphdata/GraphView.h>
#include <jass/utils/bitvec.h>
#include <jass/Debug.h>

namespace jass
{
	enum EShape
	{
		EShape_Circle,
		EShape_Triangle,
		EShape_Square,
		EShape_Diamond,
		EShape_Pentagon,
		EShape_Hexagon,
		EShape_Star,

		EShape_COUNT
	};
	
	struct SNodeDesc
	{
		uint32_t Index;
		uint32_t Category;
		QPoint Pos;
	};

	struct SEdgeDesc
	{
		uint32_t Index;
		uint32_t Node0;
		uint32_t Node1;
	};

	class CGraphModel: public QObject
	{
		Q_OBJECT
	public:
		typedef QRgb color_t;
		typedef uint32_t index_t;
		typedef index_t node_index_t;
		typedef index_t edge_index_t;
		typedef uint32_t category_index_t;
		typedef QPoint position_t;
		typedef std::pair<node_index_t, node_index_t> node_pair_t;
		typedef std::span<const node_index_t> const_node_indices_t;
		typedef std::span<const edge_index_t> const_edge_indices_t;
		typedef std::span<const node_index_t> node_remap_table_t;
		typedef std::span<const node_pair_t> const_node_pairs_t;

		static const node_index_t NO_NODE = (node_index_t)-1;
		static const category_index_t NO_CATEGORY = (category_index_t)-1;

		struct SCategory
		{
			color_t Color;
			EShape Shape;
		};

		CGraphModel();

		inline node_index_t NodeCount() const { return (node_index_t)m_NodePositions.size(); }

		inline node_index_t EdgeCount() const { return (node_index_t)m_NeighboursPerNode.size() >> 1; }

		node_index_t AddNodes(size_t count);

		void BeginModifyNodes();
		
		void EndModifyNodes();

		inline const position_t& NodePosition(node_index_t node_index) const { return m_NodePositions[node_index]; }

		inline void SetNodePosition(node_index_t node_index, const position_t& position) { m_NodePositions[node_index] = position; SetNodeModified(node_index); }
		inline void SetNodePosition(node_index_t node_index, const QPointF& position) { m_NodePositions[node_index] = QPoint((int)std::round(position.x()), (int)std::round(position.y())); SetNodeModified(node_index); }

		inline category_index_t NodeCategory(node_index_t node_index) const { return m_NodeCategories[node_index]; }

		inline void SetNodeCategory(node_index_t node_index, category_index_t category_index) { m_NodeCategories[node_index] = category_index; SetNodeModified(node_index); }

		inline const QString& NodeName(node_index_t node_index) const { return m_NodeNames[node_index]; }

		inline void SetNodeName(node_index_t node_index, const QString& name) { m_NodeNames[node_index] = name; SetNodeModified(node_index); }

		inline std::span<const node_index_t> NodeNeighbours(node_index_t node_index) const;

		template <class TLambda> void ForEachEdgeFromNode(node_index_t node_index, TLambda&&) const;

		void InsertNodes(const std::span<const SNodeDesc>& nodes);

		void RemoveNodes(const const_node_indices_t& node_indices);

		void AddEdges(const std::span<const node_pair_t>& edges);

		void InsertEdges(const std::span<const SEdgeDesc>& edges);

		void RemoveEdges(const std::span<const edge_index_t>& edge_indices);

		inline bool TryGetEdgeFromNodePair(const node_pair_t& node_pair, edge_index_t& our_edge_index) const;

		inline edge_index_t EdgeFromNodePair(const node_pair_t& node_pair) const;

		inline node_pair_t EdgeNodePair(edge_index_t edge_index) const;

	Q_SIGNALS:
		void NodesInserted(const const_node_indices_t& node_indices, const node_remap_table_t& remap_table);
		void NodesRemoved(const const_node_indices_t& node_indices, const node_remap_table_t& remap_table);
		void EdgesAdded(size_t count);
		void EdgesInserted(const const_edge_indices_t& edge_indices, const node_remap_table_t& remap_table);
		void EdgesRemoved(const const_edge_indices_t& edge_indices);
		void NodesModified(const bitvec& node_mask);

	private:
		typedef size_t edge_key_t;
		typedef std::unordered_map<edge_key_t, edge_index_t> edge_map_t;

		void RebuildNeighbourTables(const std::span<const node_pair_t>& edges);

		inline void SetNodeModified(node_index_t node_index) { VerifyModifyingNodes(); m_NodeModificationMask.set(node_index); }
		inline void VerifyModifyingNodes() const { ASSERT(!m_NodeModificationMask.empty()); }

		static void RebuildEdgeMap(edge_map_t& edge_map, const const_node_pairs_t& node_pairs);

		inline static edge_key_t MakeEdgeMapKey(const node_pair_t& node_pair);

		std::vector<QString> m_NodeNames;
		std::vector<position_t> m_NodePositions;
		std::vector<category_index_t> m_NodeCategories;
		std::vector<node_index_t> m_FirstEdgePerNode;  // has one extra element!
		std::vector<node_index_t> m_NeighboursPerNode;
		std::vector<node_pair_t> m_Edges;
		std::unordered_map<edge_key_t, edge_index_t> m_EdgeMap;
		std::vector<index_t> m_TempIndices;
		bitvec m_NodeModificationMask;
	};

	inline std::span<const CGraphModel::node_index_t> CGraphModel::NodeNeighbours(node_index_t node_index) const
	{
		return std::span<const node_index_t>(
			m_NeighboursPerNode.data() + m_FirstEdgePerNode[node_index],
			m_NeighboursPerNode.data() + m_FirstEdgePerNode[node_index + 1]);
	}

	template <class TLambda> void CGraphModel::ForEachEdgeFromNode(node_index_t node_index, TLambda&& fn) const
	{
		for (auto neighbour_index : NodeNeighbours((CGraphModel::node_index_t)node_index))
		{
			const auto edge_index = EdgeFromNodePair(CGraphModel::node_pair_t((CGraphModel::node_index_t)node_index, neighbour_index));
			fn(edge_index, neighbour_index);
		}
	}

	inline bool CGraphModel::TryGetEdgeFromNodePair(const node_pair_t& node_pair, edge_index_t& out_edge_index) const
	{
		auto it = m_EdgeMap.find(MakeEdgeMapKey(node_pair));
		if (m_EdgeMap.end() == it)
		{
			return false;
		}
		out_edge_index = it->second;
		return true;
	}

	inline CGraphModel::edge_index_t CGraphModel::EdgeFromNodePair(const node_pair_t& node_pair) const
	{
		edge_index_t edge_index;
		VERIFY(TryGetEdgeFromNodePair(node_pair, edge_index));
		return edge_index;
	}

	inline CGraphModel::node_pair_t CGraphModel::EdgeNodePair(edge_index_t edge_index) const
	{
		return m_Edges[edge_index];
	}

	inline CGraphModel::edge_key_t CGraphModel::MakeEdgeMapKey(const node_pair_t& node_pair)
	{
		return ((size_t)std::min(node_pair.first, node_pair.second) << 32) | std::max(node_pair.first, node_pair.second);
	}


	// CGraphSelectionModel

	class CGraphSelectionModel: public QObject
	{
		Q_OBJECT
	public:
		typedef CGraphModel::node_index_t node_index_t;

		CGraphSelectionModel(CGraphModel& data_model);

		inline bitvec_set_bit_indices_view<node_index_t> SelectedNodeIndicesView() const;

		inline const bitvec& NodeMask() const { return m_NodeMask; }
		
		inline const bitvec& EdgeMask() const { return m_EdgeMask; }
		
		inline bool Empty() const { return !AnySelected(); }

		inline bool AnySelected() const { return AnyNodesSelected() || AnyEdgesSelected(); }

		inline bool AnyNodesSelected() const { return SelectedNodeCount() > 0; }

		inline bool AnyEdgesSelected() const { return SelectedEdgeCount() > 0; }

		inline size_t SelectedNodeCount() const { return m_NodeMask.count_set_bits(); }

		inline size_t SelectedEdgeCount() const { return m_EdgeMask.count_set_bits(); }

		inline bool IsNodeSelected(size_t index) const { return m_NodeMask.get(index); }

		inline bool IsEdgeSelected(size_t index) const { return m_EdgeMask.get(index); }

		inline void BeginModify() { ASSERT(!m_IsModifying);  m_IsModifying = true; }

		inline void DeselectAll();

		inline void DeselectAllNodes();

		inline void DeselectAllEdges();

		inline void SelectNode(size_t index);

		inline void SelectEdge(size_t index);

		inline void DeselectNode(size_t index);

		inline void DeselectEdge(size_t index);

		void SetNodeMask(const bitvec& mask);

		void SetEdgeMask(const bitvec& mask);

		inline void EndModify();

	Q_SIGNALS:
		void SelectionChanged();

	private Q_SLOTS:
		void OnNodesInserted(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table);
		void OnNodesRemoved(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table);
		void OnEdgesInserted(const CGraphModel::const_edge_indices_t& edge_indices, const CGraphModel::node_remap_table_t& remap_table);
		void OnEdgesRemoved(const CGraphModel::const_edge_indices_t& edge_indices);

	private:
		inline void VerifyModifying() const { ASSERT(m_IsModifying); }

		CGraphModel& m_DataModel;
		bitvec m_NodeMask;
		bitvec m_EdgeMask;
		bool m_IsModifying = false;
	};

	inline bitvec_set_bit_indices_view<CGraphSelectionModel::node_index_t> CGraphSelectionModel::SelectedNodeIndicesView() const
	{
		return bitvec_set_bit_indices_view<node_index_t>(m_NodeMask); 
	}

	inline void CGraphSelectionModel::DeselectAll()
	{
		DeselectAllNodes();
		DeselectAllEdges();
	}

	inline void CGraphSelectionModel::DeselectAllNodes()
	{
		VerifyModifying();
		m_NodeMask.clearAll();
	}

	inline void CGraphSelectionModel::DeselectAllEdges()
	{
		VerifyModifying();
		m_EdgeMask.clearAll();
	}

	inline void CGraphSelectionModel::SelectNode(size_t index)
	{
		VerifyModifying();
		m_NodeMask.set(index);
	}

	inline void CGraphSelectionModel::SelectEdge(size_t index)
	{
		VerifyModifying();
		m_EdgeMask.set(index);
	}

	inline void CGraphSelectionModel::DeselectNode(size_t index)
	{
		VerifyModifying();
		m_NodeMask.clear(index);
	}

	inline void CGraphSelectionModel::DeselectEdge(size_t index)
	{
		VerifyModifying();
		m_EdgeMask.clear(index);
	}

	inline void CGraphSelectionModel::EndModify()
	{
		VerifyModifying();
		m_IsModifying = false;
		emit SelectionChanged();
	}
}