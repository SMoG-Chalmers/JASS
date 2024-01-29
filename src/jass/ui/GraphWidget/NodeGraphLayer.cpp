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

#include <qapplib/commands/CommandHistory.hpp>

#include <jass/math/Geometry.h>
#include <jass/utils/range_utils.h>
#include <jass/commands/CmdMoveNodes.h>
#include <jass/GraphEditor/JassEditor.hpp>

#include "NodeGraphLayer.hpp"

namespace jass
{
	CNodeGraphLayer::CNodeGraphLayer(CGraphWidget& graphWidget, CJassEditor& editor, std::shared_ptr<CGraphNodeTheme>&& theme)
		: CItemGraphLayer(graphWidget)
		, m_GraphModel(editor.DataModel())
		, m_SelectionModel(editor.SelectionModel())
		, m_CommandHistory(editor.CommandHistory())
	{
		SetTheme(std::move(theme));

		connect(&m_SelectionModel, &CGraphSelectionModel::SelectionChanged, this, &CNodeGraphLayer::OnSelectionChanged);
		connect(&m_GraphModel, &CGraphModel::NodesRemoved, this, &CNodeGraphLayer::OnNodesRemoved);
		connect(&m_GraphModel, &CGraphModel::NodesInserted, this, &CNodeGraphLayer::OnNodesInserted);
		connect(&m_GraphModel, &CGraphModel::NodesModified, this, &CNodeGraphLayer::OnNodesModified);

		RebuildNodes();
	}

	CNodeGraphLayer::~CNodeGraphLayer()
	{
		SetTheme(nullptr);
	}

	const CGraphNodeTheme* CNodeGraphLayer::Theme() const
	{
		return m_Theme.get();
	}

	void CNodeGraphLayer::SetTheme(std::shared_ptr<CGraphNodeTheme>&& theme)
	{
		if (m_Theme)
		{
			m_Theme->disconnect(this);
		}

		m_Theme = std::move(theme);

		if (m_Theme)
		{
			connect(m_Theme.get(), &CGraphNodeTheme::Updated, this, &CNodeGraphLayer::OnThemeUpdated);
		}

		Update();
	}

	QPoint CNodeGraphLayer::ElementPosition(element_t element) const
	{
		return QPointFromRoundedQPointF(GraphWidget().ScreenFromModel(m_GraphModel.NodePosition((CGraphModel::node_index_t)element)));
	}

	CNodeGraphLayer::EElementStyle CNodeGraphLayer::ElementStyle(element_t element) const
	{
		if (IsNodeHilighted(element))
			return EElementStyle::Hilighted;
		else if (IsNodeSelected(element))
			return EElementStyle::Selected;
		return EElementStyle::Normal;
	}

	QRect CNodeGraphLayer::ItemRect(element_t element) const
	{
		return m_Theme->ElementLocalRect(element, ElementStyle(element)).translated(ElementPosition(element));
	}

	void CNodeGraphLayer::DrawItem(element_t element, QPainter& painter, const QRect& rc) const
	{
		m_Theme->DrawElement(element, ElementStyle(element), ElementPosition(element), painter);
	}

	void CNodeGraphLayer::SetHilighted(element_t element, bool hilighted)
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

	bool CNodeGraphLayer::CanMoveElements() const
	{
		return true;
	}

	void CNodeGraphLayer::BeginMoveElements(const bitvec& element_mask)
	{
		m_MoveElementMask = element_mask;
		m_TempPoints.clear();
		m_TempPoints.reserve(m_TempSelectionMask.count_set_bits());
		element_mask.for_each_set_bit([&](size_t node_index)
			{
				m_TempPoints.push_back(m_GraphModel.NodePosition((CGraphModel::node_index_t)node_index));
			});
	}

	void CNodeGraphLayer::MoveElements(const QPoint& delta)
	{
		const QPointF delta_move_model = QPointF(delta) * GraphWidget().ScreenToModelScale();

		m_GraphModel.BeginModifyNodes();
		m_MoveElementMask.for_each_set_bit([&](size_t node_index)
			{
				const auto new_pos = m_GraphModel.NodePosition((CGraphModel::node_index_t)node_index) + delta_move_model;
				m_GraphModel.SetNodePosition((CGraphModel::node_index_t)node_index, new_pos);
			});
		m_GraphModel.EndModifyNodes();
	}

	void CNodeGraphLayer::EndMoveElements(bool apply)
	{
		{
			m_GraphModel.BeginModifyNodes();
			size_t n = 0;
			m_MoveElementMask.for_each_set_bit([&](size_t node_index)
				{
					const auto position = m_GraphModel.NodePosition((CGraphModel::node_index_t)node_index);
					m_GraphModel.SetNodePosition((CGraphModel::node_index_t)node_index, m_TempPoints[n]);
					m_TempPoints[n] = position;
					++n;
				});
			m_GraphModel.EndModifyNodes();
		}

		if (apply)
		{
			m_CommandHistory.NewCommand<CCmdMoveNodes>(m_GraphModel, m_MoveElementMask, to_const_span(m_TempPoints));
		}
	}

	void CNodeGraphLayer::OnSelectionChanged()
	{
		m_TempSelectionMask.bitwise_xor(m_SelectionMask, m_SelectionModel.NodeMask());

		m_TempSelectionMask.for_each_set_bit([&](size_t node_index)
			{
				m_SelectionMask.toggle(node_index);
				Update(ItemRect(node_index).united(LastItemRect(node_index)));
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

	void CNodeGraphLayer::OnThemeUpdated()
	{
		Update();
	}

	void CNodeGraphLayer::RebuildNodes()
	{
		ClearItems();
		InsertItems(0, m_GraphModel.NodeCount());
		m_SelectionMask.resize(m_GraphModel.NodeCount());
		m_HilightMask.resize(m_GraphModel.NodeCount());
		m_HilightMask.clearAll();
	}
}

#include <moc_NodeGraphLayer.cpp>