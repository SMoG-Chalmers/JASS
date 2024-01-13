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

#include "EdgeGraphLayer.hpp"

namespace jass
{
	// Google Maps Blue
	static const QRgb GOOGLE_MAPS_BLUE = qRgb(0x0a, 0x84, 0xff);
	static const QRgb COLOR_NORMAL = qRgb(0, 0, 0);
	static const QRgb COLOR_SELECTED = qRgb(0x0a, 0x84, 0xff);
	static const QRgb COLOR_HILIGHT = Blend(COLOR_SELECTED, 0xFFFFFFFF, 48);
	//static const QRgb COLOR_HILIGHT = qRgb(0xe7, 0x85, 0x1d);  // Orange
	

	// TODO: Improve performance
	template <class TFunc>
	void CEdgeGraphLayer::ForEachPotentialEdgeInRange(const QRectF& range, TFunc&& func) const
	{
		for (auto& edge : m_Edges)
		{
			func(edge);
		}
	}

	CEdgeGraphLayer::CEdgeGraphLayer(CGraphWidget& graphWidget, CGraphModel& graph_model, CGraphSelectionModel& selection_model)
		: CGraphLayer(graphWidget)
		, m_GraphModel(graph_model)
		, m_SelectionModel(selection_model)
	{
		connect(&graph_model, &CGraphModel::EdgesInserted, this, &CEdgeGraphLayer::OnEdgesInserted);
		connect(&graph_model, &CGraphModel::EdgesRemoved, this, &CEdgeGraphLayer::OnEdgesRemoved);
		connect(&selection_model, &CGraphSelectionModel::SelectionChanged, this, &CEdgeGraphLayer::OnSelectionChanged);
		connect(&graph_model, &CGraphModel::NodesModified, this, &CEdgeGraphLayer::OnNodesModified);
		RebuildEdges();
	}

	void CEdgeGraphLayer::SetTempLine(const QLineF& line)
	{
		if (m_TempLine == line)
		{
			return;
		}

		QRect rcUpdate = LineRect(m_TempLine, m_TempLineWidth);

		m_TempLine = line;

		if (!line.isNull())
		{
			rcUpdate = rcUpdate.isEmpty() ? LineRect(line, m_TempLineWidth) : rcUpdate.united(LineRect(line, m_TempLineWidth));
		}

		if (!rcUpdate.isEmpty())
		{
			Update(rcUpdate);
		}
	}

	void CEdgeGraphLayer::HideTempLine()
	{
		Update(LineRect(m_TempLine, m_TempLineWidth));
		m_TempLine = QLineF();
	}

	bool CEdgeGraphLayer::RangedHitTest(const QRect& rc, bitvec& out_hit_elements) const
	{
		bool any_hit = false;
		out_hit_elements.clear();
		out_hit_elements.resize(m_Edges.size());

		const int inflate_amount = std::ceil(.5f * m_LineWidth);
		const auto rcModelTest = GraphWidget().ModelFromScreen(QRectF(rc).adjusted(-inflate_amount, -inflate_amount, inflate_amount, inflate_amount));

		ForEachPotentialEdgeInRange(
			rcModelTest,
			[&](auto& edge)
			{
				if (Intersects(rcModelTest, edge.Line))
				{
					out_hit_elements.set(&edge - m_Edges.data());
					any_hit = true;
				}
			});

		return any_hit;
	}

	void CEdgeGraphLayer::SetHilighted(element_t edge, bool hilighted)
	{
		if (m_HilightMask.get(edge) == hilighted)
		{
			return;
		}
		Update(EdgeScreenRect(m_Edges[edge]));
		m_HilightMask.set(edge, hilighted);
		Update(EdgeScreenRect(m_Edges[edge]));
	}

