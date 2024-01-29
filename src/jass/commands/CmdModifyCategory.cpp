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
#include <jass/GraphEditor/CategorySet.hpp>
#include <jass/GraphEditor/JassEditor.hpp>

#include "CmdModifyCategory.h"

namespace jass
{
	CCmdModifyCategory::CCmdModifyCategory(qapp::SCommandCreationContext& ctx, size_t category_index, const QString& name, QRgb color, EShape shape)
	{
		auto* editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(editor);
		const auto& categories = editor->Categories();

		qapp::twrite(ctx.m_Data, (uint32_t)category_index);

		// Old
		qapp::twrite(ctx.m_Data, categories.Name(category_index));
		qapp::twrite(ctx.m_Data, categories.Color(category_index));
		qapp::twrite(ctx.m_Data, (uint8_t)categories.Shape(category_index));
		
		// New
		qapp::twrite(ctx.m_Data, name);
		qapp::twrite(ctx.m_Data, color);
		qapp::twrite(ctx.m_Data, shape);
	}

	void CCmdModifyCategory::Do(qapp::SCommandExecutionContext& ctx)
	{
		auto* editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(editor);

		const auto category_index = qapp::tread<uint32_t>(ctx.m_Data);

		// Old
		qapp::tread<QString>(ctx.m_Data);
		qapp::tread<QRgb>(ctx.m_Data);
		qapp::tread<uint8_t>(ctx.m_Data);

		// New
		const auto name = qapp::tread<QString>(ctx.m_Data);
		const auto color = qapp::tread<QRgb>(ctx.m_Data);
		const auto shape = (EShape)qapp::tread<uint8_t>(ctx.m_Data);

		editor->Categories().SetCategory(category_index, name, color, shape);
	}

	void CCmdModifyCategory::Undo(qapp::SCommandExecutionContext& ctx)
	{
		auto* editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(editor);

		const auto category_index = qapp::tread<uint32_t>(ctx.m_Data);

		// Old
		const auto name = qapp::tread<QString>(ctx.m_Data);
		const auto color = qapp::tread<QRgb>(ctx.m_Data);
		const auto shape = (EShape)qapp::tread<uint8_t>(ctx.m_Data);

		// New
		qapp::tread<QString>(ctx.m_Data);
		qapp::tread<QRgb>(ctx.m_Data);
		qapp::tread<uint8_t>(ctx.m_Data);
		
		editor->Categories().SetCategory(category_index, name, color, shape);
	}
}