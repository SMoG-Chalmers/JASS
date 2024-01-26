
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

#pragma once

#include <jass/analysis/BfsTraversal.h>
#include <jass/analysis/ImmutableDirectedGraph.h>
#include "DepthAnalysis.h"

namespace jass
{
	class CDepthAnalysis::CMyBfsTraversal : public CBfsTraversal<CImmutableDirectedGraph>
	{
	};

	CDepthAnalysis::CDepthAnalysis()
		: m_BfsTraversal(new CMyBfsTraversal)
	{
	}

	CDepthAnalysis::~CDepthAnalysis()
	{
	}

	void CDepthAnalysis::RunAnalysis(IAnalysisContext& ctx)
	{
		const auto& graph = ctx.ImmutableDirectedGraph();
		
		if (graph.NodeCount() == 0)
		{
			return;
		}

		const size_t root_node_index = ctx.RootNodeIndex();
		if (root_node_index == (size_t)-1)
		{
			// No root node
			return;
		}

		auto depth_values = ctx.NewMetricVector();
		depth_values.resize(graph.NodeCount(), std::numeric_limits<float>::quiet_NaN());

		m_BfsTraversal->Traverse(ctx.ImmutableDirectedGraph(), root_node_index,
			[&](auto node_handle, auto depth)
			{
				const auto node_index = graph.NodeIndex(node_handle);
				depth_values[node_index] = depth;
			});

		ctx.OutputMetric(QString("Depth"), std::move(depth_values));
	}
}