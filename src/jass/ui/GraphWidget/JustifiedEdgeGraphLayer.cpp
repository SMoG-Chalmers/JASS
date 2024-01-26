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

#include <QtCore/qline.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpen.h>

#include <jass/math/Geometry.h>
#include <jass/ui/ImageFx.h>
#include <jass/Debug.h>
#include <jass/GraphModel.hpp>

#include "JustifiedEdgeGraphLayer.hpp"

namespace jass
{
	// Google Maps Blue
	static const QRgb COLOR_NORMAL = qRgb(0, 0, 0);

	CJustifiedEdgeGraphLayer::CJustifiedEdgeGraphLayer(CGraphWidget& graphWidget, CGraphModel& graph_model, CGraphSelectionModel& selection_model)
		: CGraphLayer(graphWidget)
		, m_GraphModel(graph_model)
		, m_SelectionModel(selection_model)
	{
		m_JPositionNodeAttribute = TryGetJPositionNodeAttribute(m_GraphModel);

		connect(&graph_model, &CGraphModel::EdgesAdded, this, &CJustifiedEdgeGraphLayer::OnEdgesAdded);
		connect(&graph_model, &CGraphModel::EdgesInserted, this, &CJustifiedEdgeGraphLayer::OnEdgesInserted);
		connect(&graph_model, &CGraphModel::EdgesRemoved, this, &CJustifiedEdgeGraphLayer::OnEdgesRemoved);
		connect(&graph_model, &CGraphModel::NodesModified, this, &CJustifiedEdgeGraphLayer::OnNodesModified);
		
		Reset();
	}

	void CJustifiedEdgeGraphLayer::Paint(QPainter& painter, const QRect& rcClip)
	{
		QPen penNormal(COLOR_NORMAL);
		penNormal.setWidth(m_LineWidth);
		painter.setPen(penNormal);

		painter.setRenderHint(QPainter::Antialiasing);

		const int inflateAmount = m_LineWidth + 2;
		const auto rcIntersect = QRectF(rcClip.adjusted(-inflateAmount, -inflateAmount, +inflateAmount, +inflateAmount));

		QPoint p0, p1;
		for (size_t edge_index = 0; edge_index < m_GraphModel.EdgeCount(); ++edge_index)
		{
			if (!TryGetEdgePoints(edge_index, p0, p1))
			{
				continue;
			}
			const QLineF line(p0, p1);
			if (!Intersects(rcIntersect, line))
			{
				continue;
			}
			painter.drawLine(line);
			m_Edges[edge_index].LastRect = EdgeRect(p0, p1);
		}
	}

	void CJustifiedEdgeGraphLayer::OnViewChanged(const QRect& rc, float screen_to_model_scale)
	{
		Reset();
	}

	void CJustifiedEdgeGraphLayer::OnEdgesAdded(size_t count)
	{
		Reset();
	}

	void CJustifiedEdgeGraphLayer::OnEdgesInserted(const CGraphModel::const_edge_indices_t& edge_indices, const CGraphModel::node_remap_table_t& remap_table)
	{
		Reset();
	}

	void CJustifiedEdgeGraphLayer::OnEdgesRemoved(const CGraphModel::const_edge_indices_t& edge_indices)
	{
		Reset();
	}

	void CJustifiedEdgeGraphLayer::OnNodesModified(const bitvec& nodes_mask)
	{
		QRect rcUpdate;
		nodes_mask.for_each_set_bit([&](const size_t node_index)
			{
				const auto node_pos = m_GraphModel.NodePosition((CGraphModel::node_index_t)node_index);
				m_GraphModel.ForEachEdgeFromNode((CGraphModel::node_index_t)node_index, [&](auto edge_index, auto neighbour_index)
				{
					rcUpdate = rcUpdate.united(m_Edges[edge_index].LastRect);
					rcUpdate = rcUpdate.united(EdgeRect(edge_index));
				});
			});
		if (!rcUpdate.isEmpty())
		{
			Update(rcUpdate);
		}
	}

	bool CJustifiedEdgeGraphLayer::IsNodeJustified(size_t node_index) const
	{
		ASSERT(m_JPositionNodeAttribute);
		return m_JPositionNodeAttribute && m_JPositionNodeAttribute->Value(node_index).y() >= 0;
	}

	QPoint CJustifiedEdgeGraphLayer::NodePos(size_t node_index) const
	{
		ASSERT(m_JPositionNodeAttribute);
		return QPointFromRoundedQPointF(GraphWidget().ScreenFromModel(m_JPositionNodeAttribute->Value(node_index)));
	}

	QRect CJustifiedEdgeGraphLayer::EdgeRect(size_t edge_index) const
	{
		QPoint p0, p1;
		return TryGetEdgePoints(edge_index, p0, p1) ?
			EdgeRect(p0, p1) :
			QRect();
	}

	QRect CJustifiedEdgeGraphLayer::EdgeRect(const QPoint& p0, const QPoint& p1) const
	{
		return QRect(
			std::min(p0.x(), p1.x()) - m_LineWidth,
			std::min(p0.y(), p1.y()) - m_LineWidth,
			std::abs(p1.x() - p0.x()) + m_LineWidth * 2 + 1,
			std::abs(p1.y() - p0.y()) + m_LineWidth * 2 + 1);
	}

	bool CJustifiedEdgeGraphLayer::TryGetEdgePoints(size_t edge_index, QPoint& out_p0, QPoint& out_p1) const
	{
		const auto node_pair = m_GraphModel.EdgeNodePair((CGraphModel::edge_index_t)edge_index);
		if (!IsNodeJustified(node_pair.first) || !IsNodeJustified(node_pair.second))
		{
			return false;
		}
		out_p0 = NodePos(node_pair.first);
		out_p1 = NodePos(node_pair.second);
		return true;
	}

	void CJustifiedEdgeGraphLayer::Reset()
	{
		m_Edges.clear();
		m_Edges.resize(m_GraphModel.EdgeCount(), {});
		Update();
	}
}

#include <moc_JustifiedEdgeGraphLayer.cpp>