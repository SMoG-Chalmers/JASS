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

#include <jass/JassDocument.hpp>
#include <jass/GraphEditor/JassEditor.hpp>
#include "CmdSetBackgroundImage.h"

namespace jass
{
	CCmdSetBackgroundImage::CCmdSetBackgroundImage(qapp::SCommandCreationContext& ctx, CJassEditor& editor, QByteArray&& image_data, QString extension_no_dot)
		: m_OldImageData(editor.JassDocument().ImageData())
		, m_OldExtensionNoDot(editor.JassDocument().ImageExtensionNoDot())
		, m_NewImageData(std::move(image_data))
		, m_NewExtensionNoDot(extension_no_dot)
	{
	}
	
	void CCmdSetBackgroundImage::Do(qapp::SCommandExecutionContext& ctx)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		jass_editor->SetBackgroundImage(m_NewImageData, m_NewExtensionNoDot);
	}

	void CCmdSetBackgroundImage::Undo(qapp::SCommandExecutionContext& ctx)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		jass_editor->SetBackgroundImage(m_OldImageData, m_OldExtensionNoDot);
	}
}