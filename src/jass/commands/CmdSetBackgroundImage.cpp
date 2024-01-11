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