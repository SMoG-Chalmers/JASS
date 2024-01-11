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
#include <jass/graphdata/GraphView.h>
#include <jass/utils/range_utils.h>

#include "GraphOperations.h"
#include "CmdAddGraphElements.h"

namespace jass
{
	CCmdAddGraphElements::CCmdAddGraphElements(
		qapp::SCommandCreationContext& ctx, 
		CGraphModel& data_model,
		const IGraphView& gview)
	{
		const auto node_count = gview.NodeCount();
		std::vector<QPointF> node_positions(node_count);
		std::vector<uint32_t> node_categories(node_count);
		gview.TryGetNodeAttributes(GRAPH_NODE_ATTTRIBUTE_POSITION, to_span(node_positions));
		gview.TryGetNodeAttributes(GRAPH_NODE_ATTTRIBUTE_CATEGORY, to_span(node_categories));

		WriteInsertGraphNodesOp(ctx.m_Data, [&](auto&& add)
			{
				for (CGraphModel::node_index_t node_index = 0; node_index < node_count; ++node_index)
				{
					add(
						data_model.NodeCount() + node_index,
						node_categories.empty() ? CGraphModel::NO_CATEGORY : node_categories[node_index],
						node_positions.empty() ? 0 : node_positions[node_index].x(),
						node_positions.empty() ? 0 : node_positions[node_index].y());
				}
			});

		WriteInsertGraphEdgesOp(ctx.m_Data, [&](auto&& add)
			{
				auto edge_index = data_model.EdgeCount();
				std::vector<IGraphView::edge_t> edges(gview.EdgeCount());
				gview.GetEdges(edges);
				for (const auto& edge : edges)
				{
					SEdgeDesc desc;
					desc.Index = edge_index++;
					desc.Node0 = edge.first + data_model.NodeCount();
					desc.Node1 = edge.second + data_model.NodeCount();
					add(desc);
				}
			});
	}
}