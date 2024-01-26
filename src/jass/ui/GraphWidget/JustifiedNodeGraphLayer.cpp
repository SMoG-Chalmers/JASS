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

#include <qapplib/commands/CommandHistory.hpp>

#include <jass/math/Geometry.h>
#include <jass/utils/range_utils.h>
#include <jass/commands/CmdSetNodeAttributes.h>
#include <jass/GraphEditor/Analyses.hpp>
#include <jass/GraphEditor/JassEditor.hpp>

#include "JustifiedNodeGraphLayer.hpp"

namespace jass
{
	CJustifiedNodeGraphLayer::CJustifiedNodeGraphLayer(CGraphWidget& graphWidget, CJassEditor& editor)
		: CSpriteGraphLayer(graphWidget, &m_Sprites)
		, m_GraphModel(editor.DataModel())
		, m_SelectionModel(editor.SelectionModel())
		, m_Analyses(editor.Analyses())
		, m_CommandHistory(editor.CommandHistory())
		, m_Sprites(editor.Categories())
	{
		m_JPositionNodeAttribute = TryGetJPositionNodeAttribute(m_GraphModel);
		connect(&m_SelectionModel, &CGraphSelectionModel::SelectionChanged, this, &CJustifiedNodeGraphLayer::OnSelectionChanged);
		connect(&m_GraphModel, &CGraphModel::NodesRemoved, this, &CJustifiedNodeGraphLayer::OnNodesRemoved);
		connect(&m_GraphModel, &CGraphModel::NodesInserted, this, &CJustifiedNodeGraphLayer::OnNodesInserted);
		connect(&m_GraphModel, &CGraphModel::NodesModified, this, &CJustifiedNodeGraphLayer::OnNodesModified);
		RebuildNodes();
	}

	QPoint CJustifiedNodeGraphLayer::ItemPosition(element_t element) const
	{
		if (m_JPositionNodeAttribute)
		{
			auto pos = m_JPositionNodeAttribute->Value(element);
			if (pos.y() >= 0)
			{
				//pos = pos * 50;
				return QPointFromRoundedQPointF(GraphWidget().ScreenFromModel(pos));
			}
		}
		// Invalid
		return QPoint(-12345, -12345);
	}

	size_t CJustifiedNodeGraphLayer::ItemSpriteIndex(element_t element) const
	{
		const auto node_index = (CGraphModel::node_index_t)element;
		CNodeSpriteSet::EStyle style;
		if (IsNodeHilighted(element))
			style = CNodeSpriteSet::EStyle::Hilighted;
		else if (IsNodeSelected(element))
			style = CNodeSpriteSet::EStyle::Selected;
		else
			style = CNodeSpriteSet::EStyle::Normal;
		return m_Sprites.SpriteIndex(m_GraphModel.NodeCategory(node_index), style);
	}

	void CJustifiedNodeGraphLayer::DrawItem(element_t element, QPainter& painter, const QRect& rc) const
	{
		if (!m_JPositionNodeAttribute || m_JPositionNodeAttribute->Value(element).y() < 0)
		{
			return;
		}
		CSpriteGraphLayer::DrawItem(element, painter, rc);
	}

	void CJustifiedNodeGraphLayer::SetHilighted(element_t element, bool hilighted)
	{
		if (m_HilightMask.get(element) == hilighted)
		{
			return;
		}
		if (hilighted)
			m_HilightMask.set(element);
		else
			m_HilightMask.clear(element);
		Update(ItemRect(element).united(LastItemRect(element)));
	}

	void CJustifiedNodeGraphLayer::GetSelection(bitvec& out_selection_mask) const
	{
		out_selection_mask = m_SelectionModel.NodeMask();
	}

	void CJustifiedNodeGraphLayer::SetSelection(const bitvec& selection_mask) const
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

	void CJustifiedNodeGraphLayer::OnViewChanged(const QRect& rc, float screen_to_model_scale)
	{
		RebuildNodes();
	}

