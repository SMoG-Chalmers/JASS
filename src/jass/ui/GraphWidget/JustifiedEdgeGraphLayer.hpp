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

#include <vector>
#include <jass/GraphModel.hpp>
#include <jass/StandardNodeAttributes.h>
#include "GraphWidget.hpp"

namespace jass
{
	class CJustifiedEdgeGraphLayer: public QObject, public CGraphLayer
	{
		Q_OBJECT
	public:
		CJustifiedEdgeGraphLayer(CGraphWidget& graphWidget, CGraphModel& graph_model, CGraphSelectionModel& selection_model);

		// CGraphLayer overrides
		void Paint(QPainter& painter, const QRect& rc) override;
		void OnViewChanged(const QRect& rc, float screen_to_model_scale) override;

	private Q_SLOTS:
		void OnEdgesAdded(size_t count);
		void OnEdgesInserted(const CGraphModel::const_edge_indices_t& edge_indices, const CGraphModel::node_remap_table_t& remap_table);
		void OnEdgesRemoved(const CGraphModel::const_edge_indices_t& edge_indices);
		void OnNodesModified(const bitvec& nodes_mask);

	private:
		bool   IsNodeJustified(size_t node_index) const;
		QPoint NodePos(size_t node_index) const;
		QRect  EdgeRect(size_t edge_index) const;
		QRect  EdgeRect(const QPoint& p0, const QPoint& p1) const;
		bool   TryGetEdgePoints(size_t edge_index, QPoint& out_p0, QPoint& out_p1) const;

		struct SEdge
		{
			QRect LastRect;
		};

		void Reset();

		CGraphModel& m_GraphModel;
		CGraphSelectionModel& m_SelectionModel;
		JPosition_NodeAttribute_t* m_JPositionNodeAttribute = nullptr;

		int m_LineWidth = 2;
		std::vector<SEdge> m_Edges;
	};
}