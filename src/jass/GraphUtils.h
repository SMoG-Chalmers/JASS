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

#include <algorithm>
#include <jass/utils/range_utils.h>
#include "GraphModel.hpp"

namespace jass
{
	template <class TNodeIndexSet, class TEdgeContainer>
	void GetEdgesConnectedToNodes(CGraphModel& graph_model, const TNodeIndexSet& node_indices, TEdgeContainer& out_edges)
	{
		out_edges.clear();
		for (auto node_index0 : node_indices)
		{
			for (auto node_index1 : graph_model.NodeNeighbours(node_index0))
			{
				out_edges.push_back(graph_model.EdgeFromNodePair(CGraphModel::node_pair_t(node_index0, node_index1)));
			}
		}
		std::sort(out_edges.begin(), out_edges.end());
		out_edges.resize(remove_duplicates_from_ordered(out_edges.begin(), out_edges.end()));
	}
}