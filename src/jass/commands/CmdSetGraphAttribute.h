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

#pragma once

#include <qapplib/commands/Command.h>
#include <jass/GraphModel.hpp>

namespace jass
{
	class CCmdSetGraphAttribute: public qapp::ICommand
	{
	public:
		CCmdSetGraphAttribute(qapp::SCommandCreationContext& ctx, CGraphModel::attribute_index_t attribute_index, const QVariant& value);
		void Do(qapp::SCommandExecutionContext& ctx) override;
		void Undo(qapp::SCommandExecutionContext& ctx) override;
	};
}