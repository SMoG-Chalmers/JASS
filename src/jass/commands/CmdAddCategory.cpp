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

#include "CmdAddCategory.h"

namespace jass
{
	CCmdAddCategory::CCmdAddCategory(qapp::SCommandCreationContext& ctx, const QString& name, QRgb color, EShape shape)
	{
		qapp::twrite(ctx.m_Data, name);
		qapp::twrite(ctx.m_Data, color);
		qapp::twrite(ctx.m_Data, (uint8_t)shape);
	}

	void CCmdAddCategory::Do(qapp::SCommandExecutionContext& ctx)
	{
		auto* editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(editor);

		const auto name = qapp::tread<QString>(ctx.m_Data);
		const auto color = qapp::tread<QRgb>(ctx.m_Data);
		const auto shape = (EShape)qapp::tread<uint8_t>(ctx.m_Data);

		editor->Categories().AddCategory(name, color, shape);
	}

	void CCmdAddCategory::Undo(qapp::SCommandExecutionContext& ctx)
	{
		auto* editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(editor);

		editor->Categories().RemoveCategory(editor->Categories().Size() - 1);
	}
}