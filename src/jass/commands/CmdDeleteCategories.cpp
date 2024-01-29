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
#include <jass/GraphEditor/CategorySet.hpp>
#include <jass/GraphEditor/JassEditor.hpp>

#include "CmdDeleteCategories.h"

namespace jass
{
	CCmdDeleteCategories::CCmdDeleteCategories(qapp::SCommandCreationContext& ctx, std::span<const size_t> category_indexes)
	{
		auto* editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(editor);

		const auto& data_model = editor->DataModel();
		const auto& categories = editor->Categories();

		bitvec node_mask;
		node_mask.resize(data_model.NodeCount());

		qapp::twrite(ctx.m_Data, (uint32_t)category_indexes.size());
		int dbg_last_category_index = -1;
		for (auto i : category_indexes)
		{
			const auto category_index = (CGraphModel::category_index_t)i;
			
			// Verify indices are in order, since we rely on this
			ASSERT((int)category_index > dbg_last_category_index);
			dbg_last_category_index = (int)category_index;

			qapp::twrite(ctx.m_Data, category_index);
			qapp::twrite(ctx.m_Data, categories.Name(category_index));
			qapp::twrite(ctx.m_Data, categories.Color(category_index));
			qapp::twrite(ctx.m_Data, (uint8_t)categories.Shape(category_index));

			node_mask.clearAll();
			for (CGraphModel::node_index_t node_index = 0; node_index < data_model.NodeCount(); ++node_index)
			{
				if (data_model.NodeCategory(node_index) == (CGraphModel::category_index_t)category_index)
				{
					node_mask.set(node_index);
				}
			}
			ctx.m_Data.write((const char*)node_mask.data(), node_mask.dataByteSize());
		}
	}

	void CCmdDeleteCategories::Do(qapp::SCommandExecutionContext& ctx)
	{
		auto* editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(editor);

		auto& data_model = editor->DataModel();
		auto& categories = editor->Categories();

		bitvec node_mask;
		node_mask.resize(data_model.NodeCount());

		data_model.BeginModifyNodes();

		const auto category_index_count = qapp::tread<uint32_t>(ctx.m_Data);
		for (uint32_t i = 0; i < category_index_count; ++i)
		{
			const auto category_index = qapp::tread<CGraphModel::category_index_t>(ctx.m_Data);
			categories.RemoveCategory(category_index - i);  // Adjust index to number of categories already removed
			qapp::tread<QString>(ctx.m_Data);  // Skip
			qapp::tread<QRgb>(ctx.m_Data);     // Skip
			qapp::tread<uint8_t>(ctx.m_Data);  // Skip
			qapp::read_exact(ctx.m_Data, node_mask.data(), node_mask.dataByteSize());
			//node_mask.for_each_set_bit([&](size_t node_index)
			//	{
			//		data_model.SetNodeCategory((CGraphModel::node_index_t)node_index, CGraphModel::NO_CATEGORY);
			//	});
		}

		data_model.EndModifyNodes();
	}

	void CCmdDeleteCategories::Undo(qapp::SCommandExecutionContext& ctx)
	{
		auto* editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(editor);

		auto& data_model = editor->DataModel();
		auto& categories = editor->Categories();

		bitvec node_mask;
		node_mask.resize(data_model.NodeCount());

		data_model.BeginModifyNodes();

		const auto category_index_count = qapp::tread<uint32_t>(ctx.m_Data);
		for (uint32_t i = 0; i < category_index_count; ++i)
		{
			const auto category_index = qapp::tread<CGraphModel::category_index_t>(ctx.m_Data);
			const auto name = qapp::tread<QString>(ctx.m_Data);
			const auto color = qapp::tread<QRgb>(ctx.m_Data);
			const auto shape = (EShape)qapp::tread<uint8_t>(ctx.m_Data);
			categories.InsertCategory(category_index, name, color, shape);
			qapp::read_exact(ctx.m_Data, node_mask.data(), node_mask.dataByteSize());
			node_mask.for_each_set_bit([&](size_t node_index)
				{
					data_model.SetNodeCategory((CGraphModel::node_index_t)node_index, category_index);
				});
		}

		data_model.EndModifyNodes();
	}
}