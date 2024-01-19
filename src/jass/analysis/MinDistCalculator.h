#pragma once

#include <numeric>
#include <span>
#include "BfsTraversal.h"

namespace jass
{
	template <typename TGraph>
	class CMinDistCalculator
	{
	public:
		template <typename TDistance>
		void CalculateMinimumDistances(const TGraph& graph, size_t src_node_index, std::span<TDistance>& out_node_depths)
		{
			const auto MAX_DISTANCE = std::numeric_limits<TDistance>::max();
			const size_t node_count = graph.NodeCount();
			for (size_t node_index = 0; node_index < node_count; ++node_index)
			{
				out_node_depths[node_index] = MAX_DISTANCE;
			}

			m_BfsTraversal.Traverse(graph, src_node_index, [&](auto node, auto depth)
			{
				out_node_depths[graph.NodeIndex(node)] = (TDistance)depth;
			});
		}

	private:
		CBfsTraversal<TGraph> m_BfsTraversal;
	};
}