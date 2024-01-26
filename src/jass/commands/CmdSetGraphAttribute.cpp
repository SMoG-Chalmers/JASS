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

#include <QtCore/qvariant.h>
#include <qapplib/utils/StreamUtils.h>
#include <jass/GraphEditor/JassEditor.hpp>
#include "CmdSetGraphAttribute.h"

namespace jass
{
	CCmdSetGraphAttribute::CCmdSetGraphAttribute(qapp::SCommandCreationContext& ctx, CGraphModel::attribute_index_t attribute_index, const QVariant& value)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		const auto& data_model = jass_editor->DataModel();

		qapp::twrite(ctx.m_Data, attribute_index);
		qapp::twrite(ctx.m_Data, data_model.AttributeValue(attribute_index));
		qapp::twrite(ctx.m_Data, value);
	}

	void CCmdSetGraphAttribute::Do(qapp::SCommandExecutionContext& ctx)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		const auto attribute_index = qapp::tread<CGraphModel::attribute_index_t>(ctx.m_Data);
		qapp::tread<QVariant>(ctx.m_Data); // Skip
		jass_editor->DataModel().SetAttribute(attribute_index, qapp::tread<QVariant>(ctx.m_Data));
	}

	void CCmdSetGraphAttribute::Undo(qapp::SCommandExecutionContext& ctx)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&ctx.m_Editor);
		ASSERT(jass_editor);
		const auto attribute_index = qapp::tread<CGraphModel::attribute_index_t>(ctx.m_Data);
		jass_editor->DataModel().SetAttribute(attribute_index, qapp::tread<QVariant>(ctx.m_Data));
	}
}