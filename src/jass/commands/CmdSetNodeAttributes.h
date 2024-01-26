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

#pragma once

#include <qapplib/commands/Command.h>
#include <qapplib/utils/StreamUtils.h>
#include <jass/GraphModel.hpp>

namespace jass
{
	template <class T>
	class CCmdSetNodeAttributes: public qapp::ICommand
	{
	public:
		CCmdSetNodeAttributes(qapp::SCommandCreationContext& ctx, CNodeAttribute<T>* node_attribute, const bitvec& node_mask, const std::span<const T>& values)
			: m_NodeAttribute(node_attribute)
		{
			qapp::twrite(ctx.m_Data, (uint32_t)values.size());
			size_t n = 0;
			node_mask.for_each_set_bit([&](size_t node_index)
				{
					qapp::twrite<SNodeData>(ctx.m_Data, SNodeData
						{
							(uint32_t)node_index,
							node_attribute->Value(node_index),
							values[n++]
						});
				});
		}

		void Do(qapp::SCommandExecutionContext& ctx) override
		{
			const auto count = qapp::tread<uint32_t>(ctx.m_Data);
			m_NodeAttribute->BeginModify();
			qapp::for_each_in_stream<SNodeData>(ctx.m_Data, count, [&](const auto& node_data)
				{
					m_NodeAttribute->SetValue(node_data.Index, node_data.NewValue);
				});
			m_NodeAttribute->EndModify();
		}

		void Undo(qapp::SCommandExecutionContext& ctx) override
		{
			const auto count = qapp::tread<uint32_t>(ctx.m_Data);
			m_NodeAttribute->BeginModify();
			qapp::for_each_in_stream<SNodeData>(ctx.m_Data, count, [&](const auto& node_data)
				{
					m_NodeAttribute->SetValue(node_data.Index, node_data.OldValue);
				});
			m_NodeAttribute->EndModify();
		}
	
	private:
		CNodeAttribute<T>* m_NodeAttribute = nullptr;

		struct SNodeData
		{
			uint32_t Index;
			T        OldValue;
			T        NewValue;
		};
	};
}