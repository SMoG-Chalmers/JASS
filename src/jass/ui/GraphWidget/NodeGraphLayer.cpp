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

#include <array>
#include <numbers>

#include <QtGui/qpainter.h>
#include <QtGui/qimage.h>
#include <QtGui/qpixmap.h>

#include <jass/math/Geometry.h>
#include <jass/ui/ImageFx.h>
#include <jass/GraphEditor/CategorySet.hpp>
#include <jass/Shape.h>
#include "NodeGraphLayer.hpp"

namespace jass
{
	static const QRgb COLOR_SELECTED = qRgb(0x0a, 0x84, 0xff);
	static const QRgb COLOR_HILIGHT = Blend(COLOR_SELECTED, 0xFFFFFFFF, 48);
	//static const QRgb COLOR_HILIGHT = qRgb(0xe7, 0x85, 0x1d);  // Orange

	void CNodeGraphLayer::SSprite::Draw(QPainter& painter, const QPoint& at) const
	{
		painter.drawPixmap(at - this->Origin, this->Pixmap);
	}

	CNodeGraphLayer::CNodeGraphLayer(CGraphWidget& graphWidget, CGraphModel& graph_model, CCategorySet& categories, CGraphSelectionModel& selection_model)
		: CGraphLayer(graphWidget)
		, m_GraphModel(graph_model)
		, m_Categories(categories)
		, m_SelectionModel(selection_model)
	{
		// Google Maps Blue
		// QColor("#0a84ff")

		const QColor palette[] = {
			QColor("#cd001a"),
			QColor("#ef6a00"),
			QColor("#f2cd00"),
			QColor("#79c300"),
			QColor("#1961ae"),
			QColor("#61007d"),
			QColor("#A09080"),
		};

		//const QColor palette[] = {
		//	QColor("#f94144"),
		//	QColor("#f3722c"),
		//	QColor("#f8961e"),
		//	QColor("#f9c74f"),
		//	QColor("#90be6d"),
		//	QColor("#43aa8b"),
		//	QColor("#577590"),
		//};

		SShapeSpriteDesc ssdNormal;
		ssdNormal.Radius = 9.5f;
		ssdNormal.OutlineWidth = 3.0f;
		ssdNormal.ShadowOffset = { 1, 1 };
		ssdNormal.ShadowBlurRadius = 3.0f;
		ssdNormal.OutlineColor = Qt::white;
		ssdNormal.ShadowColor = qRgba(0, 0, 0, 255);

		SShapeSpriteDesc ssdSelected = ssdNormal;
		ssdSelected.OutlineWidth2 = ssdNormal.OutlineWidth;
		ssdSelected.OutlineColor2 = QColor::fromRgba(COLOR_SELECTED);

		SShapeSpriteDesc ssdHilighted = ssdSelected;
		ssdHilighted.OutlineColor2 = QColor::fromRgba(COLOR_HILIGHT);
		ssdHilighted.OutlineWidth2 = 4;

		for (size_t category_index = 0; category_index < m_Categories.Size(); ++category_index)
		{
			ssdNormal.Shape = m_Categories.Shape(category_index);
			ssdNormal.FillColor = QColor::fromRgba(m_Categories.Color(category_index));
			m_Sprites.push_back(CreateSprite(ssdNormal));

			ssdSelected.Shape = ssdNormal.Shape;
			ssdSelected.FillColor = ssdNormal.FillColor;
			m_Sprites.push_back(CreateSprite(ssdSelected));

			ssdHilighted.Shape = ssdNormal.Shape;
			ssdHilighted.FillColor = Blend(m_Categories.Color(category_index), 0xFFFFFFFF, 64);
			m_Sprites.push_back(CreateSprite(ssdHilighted));
		}

		connect(&selection_model, &CGraphSelectionModel::SelectionChanged, this, &CNodeGraphLayer::OnSelectionChanged);
		connect(&graph_model, &CGraphModel::NodesRemoved, this, &CNodeGraphLayer::OnNodesRemoved);
		connect(&graph_model, &CGraphModel::NodesInserted, this, &CNodeGraphLayer::OnNodesInserted);
		connect(&graph_model, &CGraphModel::NodesModified, this, &CNodeGraphLayer::OnNodesModified);

		RebuildNodes();
	}

	QPoint CNodeGraphLayer::NodeScreenPos(element_t node) const
	{
		return m_Nodes[node].Position + GraphWidget().ScreenTranslation();
	}

	void CNodeGraphLayer::Paint(QPainter& painter, const QRect& rcClip)
	{
		for (size_t node_index = 0; node_index < m_Nodes.size(); ++node_index)
		{
			const auto& node = m_Nodes[node_index];
			const auto& sprite = NodeSprite(node);
			const auto nodeRect = NodeRect(node);
			const auto rcVis = nodeRect.intersected(rcClip);
			if (rcVis.isEmpty())
			{
				continue;
			}
			const QRect srcRect(rcVis.left() - nodeRect.left(), rcVis.top() - nodeRect.top(), rcVis.width(), rcVis.height());
			painter.drawPixmap(rcVis.topLeft(), sprite.Pixmap, srcRect);
		}
	}

