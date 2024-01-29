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

#include <jass/Debug.h>
#include <jass/GraphModel.hpp>
#include <jass/GraphUtils.h>

#include "GraphOperations.h"
#include "CmdCreateNode.h"

namespace jass
{
	CCmdCreateNode::CCmdCreateNode(qapp::SCommandCreationContext& ctx, CGraphModel& data_model, const SNodeDesc& node_desc)
	{
		WriteInsertGraphNodesOp(ctx.m_Data, [&](auto&& add)
			{
				add(node_desc.Index, node_desc.Category, node_desc.PositionF.x(), node_desc.PositionF.y());
			});
	}
}