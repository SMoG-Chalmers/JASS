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
#include "CmdSetNodeCategory.h"

namespace jass
{
	CCmdSetNodeCategory::CCmdSetNodeCategory(
		qapp::SCommandCreationContext& ctx, 
		CGraphModel& data_model, 
		const bitvec& node_mask, 
		CGraphModel::category_index_t category)
	{
		qapp::twrite(ctx.m_Data, (CGraphModel::category_index_t)category);
		qapp::twrite(ctx.m_Data, (CGraphModel::node_index_t)node_mask.count_set_bits());
		node_mask.for_each_set_bit([&](size_t node_index)
		{
			qapp::twrite(ctx.m_Data, SPerNodeData { 
				(CGraphModel::node_index_t)node_index,
				(CGraphModel::category_index_t)data_model.NodeCategory((CGraphModel::node_index_t)node_index)
			});
		});
	}

	void CCmdSetNodeCategory::Do(qapp::SCommandExecutionContext& ctx)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		auto& data_model = jass_editor->DataModel();
		const auto category = qapp::tread<CGraphModel::category_index_t>(ctx.m_Data);
		const auto node_count = qapp::tread<CGraphModel::node_index_t>(ctx.m_Data);
		data_model.BeginModifyNodes();
		qapp::for_each_in_stream<SPerNodeData>(ctx.m_Data, node_count, [&](const auto& node_data)
			{
				data_model.SetNodeCategory(node_data.Index, category);
			});
		data_model.EndModifyNodes();
	}

	void CCmdSetNodeCategory::Undo(qapp::SCommandExecutionContext& ctx)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		auto& data_model = jass_editor->DataModel();
		const auto category = qapp::tread<CGraphModel::category_index_t>(ctx.m_Data);
		const auto node_count = qapp::tread<CGraphModel::node_index_t>(ctx.m_Data);
		data_model.BeginModifyNodes();
		qapp::for_each_in_stream<SPerNodeData>(ctx.m_Data, node_count, [&](const auto& node_data)
			{
				data_model.SetNodeCategory(node_data.Index, node_data.Category);
			});
		data_model.EndModifyNodes();
	}
}