	void CEdgeGraphLayer::Paint(QPainter& painter, const QRect& rcClip)
	{
		QPen penNormal(COLOR_NORMAL);
		penNormal.setWidth(m_LineWidth);

		QPen penSelected(COLOR_SELECTED);
		penSelected.setWidth(m_LineWidth);

		QPen penHilight(COLOR_HILIGHT);
		penHilight.setWidth(m_TempLineWidth);

		const int inflate_amount = std::ceil(.5f * m_LineWidth);
		const auto rcModelTest = GraphWidget().ModelFromScreen(QRectF(rcClip).adjusted(-inflate_amount, -inflate_amount, inflate_amount, inflate_amount));

		painter.setRenderHint(QPainter::Antialiasing);

		ForEachPotentialEdgeInRange(
			rcModelTest,
			[&](auto& edge)
			{
				if (rcModelTest.intersects(EdgeModelRect(edge)))
				{
					if (IsHilighted(&edge - m_Edges.data()))
						painter.setPen(penHilight);
					else if (IsSelected(&edge - m_Edges.data()))
						painter.setPen(penSelected);
					else
						painter.setPen(penNormal);
					painter.drawLine(GraphWidget().ScreenFromModel(edge.Line.p1()), GraphWidget().ScreenFromModel(edge.Line.p2()));
				}
			});

		if (!m_TempLine.isNull())
		{
			QPen pen(COLOR_HILIGHT);
			pen.setCapStyle(Qt::RoundCap);
			pen.setWidth(m_TempLineWidth);
			painter.setPen(pen);
			painter.drawLine(m_TempLine);
		}
	}

	CEdgeGraphLayer::element_t CEdgeGraphLayer::HitTest(const QPoint& pt)
	{
		const float MAX_DIST_SCREEN = 5.0f;
		const float MAX_DIST_MODEL = MAX_DIST_SCREEN * GraphWidget().ScreenToModelScale();

		const auto pt_model = GraphWidget().ModelFromScreen(pt);

		const SEdge* closestEdge = nullptr;
		float min_dist_sqr = std::numeric_limits<float>::infinity();
		ForEachPotentialEdgeInRange(
			QRectF(pt_model.x() - MAX_DIST_MODEL, pt_model.y() - MAX_DIST_MODEL, MAX_DIST_MODEL * 2, MAX_DIST_MODEL * 2),
			[&](auto& edge)
			{
				const auto dist_sqr = SquaredDistanceFromPointToLineSegment(pt_model, edge.Line);
				if (dist_sqr < min_dist_sqr)
				{
					min_dist_sqr = dist_sqr;
					closestEdge = &edge;
				}
			});
		
		if (min_dist_sqr <= MAX_DIST_MODEL * MAX_DIST_MODEL)
		{
			return (element_t)(closestEdge - m_Edges.data());
		}

		return NO_ELEMENT;
	}

	void CEdgeGraphLayer::GetSelection(bitvec& out_selection_mask) const
	{
		out_selection_mask = m_SelectionModel.EdgeMask();
	}

	void CEdgeGraphLayer::SetSelection(const bitvec& selection_mask) const
	{
		m_SelectionModel.BeginModify();
		if (selection_mask.empty())
		{
			m_SelectionModel.DeselectAllEdges();
		}
		else
		{
			m_SelectionModel.SetEdgeMask(selection_mask);
		}
		m_SelectionModel.EndModify();
	}

	void CEdgeGraphLayer::OnViewChanged(const QRect& rc, float screen_to_model_scale)
	{
		RebuildEdges();
	}

	void CEdgeGraphLayer::OnSelectionChanged()
	{
		m_TempSelectionMask.bitwise_xor(m_SelectionMask, m_SelectionModel.EdgeMask());

		m_TempSelectionMask.for_each_set_bit([&](size_t edge_index)
			{
				auto& edge = m_Edges[edge_index];
				Update(EdgeScreenRect(edge));
				m_SelectionMask.toggle(edge_index);
				Update(EdgeScreenRect(edge));
			});
	}

	void CEdgeGraphLayer::OnEdgesInserted(const CGraphModel::const_edge_indices_t& edge_indices, const CGraphModel::node_remap_table_t& remap_table)
	{
		RebuildEdges();
		Update();
	}

	void CEdgeGraphLayer::OnEdgesRemoved(const CGraphModel::const_edge_indices_t& edge_indices)
	{
		RebuildEdges();
		Update();
	}

	void CEdgeGraphLayer::OnNodesModified(const bitvec& nodes_mask)
	{
		QRect rc, rcUpdate;
		nodes_mask.for_each_set_bit([&](const size_t node_index)
			{
				const auto node_pos = m_GraphModel.NodePositionF((CGraphModel::node_index_t)node_index);
				m_GraphModel.ForEachEdgeFromNode((CGraphModel::node_index_t)node_index, [&](auto edge_index, auto neighbour_index)
				{
					if (MoveEdge(edge_index, node_pos, m_GraphModel.NodePositionF(neighbour_index), rc))
					{
						rcUpdate = rcUpdate.united(rc);
					}
				});
			});
		if (!rcUpdate.isNull())
		{
			Update(rcUpdate);
		}
	}

