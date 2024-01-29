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

#include <jass/Debug.h>
#include <jass/GraphModel.hpp>
#include <jass/GraphUtils.h>

#include "GraphOperations.h"
#include "CmdCreateEdge.h"

namespace jass
{
	std::optional<CCmdCreateEdge> CCmdCreateEdge::Create(
		qapp::SCommandCreationContext& ctx, 
		CGraphModel& data_model, 
		CGraphModel::node_pair_t node_pair)
	{
		for (auto target_node : data_model.NodeNeighbours(node_pair.first))
		{
			if (target_node == node_pair.second)
			{
				// Edge already exists
				return {};
			}
		}

		WriteInsertGraphEdgesOp(ctx.m_Data, [&](auto&& add)
			{
				SEdgeDesc edge_desc;
				edge_desc.Index = data_model.EdgeCount();
				edge_desc.Node0 = std::min(node_pair.first, node_pair.second);
				edge_desc.Node1 = std::max(node_pair.first, node_pair.second);
				add(edge_desc);
			});

		return std::make_optional<CCmdCreateEdge>();
	}
}