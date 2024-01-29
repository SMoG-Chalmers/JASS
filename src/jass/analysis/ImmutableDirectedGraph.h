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

#include <cstdint>
#include <span>
#include <vector>

namespace jass
{
	/*
	concept GraphConcept
	{
	public:
		typedef ... node_index_t;
		typedef ... node_handle_t;
		typedef ... edge_handle_t;

		size_t        NodeCount() const;
		node_handle_t NodeFromIndex(node_index_t index) const;
		node_range_t  Nodes() const;
		node_index_t  NodeIndex(node_handle_t node) const;
		node_index_t  Node(const node_handle_t node) const;
		word_t        NodeEdgeCount(const node_handle_t node) const;
		edge_range_t  NodeEdges(const node_handle_t node) const;
		node_handle_t EdgeTargetNode(const edge_handle_t edge) const;
		node_handle_t EdgeTargetNodeIndex(const edge_handle_t edge) const;
	}
	*/

	class CImmutableDirectedGraph
	{
	public:
		typedef uint16_t word_t;
		typedef word_t node_index_t;

		struct SEdge
		{
		private:
			word_t TargetNodeIndex;
			word_t TargetNodeAddress;
			friend CImmutableDirectedGraph;
		};

		struct SNode
		{
		private:
			word_t Index;
			word_t EdgeCount;
			friend CImmutableDirectedGraph;
		};

		class NodeList;
		class EdgeList;

		typedef NodeList node_range_t;
		typedef EdgeList edge_range_t;

		typedef const void* node_handle_t;
		typedef const void* edge_handle_t;

		inline size_t NodeCount() const;

		inline node_handle_t NodeFromIndex(node_index_t index) const;

		inline node_range_t Nodes() const;

		inline static node_index_t NodeIndex(node_handle_t node);

		inline static word_t NodeEdgeCount(node_handle_t node);

		inline static edge_range_t NodeEdges(node_handle_t node);

		inline node_handle_t EdgeTargetNode(edge_handle_t edge) const;

		inline node_index_t EdgeTargetNodeIndex(edge_handle_t edge) const;

		template <class TGraphView>
		inline void CopyView(const TGraphView& view);

	private:
		typedef word_t node_address_t;

		static const size_t NODE_ALIGN_BYTES = sizeof(node_handle_t) * 2;

		inline void Clear();

		inline static SNode& NodeFromHandle(node_handle_t node);

		inline static node_handle_t HandleFromNode(const SNode& node);

		inline static SEdge& EdgeFromHandle(edge_handle_t edge);

		inline static size_t NodeByteSizeFromEdgeCount(word_t edge_count);

		inline static size_t NodeByteSize(const SNode& node);

		inline static size_t OffsetFromNodeAddress(node_address_t addr);

		inline node_handle_t NodeHandleFromAddress(node_address_t addr) const;

		inline SNode& NodeFromAddress(node_address_t addr);

		inline void ReserveSpace(size_t byte_size);

		size_t m_NodeCount = 0;
		void* m_NodeBuffer = nullptr;
		const void* m_NodeBufferEnd = nullptr;
		size_t m_NodeBufferSize = 0;
		std::vector<node_address_t> m_NodeAddresses;
	};

	inline size_t CImmutableDirectedGraph::NodeByteSize(const SNode& node)
	{
		return NodeByteSizeFromEdgeCount(node.EdgeCount);
	}

	inline size_t CImmutableDirectedGraph::OffsetFromNodeAddress(node_address_t addr)
	{
		return (size_t)addr * NODE_ALIGN_BYTES;
	}

	inline CImmutableDirectedGraph::node_handle_t CImmutableDirectedGraph::NodeHandleFromAddress(node_address_t addr) const
	{
		return (node_handle_t)((char*)m_NodeBuffer + OffsetFromNodeAddress(addr));
	}

	inline CImmutableDirectedGraph::SNode& CImmutableDirectedGraph::NodeFromAddress(node_address_t addr)
	{
		return NodeFromHandle(NodeHandleFromAddress(addr));
	}

	inline void CImmutableDirectedGraph::Clear()
	{
		m_NodeCount = 0;
		m_NodeBufferEnd = m_NodeBuffer;
		m_NodeBufferSize = 0;
		m_NodeAddresses.clear();
	}

	inline CImmutableDirectedGraph::SNode& CImmutableDirectedGraph::NodeFromHandle(node_handle_t node)
	{
		return *(SNode*)node;
	}

	inline CImmutableDirectedGraph::node_handle_t CImmutableDirectedGraph::HandleFromNode(const SNode& node)
	{
		return (CImmutableDirectedGraph::node_handle_t)&node;
	}

	inline CImmutableDirectedGraph::SEdge& CImmutableDirectedGraph::EdgeFromHandle(edge_handle_t edge)
	{
		return *(SEdge*)edge;
	}

	class CImmutableDirectedGraph::NodeList
	{
	public:
		NodeList(const void* beg, const void* end) : m_Beg(beg), m_End(end) {}

		class Iterator
		{
		public:
			Iterator(const SNode* ptr) : m_Ptr(ptr) {}

			const node_handle_t operator*() const
			{
				return m_Ptr;
			}

			Iterator& operator++()
			{
				m_Ptr = (SNode*)((const char*)m_Ptr + NodeByteSize(*m_Ptr));
				return *this;
			}

