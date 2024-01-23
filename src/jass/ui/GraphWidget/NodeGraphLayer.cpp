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
		connect(&selection_model, &CGraphSelectionModel::SelectionChanged, this, &CNodeGraphLayer::OnSelectionChanged);
		connect(&graph_model, &CGraphModel::NodesRemoved, this, &CNodeGraphLayer::OnNodesRemoved);
		connect(&graph_model, &CGraphModel::NodesInserted, this, &CNodeGraphLayer::OnNodesInserted);
		connect(&graph_model, &CGraphModel::NodesModified, this, &CNodeGraphLayer::OnNodesModified);

		connect(&categories, &CCategorySet::rowsInserted, this, &CNodeGraphLayer::OnCategoriesInserted);
		connect(&categories, &CCategorySet::rowsRemoved, this, &CNodeGraphLayer::OnCategoriesRemoved);
		connect(&categories, &CCategorySet::CategoriesRemapped, this, &CNodeGraphLayer::OnCategoriesRemapped);
		connect(&categories, &CCategorySet::dataChanged, this, &CNodeGraphLayer::OnCategoriesChanged);

		UpdateSprites();

		RebuildNodes();
	}

	QPoint CNodeGraphLayer::NodeScreenPos(element_t node) const
	{
		return QPointFromRoundedQPointF(GraphWidget().ScreenFromModel(m_GraphModel.NodePosition((CGraphModel::node_index_t)node)));
	}

	void CNodeGraphLayer::Paint(QPainter& painter, const QRect& rcClip)
	{
		for (size_t node_index = 0; node_index < m_Nodes.size(); ++node_index)
		{
			auto& node = m_Nodes[node_index];
			const auto& sprite = NodeSprite(node);
			const auto nodeRect = NodeRect(node);
			const auto rcVis = nodeRect.intersected(rcClip);
			if (rcVis.isEmpty())
			{
				continue;
			}
			const QRect srcRect(rcVis.left() - nodeRect.left(), rcVis.top() - nodeRect.top(), rcVis.width(), rcVis.height());
			painter.drawPixmap(rcVis.topLeft(), sprite.Pixmap, srcRect);
			node.LastRect = nodeRect;
		}
	}

	CNodeGraphLayer::element_t CNodeGraphLayer::HitTest(const QPoint& pt)
	{
		for (size_t node_index = m_Nodes.size() - 1; node_index < m_Nodes.size(); --node_index)
		{
			const auto& node = m_Nodes[node_index];
			if (node.LastRect.contains(pt))
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
			if (hitRect.contains(NodeScreenPos(node_index)))
			{
				out_hit_elements.set(node_index);
				any_hit = true;
			}
		}
		return any_hit;
	}

	void CNodeGraphLayer::SetHilighted(element_t node_index, bool hilighted)
	{
		if (m_HilightMask.get(node_index) == hilighted)
		{
			return;
		}
		if (hilighted)
			m_HilightMask.set(node_index);
		else
			m_HilightMask.clear(node_index);
		const auto& node = m_Nodes[node_index];
		Update(NodeRect(node).united(node.LastRect));
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
		QRect rcUpdate;
		node_mask.for_each_set_bit([&](const size_t node_index)
			{
				const auto& node = m_Nodes[node_index];
				if (rcUpdate.isEmpty())
				{
					rcUpdate = NodeRect(node);
				}
				else
				{
					rcUpdate = rcUpdate.united(NodeRect(node));
				}
				if (!node.LastRect.isEmpty())
				{
					rcUpdate = rcUpdate.united(node.LastRect);
				}
			});
		if (!rcUpdate.isEmpty())
		{
			Update(rcUpdate);
		}
	}

	void CNodeGraphLayer::OnCategoriesInserted(const QModelIndex& parent, int first, int last)
	{
		const auto inserted_count = last - first + 1;
		m_Sprites.insert(m_Sprites.begin() + first * SPRITE_COUNT_PER_CATEGORY, inserted_count * SPRITE_COUNT_PER_CATEGORY, SSprite());
		m_CategoryCount += inserted_count;
		for (int category_index = first; category_index <= last; ++category_index)
		{
			UpdateSpritesForCategory((size_t)category_index);
		}
	}

	void CNodeGraphLayer::OnCategoriesRemoved(const QModelIndex& parent, int first, int last)
	{
		const auto removed_count = last - first + 1;
		m_Sprites.erase(m_Sprites.begin() + first * SPRITE_COUNT_PER_CATEGORY, m_Sprites.begin() + (last + 1) * SPRITE_COUNT_PER_CATEGORY);
		m_CategoryCount -= removed_count;
	}

	void CNodeGraphLayer::OnCategoriesRemapped(const std::span<const size_t>&)
	{
		UpdateSprites();
	}

	void CNodeGraphLayer::OnCategoriesChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& /*roles*/)
	{
		for (auto category_index = (size_t)topLeft.row(); category_index <= (size_t)bottomRight.row(); ++category_index)
		{
			UpdateSpritesForCategory(category_index);
		}
	}

	const CNodeGraphLayer::SSprite& CNodeGraphLayer::NodeSprite(const SNode& node) const
	{
		const auto node_index = (CGraphModel::node_index_t)(&node - m_Nodes.data());
		ENodeSpriteStyle style;
		if (IsNodeHilighted(&node - m_Nodes.data()))
			style = ENodeSpriteStyle::Hilighted;
		else if (IsNodeSelected(node))
			style = ENodeSpriteStyle::Selected;
		else
			style = ENodeSpriteStyle::Normal;
		return NodeSprite(m_GraphModel.NodeCategory(node_index), style);
	}

	bool CNodeGraphLayer::IsNodeSelected(const SNode& node) const
	{
		return m_SelectionMask.get(&node - m_Nodes.data());
	}

	QRect CNodeGraphLayer::NodeRect(const SNode& node) const
	{
		const size_t node_index = &node - m_Nodes.data();
		return NodeSprite(node).Rect().translated(NodeScreenPos(node_index));
	}

	void CNodeGraphLayer::UpdateSprites()
	{
		m_CategoryCount = (uint32_t)m_Categories.Size();

		// NOTE: Last one will be "None" (with same index as number of categories)
		m_Sprites.resize((m_CategoryCount + 1) * SPRITE_COUNT_PER_CATEGORY);

		for (size_t category_index = 0; category_index <= m_CategoryCount; ++category_index)
		{
			UpdateSpritesForCategory(category_index);
		}
	}

	void CNodeGraphLayer::UpdateSpritesForCategory(size_t category_index)
	{
		SShapeSpriteDesc sprite_desc;
		sprite_desc.Radius = 9.5f;
		sprite_desc.OutlineWidth = 3.0f;
		sprite_desc.OutlineWidth2 = 0;
		sprite_desc.ShadowOffset = { 1, 1 };
		sprite_desc.ShadowBlurRadius = 3.0f;
		sprite_desc.OutlineColor = Qt::white;
		sprite_desc.ShadowColor = qRgba(0, 0, 0, 255);
		sprite_desc.Shape = m_Categories.Shape(category_index);

		sprite_desc.FillColor = QColor::fromRgba(m_Categories.Color(category_index));
		m_Sprites[category_index * SPRITE_COUNT_PER_CATEGORY + 0] = CreateSprite(sprite_desc);

		sprite_desc.OutlineColor2 = QColor::fromRgba(COLOR_SELECTED);
		sprite_desc.OutlineWidth2 = sprite_desc.OutlineWidth;
		m_Sprites[category_index * SPRITE_COUNT_PER_CATEGORY + 1] = CreateSprite(sprite_desc);

		sprite_desc.OutlineColor2 = QColor::fromRgba(COLOR_HILIGHT);
		sprite_desc.OutlineWidth2 = 4;
		sprite_desc.FillColor = Blend(m_Categories.Color(category_index), 0xFFFFFFFF, 64);
		m_Sprites[category_index * SPRITE_COUNT_PER_CATEGORY + 2] = CreateSprite(sprite_desc);
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
		m_Nodes.resize(m_GraphModel.NodeCount());
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