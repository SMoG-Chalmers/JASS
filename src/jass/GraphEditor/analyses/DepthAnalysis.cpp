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

#include <QtCore/qvariant.h>
#include <jass/analysis/BfsTraversal.h>
#include <jass/analysis/ImmutableDirectedGraph.h>
#include <jass/StandardNodeAttributes.h>
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

		QVariant root_node_index;
		if (!ctx.TryGetGraphAttribute(GRAPH_ATTTRIBUTE_ROOT_NODE, root_node_index) || root_node_index.toInt() < 0)
		{
			// No root node
			return;
		}

		auto depth_values = ctx.NewMetricVector();
		depth_values.resize(graph.NodeCount(), std::numeric_limits<float>::quiet_NaN());

		m_BfsTraversal->Traverse(ctx.ImmutableDirectedGraph(), root_node_index.toInt(),
			[&](auto node_handle, auto depth)
			{
				const auto node_index = graph.NodeIndex(node_handle);
				depth_values[node_index] = depth;
			});

		ctx.OutputMetric(QString("Depth"), std::move(depth_values));
	}
}