	CNodeGraphLayer::element_t CNodeGraphLayer::HitTest(const QPoint& pt)
	{
		for (size_t node_index = m_Nodes.size() - 1; node_index < m_Nodes.size(); --node_index)
		{
			const auto& node = m_Nodes[node_index];
			const auto rect = NodeRect(node);
			if (rect.contains(pt))
			{
				return node_index;
			}
		}
		return NO_ELEMENT;
	}

	bool CNodeGraphLayer::RangedHitTest(const QRect& rc, bitvec& out_hit_elements) const
	{
		bool any_hit = false;
		out_hit_elements.clear();
		out_hit_elements.resize(m_Nodes.size());
		const auto hitRect = rc.adjusted(-m_HitRadius, -m_HitRadius, m_HitRadius, m_HitRadius).translated(-GraphWidget().ScreenTranslation());
		for (size_t node_index = 0; node_index < m_Nodes.size(); ++node_index)
		{
			const auto& node = m_Nodes[node_index];
			if (hitRect.contains(node.Position))
			{
				out_hit_elements.set(node_index);
				any_hit = true;
			}
		}
		return any_hit;
	}

	void CNodeGraphLayer::SetHilighted(element_t node, bool hilighted)
	{
		if (m_HilightMask.get(node) == hilighted)
		{
			return;
		}
		auto rcBefore = NodeRect(m_Nodes[node]);
		if (hilighted)
			m_HilightMask.set(node);
		else
			m_HilightMask.clear(node);
		auto rcAfter = NodeRect(m_Nodes[node]);
		Update(rcBefore.united(rcAfter));
	}

	void CNodeGraphLayer::GetSelection(bitvec& out_selection_mask) const
	{
		out_selection_mask = m_SelectionModel.NodeMask();
	}

	void CNodeGraphLayer::SetSelection(const bitvec& selection_mask) const
	{
		m_SelectionModel.BeginModify();
		if (selection_mask.empty())
		{
			m_SelectionModel.DeselectAllNodes();
		}
		else
		{
			m_SelectionModel.SetNodeMask(selection_mask);
		}
		m_SelectionModel.EndModify();
	}

	void CNodeGraphLayer::OnViewChanged(const QRect& rc, float screen_to_model_scale)
	{
		RebuildNodes();
	}

	void CNodeGraphLayer::OnSelectionChanged()
	{
		m_TempSelectionMask.bitwise_xor(m_SelectionMask, m_SelectionModel.NodeMask());

		m_TempSelectionMask.for_each_set_bit([&](size_t node_index)
			{
				auto& node = m_Nodes[node_index];
				Update(NodeRect(node));
				m_SelectionMask.toggle(node_index);
				Update(NodeRect(node));
			});
	}

	void CNodeGraphLayer::OnNodesRemoved(const CGraphModel::const_node_indices_t& node_indices)
	{
		RebuildNodes();
		Update();
	}

	void CNodeGraphLayer::OnNodesInserted(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table)
	{
		RebuildNodes();
		Update();
	}

	void CNodeGraphLayer::OnNodesModified(const bitvec& node_mask)
	{
		const auto model_to_screen_scale = GraphWidget().ModelToScreenScale();

		node_mask.for_each_set_bit([&](const size_t node_index)
			{
				const auto category = m_GraphModel.NodeCategory((CGraphModel::node_index_t)node_index);
				const auto pos = QPointFromRoundedQPointF(m_GraphModel.NodePosition((CGraphModel::node_index_t)node_index) * model_to_screen_scale);
				auto& node = m_Nodes[node_index];
				if (category == node.Category && pos == node.Position)
				{
					return;
				}
				auto oldRect = NodeRect(node);
				node.Category = category;
				node.Position = pos;
				Update(NodeRect(node).united(oldRect));
			});
	}

	const CNodeGraphLayer::SSprite& CNodeGraphLayer::NodeSprite(const SNode& node) const
	{
		ENodeSpriteStyle style;
		if (IsNodeHilighted(&node - m_Nodes.data()))
			style = ENodeSpriteStyle::Hilighted;
		else if (IsNodeSelected(node))
			style = ENodeSpriteStyle::Selected;
		else
			style = ENodeSpriteStyle::Normal;
		return NodeSprite(node.Category, style);
	}

	bool CNodeGraphLayer::IsNodeSelected(const SNode& node) const
	{
		return m_SelectionMask.get(&node - m_Nodes.data());
	}

	QRect CNodeGraphLayer::NodeRect(const SNode& node) const
	{
		const auto translation = GraphWidget().ScreenTranslation();
		const auto& sprite = NodeSprite(node);
		const auto size = sprite.Pixmap.size();
		return QRect(node.Position.x() + translation.x() - sprite.Origin.x(), node.Position.y() + translation.y() - sprite.Origin.y(), sprite.Pixmap.width(), sprite.Pixmap.height());
	}

