#pragma once

#include <numeric>
#include <span>
#include "BfsTraversal.h"

namespace jass
{
	template <typename TGraph>
	class CDepthCalculator
	{
	public:
		// Source node is included in 'out_node_count'
		void CalculateDepth(const TGraph& graph, size_t node_index, size_t& out_max_depth, size_t& out_total_depth, size_t& out_node_count)
		{
			size_t max_depth = 0, depth_sum = 0, node_count = 0;
			m_BfsTraversal.Traverse(graph, node_index, [&](auto node, auto depth)
			{
				max_depth = depth;
				depth_sum += depth;
				++node_count;
			});
			out_max_depth = max_depth;
			out_total_depth = depth_sum;
			out_node_count = node_count;
		}

	private:
		CBfsTraversal<TGraph> m_BfsTraversal;
	};
}