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
#include "CmdDuplicate.h"

namespace jass
{
	CCmdDuplicate::CCmdDuplicate(qapp::SCommandCreationContext& ctx, CGraphModel& data_model, const bitvec& node_mask)
	{
		const QPointF duplicate_offset(10, 10);

		std::unordered_map<CGraphModel::node_index_t, CGraphModel::node_index_t> index_map;
		index_map.reserve(node_mask.count_set_bits());

		WriteInsertGraphNodesOp(ctx.m_Data, [&](auto&& add)
			{
				auto new_node_index = (CGraphModel::node_index_t)data_model.NodeCount();
				node_mask.for_each_set_bit([&](auto index)
					{
						const auto node_index = (CGraphModel::node_index_t)index;
						const auto pos = duplicate_offset + data_model.NodePosition(node_index);
						add(new_node_index, data_model.NodeCategory(node_index), pos.x() , pos.y());
						index_map.insert(std::make_pair(node_index, new_node_index));
						++new_node_index;
					});
			});

		std::vector<CGraphModel::node_pair_t> new_edges;
		node_mask.for_each_set_bit([&](auto index)
			{
				const auto node_index = (CGraphModel::node_index_t)index;
				for (auto neighbour_index : data_model.NodeNeighbours(node_index))
				{
					if (neighbour_index <= node_index)
					{
						continue;
					}
					auto it = index_map.find(neighbour_index);
					if (index_map.end() == it)
					{
						continue;
					}
					new_edges.push_back({ index_map[node_index], index_map[neighbour_index] });
				}
			});

		if (!new_edges.empty())
		{
			WriteInsertGraphEdgesOp(ctx.m_Data, [&](auto&& add)
				{
					for (CGraphModel::edge_index_t edge_index = 0; edge_index < new_edges.size(); ++edge_index)
					{
						const auto& node_pair = new_edges[edge_index];
						SEdgeDesc edge_desc;
						edge_desc.Index = edge_index + data_model.EdgeCount();
						edge_desc.Node0 = std::min(node_pair.first, node_pair.second);
						edge_desc.Node1 = std::max(node_pair.first, node_pair.second);
						add(edge_desc);
					}
				});
		}
	}
}