	static QLineF CutEnds(const QLineF& line, float cut_length)
	{
		QPointF v(line.p2() - line.p1());
		const float length = sqrtf(v.x() * v.x() + v.y() * v.y());
		QPointF t(v * (1.0f / length));
		const auto cut_v = t * cut_length;
		return QLineF(line.p1() + cut_v, line.p2() - cut_v);
	}

	void CEdgeGraphLayer::RebuildEdges()
	{
		const float CUT_END_LENGTH = 8 * GraphWidget().ScreenToModelScale();

		m_Edges.clear();
		m_Edges.resize(m_GraphModel.EdgeCount());
		for (CGraphModel::edge_index_t edge_index = 0; edge_index < m_GraphModel.EdgeCount(); ++edge_index)
		{
			const auto& node_pair = m_GraphModel.EdgeNodePair(edge_index);
			const auto p0 = QPointF(m_GraphModel.NodePosition(node_pair.first));
			const auto p1 = QPointF(m_GraphModel.NodePosition(node_pair.second));
			m_Edges[edge_index].Line = CutEnds(QLineF(p0, p1), CUT_END_LENGTH);
		}

		m_SelectionMask.resize(m_Edges.size());
		m_HilightMask.resize(m_Edges.size());
		m_HilightMask.clearAll();
	}

	bool CEdgeGraphLayer::MoveEdge(size_t edge_index, const QPointF& p0, const QPointF& p1, QRect& out_update_rect)
	{
		auto& edge = m_Edges[edge_index];
		if (edge.Line.p1() == p0 && edge.Line.p2() == p1)
		{
			return false;
		}
		out_update_rect = EdgeScreenRect(edge);
		edge.Line.setP1(p0);
		edge.Line.setP2(p1);
		out_update_rect = out_update_rect.united(EdgeScreenRect(edge));
		return true;
	}

	QRectF CEdgeGraphLayer::EdgeModelRect(const SEdge& edge) const
	{
		const auto& line = edge.Line;
		const float line_width_model = m_LineWidth * GraphWidget().ScreenToModelScale();
		return QRectF(
			std::min(line.p1().x(), line.p2().x()) - line_width_model * .5f,
			std::min(line.p1().y(), line.p2().y()) - line_width_model * .5f,
			std::abs(line.p2().x() - line.p1().x()) + line_width_model,
			std::abs(line.p2().y() - line.p1().y()) + line_width_model);
	}

	QRect CEdgeGraphLayer::EdgeScreenRect(const SEdge& edge) const
	{
		const auto& line = edge.Line;
		const auto pt_min_screen = GraphWidget().ScreenFromModel(QPointF(
			std::min(line.p1().x(), line.p2().x()),
			std::min(line.p1().y(), line.p2().y())));
		const auto pt_max_screen = GraphWidget().ScreenFromModel(QPointF(
			std::max(line.p1().x(), line.p2().x()),
			std::max(line.p1().y(), line.p2().y())));
		const QPoint pt_min_pixel(
			(int)std::floor(pt_min_screen.x() - m_LineWidth * .5f),
			(int)std::floor(pt_min_screen.y() - m_LineWidth * .5f));
		return QRect(
			pt_min_pixel,
			QSize(
				(int)std::ceil(pt_max_screen.x() + m_LineWidth * .5f) - pt_min_pixel.x(),
				(int)std::ceil(pt_max_screen.y() + m_LineWidth * .5f) - pt_min_pixel.y())
			);
	}

	QRect CEdgeGraphLayer::LineRect(const QLineF& line, float line_width)
	{
		if (line.isNull())
		{
			return QRect();
		}
		const float half_line_width = line_width * .5f;
		const QPointF pt_min(
			std::min(line.p1().x(), line.p2().x()),
			std::min(line.p1().y(), line.p2().y()));
		const QPointF pt_max(
			std::max(line.p1().x(), line.p2().x()),
			std::max(line.p1().y(), line.p2().y()));
		const QPoint pt_min_pixel(
			(int)std::floor(pt_min.x() - half_line_width),
			(int)std::floor(pt_min.y() - half_line_width));
		return QRect(
			pt_min_pixel,
			QSize(
				(int)std::ceil(pt_max.x() + half_line_width) - pt_min_pixel.x(),
				(int)std::ceil(pt_max.y() + half_line_width) - pt_min_pixel.y())
		);
	}
}

#include <moc_EdgeGraphLAyer.cpp>