	CNodeGraphLayer::SSprite CNodeGraphLayer::CreateSprite(const SShapeSpriteDesc& desc)
	{
		const auto points = GetShapePoints(desc.Shape);

		const float radius = desc.Radius;

		const QPointF shadowOffset = { std::round(desc.ShadowOffset.x()), std::round(desc.ShadowOffset.y()) };  // Limitation in DropShadow function

		const float radius_ceil = std::ceil(radius + desc.OutlineWidth * .5f + desc.OutlineWidth2);
		const float shadow_blur_radius_ceil = std::ceil(desc.ShadowBlurRadius);

		const QPointF bb_min = { shadowOffset.x() - radius_ceil - shadow_blur_radius_ceil, shadowOffset.y() - radius_ceil - shadow_blur_radius_ceil};
		const QPointF bb_max = { shadowOffset.x() + radius_ceil + shadow_blur_radius_ceil, shadowOffset.y() + radius_ceil + shadow_blur_radius_ceil };

		const ivec2 origin = { -(int)std::floor(bb_min.x()), -(int)std::floor(bb_min.y()) };
		const ivec2 dim = { origin.x + (int)std::ceil(bb_max.x()), origin.y + (int)std::ceil(bb_max.y()) };
		QImage image(dim.x, dim.y, QImage::Format_ARGB32);

		// Clear
		image.fill(Qt::transparent);

		// Create a QPainter to draw on the QPixmap
		QPainter painter(&image);

		painter.setRenderHint(QPainter::Antialiasing, true);

		QPolygonF polygon;
		if (EShape::Circle != desc.Shape)
		{
			polygon.reserve((int)points.size());
			for (const auto& pt : points)
			{
				polygon.append(QPointF(pt.x() * radius + (float)origin.x, pt.y() * radius + (float)origin.y));
			}
		}

		if (desc.OutlineWidth2 > 0.0f)
		{
			QPen pen;
			pen.setColor(desc.OutlineColor2); // Set the color of the circle
			pen.setWidthF(desc.OutlineWidth + desc.OutlineWidth2 * 2);       // Set the width of the circle outline
			//pen.setCapStyle(Qt::RoundCap);
			pen.setJoinStyle(Qt::RoundJoin);
			painter.setPen(pen);

			painter.setBrush(Qt::NoBrush);

			if (EShape::Circle == desc.Shape)
			{
				painter.drawEllipse(QRectF(
					(float)origin.x - radius,
					(float)origin.y - radius,
					radius * CIRCLE_SHAPE_SCALE * 2.0f,
					radius * CIRCLE_SHAPE_SCALE * 2.0f));
			}
			else
			{
				painter.drawPolygon(polygon);
			}
		}

		// Set the pen
		QPen pen;
		pen.setColor(desc.OutlineColor);
		pen.setWidthF(desc.OutlineWidth);
		pen.setJoinStyle(Qt::RoundJoin);
		painter.setPen(pen);

		// Set the brush
		QBrush brush(desc.FillColor);
		painter.setBrush(brush);

		if (EShape::Circle == desc.Shape)
		{
			painter.drawEllipse(QRectF(
				(float)origin.x - radius,
				(float)origin.y - radius,
				radius * CIRCLE_SHAPE_SCALE * 2.0f,
				radius * CIRCLE_SHAPE_SCALE * 2.0f));
		}
		else
		{
			painter.drawPolygon(polygon);
		}

		DropShadow(image, desc.ShadowColor, desc.ShadowBlurRadius, (int)shadowOffset.x(), (int)shadowOffset.y(), 2.0f);
		

		return { {origin.x - desc.Offset.x, origin.y - desc.Offset.y}, QPixmap::fromImage(image) };
	}

	void CNodeGraphLayer::RebuildNodes()
	{
		m_Nodes.clear();

		const auto model_to_screen_scale = GraphWidget().ModelToScreenScale();

		for (CGraphModel::node_index_t node_index = 0; node_index < m_GraphModel.NodeCount(); ++node_index)
		{
			const auto screen_pos_f = m_GraphModel.NodePosition(node_index) * model_to_screen_scale;
			const auto screen_pos = QPointFromRoundedQPointF(screen_pos_f);
			const auto node_category = m_GraphModel.NodeCategory(node_index);
			m_Nodes.push_back({ { screen_pos.x(), screen_pos.y() }, node_category });
		}

		m_SelectionMask.resize(m_Nodes.size());
		m_HilightMask.resize(m_Nodes.size());
		m_HilightMask.clearAll();
	}

	QRect CNodeGraphLayer::SSprite::Rect() const
	{
		const auto size = this->Pixmap.size();
		return QRect(-this->Origin.x(), -this->Origin.y(), this->Pixmap.width(), this->Pixmap.height());
	}
}

#include <moc_NodeGraphLayer.cpp>