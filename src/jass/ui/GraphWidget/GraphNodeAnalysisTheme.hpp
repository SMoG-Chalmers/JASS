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

#include <vector>
#include <jass/GraphModel.hpp>
#include "GraphNodeTheme.hpp"
#include "SpriteSet.h"

namespace jass
{
	class CAnalyses;
	class CCategorySet;
	class CPaletteSpriteSet;

	class CGraphNodeAnalysisTheme: public CGraphNodeTheme
	{
		Q_OBJECT
	public:
		CGraphNodeAnalysisTheme(const CGraphModel& graph_model, const CAnalyses& analyses, const CCategorySet& categories, const CPaletteSpriteSet& sprites);
		~CGraphNodeAnalysisTheme();

		void SetMetric(const QString& name, bool low_is_high);

		// CGraphNodeTheme overrides
		QRect ElementLocalRect(element_t element, EStyle style) const override;
		void  DrawElement(element_t element, EStyle style, const QPoint& pos, QPainter& painter) const override;

	private Q_SLOTS:
		void OnNodesRemoved(const CGraphModel::const_node_indices_t& node_indices);
		void OnNodesInserted(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table);
		void OnMetricUpdated(const QString& name, const std::span<const float>& values);

	private:
		inline EShape NodeShape(element_t element) const;
		void UpdateColors(const std::span<const float>& metric_values);

		typedef uint8_t color_t;
		static const color_t NO_COLOR;

		const CGraphModel& m_GraphModel;
		const CAnalyses& m_Analyses;
		const CCategorySet& m_Categories;
		const CPaletteSpriteSet& m_Sprites;
		QString m_MetricName;
		bool m_LowIsHigh;
		std::vector<color_t> m_NodeColors;
	};
}
