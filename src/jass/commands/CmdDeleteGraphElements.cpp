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

#include <jass/Debug.h>
#include <jass/GraphModel.hpp>
#include <jass/GraphUtils.h>
#include <jass/utils/range_utils.h>

#include "GraphOperations.h"
#include "CmdDeleteGraphElements.h"

namespace jass
{
	std::optional<CCmdDeleteGraphElements> CCmdDeleteGraphElements::Create(
		qapp::SCommandCreationContext& ctx, 
		CGraphModel& data_model, 
		CGraphSelectionModel& selection_model)
	{
		if (selection_model.Empty())
		{
			return {};
		}

		WriteDeleteGraphEdgesOp(ctx.m_Data, [&](auto&& del)
			{
				// Build vector of selected node indices
				// TODO: Can we get rid of this?
				std::vector<CGraphModel::node_index_t> node_indices;
				node_indices.reserve(selection_model.SelectedNodeCount());
				selection_model.NodeMask().for_each_set_bit([&](auto node_index) 
					{ 
						node_indices.push_back((CGraphModel::node_index_t)node_index); 
					});

				// Build list of edges from selected nodes
				std::vector<CGraphModel::edge_index_t> edge_indices;
				for (auto node_index0 : node_indices)
				{
					for (auto node_index1 : data_model.NodeNeighbours(node_index0))
					{
						const auto edge_index = data_model.EdgeFromNodePair(CGraphModel::node_pair_t(node_index0, node_index1));
						edge_indices.push_back(edge_index);
					}
				}

				// Add selected edges
				edge_indices.reserve(edge_indices.size() + selection_model.SelectedEdgeCount());
				selection_model.EdgeMask().for_each_set_bit([&](auto edge_index)
					{
						edge_indices.push_back((CGraphModel::edge_index_t)edge_index);
					});


				// Sort and remove duplicates
				std::sort(edge_indices.begin(), edge_indices.end());
				edge_indices.resize(remove_duplicates_from_ordered(edge_indices.begin(), edge_indices.end()));

				// Write edge node pairs
				for (const auto edge_index : edge_indices)
				{
					auto node_pair = data_model.EdgeNodePair(edge_index);
					SEdgeDesc edge_desc;
					edge_desc.Index = edge_index;
					edge_desc.Node0 = node_pair.first;
					edge_desc.Node1 = node_pair.second;
					del(edge_desc);
				}
			});

		WriteDeleteGraphNodesOp(ctx.m_Data, [&](auto&& del)
			{
				selection_model.NodeMask().for_each_set_bit([&](auto node_index)
					{
						const auto category = data_model.NodeCategory((CGraphModel::node_index_t)node_index);
						const auto pos = data_model.NodePosition((CGraphModel::node_index_t)node_index);
						del((CGraphModel::node_index_t)node_index, category, pos.x(), pos.y());
					});
			});

		return std::make_optional<CCmdDeleteGraphElements>();
	}
}