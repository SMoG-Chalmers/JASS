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