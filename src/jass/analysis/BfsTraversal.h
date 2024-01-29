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