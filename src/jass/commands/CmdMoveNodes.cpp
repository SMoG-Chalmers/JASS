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

#include <qapplib/utils/StreamUtils.h>
#include <jass/Debug.h>
#include <jass/GraphModel.hpp>
#include <jass/GraphUtils.h>
#include <jass/GraphEditor/JassEditor.hpp>
#include "CmdMoveNodes.h"

namespace jass
{
	CCmdMoveNodes::CCmdMoveNodes(
		qapp::SCommandCreationContext& ctx, 
		CGraphModel& data_model, 
		const bitvec& node_mask, 
		const std::span<const QPointF>& new_positions)
	{
		qapp::twrite(ctx.m_Data, (uint32_t)new_positions.size());
		size_t n = 0;
		node_mask.for_each_set_bit([&](size_t node_index)
		{
			SNode node;
			node.Index = (uint32_t)node_index;
			node.OldPos = data_model.NodePosition((CGraphModel::node_index_t)node_index);
			node.NewPos = new_positions[n++];
			qapp::twrite(ctx.m_Data, node);
		});
	}

	void CCmdMoveNodes::Do(qapp::SCommandExecutionContext& ctx)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		auto& data_model = jass_editor->DataModel();
		const auto count = qapp::tread<uint32_t>(ctx.m_Data);
		data_model.BeginModifyNodes();
		qapp::for_each_in_stream<SNode>(ctx.m_Data, count, [&](const auto& node)
			{
				data_model.SetNodePosition(node.Index, node.NewPos);
			});
		data_model.EndModifyNodes();
	}

	void CCmdMoveNodes::Undo(qapp::SCommandExecutionContext& ctx)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		auto& data_model = jass_editor->DataModel();
		const auto count = qapp::tread<uint32_t>(ctx.m_Data);
		data_model.BeginModifyNodes();
		qapp::for_each_in_stream<SNode>(ctx.m_Data, count, [&](const auto& node)
			{
				data_model.SetNodePosition(node.Index, node.OldPos);
			});
		data_model.EndModifyNodes();
	}
}