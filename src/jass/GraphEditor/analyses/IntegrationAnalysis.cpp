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

#include <QtCore/qstring.h>
#include <jass/analysis/DepthCalculator.h>
#include <jass/analysis/ImmutableDirectedGraph.h>
#include <jass/analysis/Integration.h>
#include "IntegrationAnalysis.h"

namespace jass
{
	class CIntegrationAnalysis::CMyDepthCalculator : public CDepthCalculator<CImmutableDirectedGraph>
	{
	};

	CIntegrationAnalysis::CIntegrationAnalysis()
		: m_DepthCalculator(new CMyDepthCalculator)
	{
	}

	CIntegrationAnalysis::~CIntegrationAnalysis()
	{
	}

	void CIntegrationAnalysis::RunAnalysis(IAnalysisContext& ctx)
	{
		const auto& graph = ctx.ImmutableDirectedGraph();
		auto INT_values = ctx.NewMetricVector();
		auto TD_values = ctx.NewMetricVector();
		auto MD_values = ctx.NewMetricVector();
		auto RA_values = ctx.NewMetricVector();
		auto RRA_values = ctx.NewMetricVector();
		for (size_t node_index = 0; node_index < graph.NodeCount(); ++node_index)
		{
			size_t max_depth, total_depth, node_count;
			m_DepthCalculator->CalculateDepth(graph, node_index, max_depth, total_depth, node_count);
			float MD, RA, RRA;
			const auto integration_value = CalculateIntegrationScore((unsigned int)node_count, (float)total_depth, MD, RA, RRA);
			INT_values.push_back(integration_value);
			TD_values.push_back(total_depth);
			MD_values.push_back(MD);
			RA_values.push_back(RA);
			RRA_values.push_back(RRA);
		}
		ctx.OutputMetric(QString("RRA"), std::move(RRA_values));
		ctx.OutputMetric(QString("RA"), std::move(RA_values));
		ctx.OutputMetric(QString("MD"), std::move(MD_values));
		ctx.OutputMetric(QString("TD"), std::move(TD_values));
		ctx.OutputMetric(QString("Integration"), std::move(INT_values));
	}
}