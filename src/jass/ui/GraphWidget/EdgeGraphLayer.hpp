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
#include "GraphWidget.hpp"

namespace jass
{
	class CEdgeGraphLayer: public QObject, public CGraphLayer
	{
		Q_OBJECT
	public:
		CEdgeGraphLayer(CGraphWidget& graphWidget, CGraphModel& graph_model, CGraphSelectionModel& selection_model);

		void SetTempLine(const QLineF& line);
		void HideTempLine();

		inline bool IsHilighted(element_t edge) const;

		// CGraphLayer overrides
		void Paint(QPainter& painter, const QRect& rc) override;
		element_t HitTest(const QPoint& pt) override;
		bool RangedHitTest(const QRect& rc, bitvec& out_hit_elements) const override;
		void SetHilighted(element_t edge, bool hilighted) override;
		void GetSelection(bitvec& out_selection_mask) const override;
		void SetSelection(const bitvec& selection_mask) const override;
		void OnViewChanged(const QRect& rc, float screen_to_model_scale) override;

	private Q_SLOTS:
		void OnSelectionChanged();
		void OnEdgesInserted(const CGraphModel::const_edge_indices_t& edge_indices, const CGraphModel::node_remap_table_t& remap_table);
		void OnEdgesRemoved(const CGraphModel::const_edge_indices_t& edge_indices);
		void OnNodesModified(const bitvec& nodes_mask);

	private:
		struct SEdge
		{
			QLineF Line;
		};

		inline bool IsSelected(element_t edge) const;

		void RebuildEdges();

		bool MoveEdge(size_t edge_index, const QPointF& p0, const QPointF& p1, QRect& out_update_rect);

		template <class TFunc>
		void ForEachPotentialEdgeInRange(const QRectF& range, TFunc&& func) const;

		QRectF EdgeModelRect(const SEdge& edge) const;
		QRect EdgeScreenRect(const SEdge& edge) const;
		static QRect LineRect(const QLineF& line, float line_width);

		CGraphModel& m_GraphModel;
		CGraphSelectionModel& m_SelectionModel;

		float m_LineWidth = 2;
		float m_TempLineWidth = 4;
		std::vector<SEdge> m_Edges;
		QLineF m_TempLine;
		bitvec m_SelectionMask;
		bitvec m_TempSelectionMask;
		bitvec m_HilightMask;
	};

	inline bool CEdgeGraphLayer::IsHilighted(element_t edge) const
	{
		return m_HilightMask.get(edge);
	}

	inline bool CEdgeGraphLayer::IsSelected(element_t edge) const
	{
		return m_SelectionMask.get(edge);
	}
}