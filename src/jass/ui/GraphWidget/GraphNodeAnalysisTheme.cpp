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

#include <jass/utils/range_utils.h>
#include <jass/GraphEditor/Analyses.hpp>
#include <jass/GraphEditor/CategorySet.hpp>
#include "GraphNodeAnalysisTheme.hpp"
#include "CategorySpriteSet.hpp"
#include "PaletteSpriteSet.h"

namespace jass
{
	CGraphNodeAnalysisTheme::CGraphNodeAnalysisTheme(const CGraphModel& graph_model, const CAnalyses& analyses, const CCategorySet& categories, const CPaletteSpriteSet& sprites)
		: m_GraphModel(graph_model)
		, m_Analyses(analyses)
		, m_Categories(categories)
		, m_Sprites(sprites)
	{
		VERIFY(connect(&graph_model, &CGraphModel::NodesRemoved,  this, &CGraphNodeAnalysisTheme::OnNodesRemoved));
		VERIFY(connect(&graph_model, &CGraphModel::NodesInserted, this, &CGraphNodeAnalysisTheme::OnNodesInserted));
		VERIFY(connect(&analyses,    &CAnalyses::MetricUpdated,   this, &CGraphNodeAnalysisTheme::OnMetricUpdated));

		m_NodeColors.resize(graph_model.NodeCount(), NO_COLOR);
	}

	CGraphNodeAnalysisTheme::~CGraphNodeAnalysisTheme()
	{
	}

	void CGraphNodeAnalysisTheme::SetMetric(const QString& name, bool low_is_high)
	{
		m_MetricName = name;
		m_LowIsHigh = low_is_high;
		const auto index = m_Analyses.FindMetricIndex(name);
		UpdateColors(index >= 0 ? m_Analyses.MetricValues(index) : std::span<const float>());
	}

	inline EShape CGraphNodeAnalysisTheme::NodeShape(element_t element) const
	{
		return m_Categories.Shape(m_GraphModel.NodeCategory((CGraphModel::node_index_t)element));
	}

	QRect CGraphNodeAnalysisTheme::ElementLocalRect(element_t element, EStyle style) const
	{
		return m_Sprites.Rect(NodeShape(element), style);
	}

	void CGraphNodeAnalysisTheme::DrawElement(element_t element, EStyle style, const QPoint& pos, QPainter& painter) const
	{
		m_Sprites.Draw(NodeShape(element), style, m_NodeColors[element], pos, painter);
	}

	void CGraphNodeAnalysisTheme::OnNodesRemoved(const CGraphModel::const_node_indices_t& node_indices)
	{
		collapse(m_NodeColors, node_indices);
	}

	void CGraphNodeAnalysisTheme::OnNodesInserted(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table)
	{
		expand(m_NodeColors, node_indices, (uint8_t)0);
	}

	void CGraphNodeAnalysisTheme::OnMetricUpdated(const QString& name, const std::span<const float>& values)
	{
		if (name == m_MetricName)
		{
			UpdateColors(values);
		}
	}

	void CGraphNodeAnalysisTheme::UpdateColors(const std::span<const float>& metric_values)
	{
		if (metric_values.empty())
		{
			memset(m_NodeColors.data(), NO_COLOR, m_NodeColors.size() * sizeof(uint8_t));
			return;
		}

		if (metric_values.size() != m_NodeColors.size())
		{
			ASSERT(false && "metric and node count mismatch");
			return;
		}

		// Find metric value range
		float min_value, max_value;
		min_value = max_value = std::numeric_limits<float>::quiet_NaN();
		for (const auto value : metric_values)
		{
			if (std::isnan(value))
			{
				continue;
			}
			min_value = std::isnan(min_value) ? value : std::min(min_value, value);
			max_value = std::isnan(max_value) ? value : std::max(max_value, value);
		}

		// Assign colors based on metric value
		const float value_range = max_value - min_value;
		const float score_to_palette = (float)m_Sprites.PaletteSize() / value_range;
		for (size_t node_index = 0; node_index < m_NodeColors.size(); ++node_index)
		{
			const auto value = metric_values[node_index];
			if (std::isnan(value))
			{
				m_NodeColors[node_index] = NO_COLOR;
			}
			else
			{
				const auto color = std::min((color_t)(m_Sprites.PaletteSize() - 1), (color_t)((value - min_value) * score_to_palette));
				m_NodeColors[node_index] = m_LowIsHigh ? (color_t)m_Sprites.PaletteSize() - 1 - color : color;
			}
		}

		emit Updated();
	}
}

#include <moc_GraphNodeAnalysisTheme.cpp>