	bool CJustifiedNodeGraphLayer::CanMoveElements() const
	{
		ASSERT(m_JPositionNodeAttribute);
		return nullptr != m_JPositionNodeAttribute;
	}

	void CJustifiedNodeGraphLayer::BeginMoveElements(const bitvec& element_mask)
	{
		m_MoveElementMask = element_mask;
		m_TempPoints.clear();
		m_TempPoints.reserve(m_TempSelectionMask.count_set_bits());
		element_mask.for_each_set_bit([&](size_t node_index)
			{
				m_TempPoints.push_back(m_JPositionNodeAttribute->Value(node_index));
			});
	}

	void CJustifiedNodeGraphLayer::MoveElements(const QPoint& delta)
	{
		ASSERT(m_JPositionNodeAttribute);

		const QPointF delta_move(GraphWidget().ScreenToModelScale() * delta.x(), 0);

		m_JPositionNodeAttribute->BeginModify();
		m_MoveElementMask.for_each_set_bit([&](size_t node_index)
			{
				m_JPositionNodeAttribute->SetValue(node_index, m_JPositionNodeAttribute->Value(node_index) + delta_move);
			});
		m_JPositionNodeAttribute->EndModify();
	}

	void CJustifiedNodeGraphLayer::EndMoveElements(bool apply)
	{
		ASSERT(m_JPositionNodeAttribute);
		
		{
			m_JPositionNodeAttribute->BeginModify();
			size_t n = 0;
			m_MoveElementMask.for_each_set_bit([&](size_t node_index)
				{
					const auto temp = m_JPositionNodeAttribute->Value(node_index);
					m_JPositionNodeAttribute->SetValue(node_index, m_TempPoints[n]);
					m_TempPoints[n] = temp;
					++n;
				});
			m_JPositionNodeAttribute->EndModify();
		}

		if (apply)
		{
			m_CommandHistory.NewCommand<CCmdSetNodeAttributes<JPosition_NodeAttribute_t::value_t>>(m_JPositionNodeAttribute, m_MoveElementMask, to_const_span(m_TempPoints));
		}
	}

	void CJustifiedNodeGraphLayer::OnSelectionChanged()
	{
		m_TempSelectionMask.bitwise_xor(m_SelectionMask, m_SelectionModel.NodeMask());

		m_TempSelectionMask.for_each_set_bit([&](size_t node_index)
			{
				m_SelectionMask.toggle(node_index);
				Update(ItemRect(node_index).united(LastItemRect(node_index)));
			});
	}

	void CJustifiedNodeGraphLayer::OnNodesRemoved(const CGraphModel::const_node_indices_t& node_indices)
	{
		RebuildNodes();
		Update();
	}

	void CJustifiedNodeGraphLayer::OnNodesInserted(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table)
	{
		RebuildNodes();
		Update();
	}

	void CJustifiedNodeGraphLayer::OnNodesModified(const bitvec& node_mask)
	{
		QRect rcUpdate;
		node_mask.for_each_set_bit([&](const size_t node_index)
			{
				if (rcUpdate.isEmpty())
				{
					rcUpdate = ItemRect(node_index);
				}
				else
				{
					rcUpdate = rcUpdate.united(ItemRect(node_index));
				}
				rcUpdate = rcUpdate.united(LastItemRect(node_index));
			});
		if (!rcUpdate.isEmpty())
		{
			Update(rcUpdate);
		}
	}

	void CJustifiedNodeGraphLayer::RebuildNodes()
	{
		ClearItems();
		InsertItems(0, m_GraphModel.NodeCount());
		m_SelectionMask.resize(m_GraphModel.NodeCount());
		m_HilightMask.resize(m_GraphModel.NodeCount());
		m_HilightMask.clearAll();
	}
}

#include <moc_JustifiedNodeGraphLayer.cpp>