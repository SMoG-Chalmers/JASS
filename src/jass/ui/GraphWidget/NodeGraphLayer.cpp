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
	CNodeGraphLayer::CNodeGraphLayer(CGraphWidget& graphWidget, CGraphModel& graph_model, CCategorySet& categories, CGraphSelectionModel& selection_model)
		: CGraphLayer(graphWidget)
		, m_GraphModel(graph_model)
		, m_Categories(categories)
		, m_SelectionModel(selection_model)
		, m_Sprites(categories)
	{
		connect(&selection_model, &CGraphSelectionModel::SelectionChanged, this, &CNodeGraphLayer::OnSelectionChanged);
		connect(&graph_model, &CGraphModel::NodesRemoved, this, &CNodeGraphLayer::OnNodesRemoved);
		connect(&graph_model, &CGraphModel::NodesInserted, this, &CNodeGraphLayer::OnNodesInserted);
		connect(&graph_model, &CGraphModel::NodesModified, this, &CNodeGraphLayer::OnNodesModified);

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
			const auto sprite_index = NodeSpriteIndex(node);
			const auto nodePos = NodeScreenPos(node_index);
			const auto nodeRect = m_Sprites.SpriteRect(sprite_index).translated(nodePos);
			const auto rcVis = nodeRect.intersected(rcClip);
			if (rcVis.isEmpty())
			{
				continue;
			}
			m_Sprites.DrawSprite(sprite_index, painter, nodePos);
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

	size_t CNodeGraphLayer::NodeSpriteIndex(const SNode& node) const
	{
		const auto node_index = (CGraphModel::node_index_t)(&node - m_Nodes.data());
		CNodeSpriteSet::EStyle style;
		if (IsNodeHilighted(&node - m_Nodes.data()))
			style = CNodeSpriteSet::EStyle::Hilighted;
		else if (IsNodeSelected(node))
			style = CNodeSpriteSet::EStyle::Selected;
		else
			style = CNodeSpriteSet::EStyle::Normal;
		return m_Sprites.SpriteIndex(m_GraphModel.NodeCategory(node_index), style);
	}

	bool CNodeGraphLayer::IsNodeSelected(const SNode& node) const
	{
		return m_SelectionMask.get(&node - m_Nodes.data());
	}

	QRect CNodeGraphLayer::NodeRect(const SNode& node) const
	{
		const auto sprite_index = NodeSpriteIndex(node);
		const size_t node_index = &node - m_Nodes.data();
		const auto nodePos = NodeScreenPos(node_index);
		return m_Sprites.SpriteRect(sprite_index).translated(nodePos);
	}

	void CNodeGraphLayer::RebuildNodes()
	{
		m_Nodes.clear();
		m_Nodes.resize(m_GraphModel.NodeCount());
		m_SelectionMask.resize(m_Nodes.size());
		m_HilightMask.resize(m_Nodes.size());
		m_HilightMask.clearAll();
	}
}

#include <moc_NodeGraphLayer.cpp>