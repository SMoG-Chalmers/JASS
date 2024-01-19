#pragma once

#include <queue>
#include <jass/utils/bitvec.h>

namespace jass
{
	template <class TGraph>
	class CBfsTraversal
	{
	public:
		typedef TGraph::node_index_t node_index_t;
		typedef TGraph::node_handle_t node_handle_t;

		template <class TFunc>
		void Traverse(const TGraph& graph, size_t start_node_index, TFunc fn)
		{
			m_VisitedMask.resize(graph.NodeCount());
			m_VisitedMask.clearAll();

			size_t num_remaining_in_frontier = 1;
			size_t depth = 0;

			m_Queue.push(graph.NodeFromIndex((node_index_t)start_node_index));
			m_VisitedMask.set(start_node_index);
			while (!m_Queue.empty())
			{
				auto node = m_Queue.front();
				m_Queue.pop();
				
				fn(node, depth);
				
				for (auto edge : graph.NodeEdges(node))
				{
					const auto target_node_index = graph.EdgeTargetNodeIndex(edge);
					if (m_VisitedMask.get(target_node_index))
					{
						continue;
					}
					m_VisitedMask.set(target_node_index);
					m_Queue.push(graph.EdgeTargetNode(edge));
				}

				if (0 == --num_remaining_in_frontier)
				{
					num_remaining_in_frontier = m_Queue.size();
					++depth;
				}
			}
		}

	private:
		jass::bitvec m_VisitedMask;
		std::queue<node_handle_t> m_Queue;
	};
}