			bool operator!=(const Iterator& other) const
			{
				return m_Ptr != other.m_Ptr;
			}

		private:
			const SNode* m_Ptr;
		};

		inline Iterator begin() const { return Iterator((const SNode*)m_Beg); }

		inline Iterator end() const { return Iterator((const SNode*)m_End); }

	private:
		const void* m_Beg;
		const void* m_End;
	};

	class CImmutableDirectedGraph::EdgeList
	{
	public:
		EdgeList(const SEdge* edges, size_t count) : m_Beg(edges), m_End(edges + count) {}

		class Iterator
		{
		public:
			Iterator(const SEdge* ptr) : m_Ptr(ptr) {}

			const edge_handle_t operator*() const
			{
				return m_Ptr;
			}

			Iterator& operator++()
			{
				++m_Ptr;
				return *this;
			}

			bool operator!=(const Iterator& other) const
			{
				return m_Ptr != other.m_Ptr;
			}

		private:
			const SEdge* m_Ptr;
		};

		inline Iterator begin() const { return Iterator(m_Beg); }

		inline Iterator end() const { return Iterator(m_End); }

	private:
		const SEdge* m_Beg;
		const SEdge* m_End;
	};

	inline size_t CImmutableDirectedGraph::NodeCount() const
	{
		return m_NodeCount;
	}

	inline CImmutableDirectedGraph::node_handle_t CImmutableDirectedGraph::NodeFromIndex(node_index_t index) const
	{
		return const_cast<CImmutableDirectedGraph*>(this)->NodeHandleFromAddress(m_NodeAddresses[index]);
	}

	inline CImmutableDirectedGraph::NodeList CImmutableDirectedGraph::Nodes() const
	{
		return NodeList(m_NodeBuffer, m_NodeBufferEnd);
	}

	inline CImmutableDirectedGraph::node_index_t CImmutableDirectedGraph::NodeIndex(node_handle_t node)
	{
		return NodeFromHandle(node).Index;
	}

	inline CImmutableDirectedGraph::word_t CImmutableDirectedGraph::NodeEdgeCount(node_handle_t node)
	{
		return NodeFromHandle(node).EdgeCount;
	}

	inline CImmutableDirectedGraph::edge_range_t CImmutableDirectedGraph::NodeEdges(node_handle_t node)
	{
		return EdgeList(reinterpret_cast<const SEdge*>(&NodeFromHandle(node) + 1), NodeEdgeCount(node));
	}

	inline CImmutableDirectedGraph::node_handle_t CImmutableDirectedGraph::EdgeTargetNode(const edge_handle_t edge) const
	{
		return NodeHandleFromAddress(EdgeFromHandle(edge).TargetNodeAddress);
	}

	inline CImmutableDirectedGraph::node_index_t CImmutableDirectedGraph::EdgeTargetNodeIndex(const edge_handle_t edge) const
	{
		return EdgeFromHandle(edge).TargetNodeIndex;
	}

	inline size_t CImmutableDirectedGraph::NodeByteSizeFromEdgeCount(word_t edge_count)
	{
		return (sizeof(SNode) + edge_count * sizeof(SEdge) + (NODE_ALIGN_BYTES - 1)) & ~(NODE_ALIGN_BYTES - 1);
	}

	inline void CImmutableDirectedGraph::ReserveSpace(size_t byte_size)
	{
		if (byte_size <= m_NodeBufferSize)
		{
			return;
		}
		auto new_size = std::max(m_NodeBufferSize, (size_t)256);
		for (; new_size < byte_size; new_size <<= 1);
		m_NodeBuffer = realloc(m_NodeBuffer, new_size);
		m_NodeBufferSize = new_size;
	}

	template <class TGraphView>
	void CImmutableDirectedGraph::CopyView(const TGraphView& view)
	{
		Clear();

		m_NodeCount = view.NodeCount();
		m_NodeAddresses.resize(m_NodeCount);

		ReserveSpace(m_NodeCount * NODE_ALIGN_BYTES);

		{
			size_t at = 0;
			word_t node_index = 0;
			for (const auto& view_node : view.Nodes())
			{
				m_NodeAddresses[node_index] = (node_address_t)(at / NODE_ALIGN_BYTES);
				const auto edge_count = (word_t)view.NodeEdgeCount(view_node);
				const auto node_size = NodeByteSizeFromEdgeCount(edge_count);
				ReserveSpace(at + node_size);
				auto& my_node = *(SNode*)((char*)m_NodeBuffer + at);
				auto my_edges = std::span<SEdge>((SEdge*)(&my_node + 1), edge_count);
				at += node_size;
				my_node.Index = node_index;
				my_node.EdgeCount = edge_count;
				auto my_edge_it = my_edges.begin();
				for (const auto view_edge : view.NodeEdges(view_node))
				{
					my_edge_it->TargetNodeIndex = (word_t)view.NodeIndex(view.EdgeTargetNode(view_edge));
					my_edge_it->TargetNodeAddress = (word_t)-1;
					++my_edge_it;
				}
				++node_index;
			}
			m_NodeBufferEnd = (char*)m_NodeBuffer + at;
		}

		for (const auto node_handle : Nodes())
		{
			for (const auto edge_handle : NodeEdges(node_handle))
			{
				auto& edge = EdgeFromHandle(edge_handle);
				edge.TargetNodeAddress = m_NodeAddresses[edge.TargetNodeIndex];
			}
		}
	}
}