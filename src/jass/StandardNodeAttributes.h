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

#include <jass/graphdata/GraphDataCommon.h>
#include "GraphModel.hpp"

namespace jass
{
	typedef CNodeAttribute<QPointF> JPosition_NodeAttribute_t;
	inline JPosition_NodeAttribute_t* TryGetJPositionNodeAttribute(CGraphModel& model)
	{
		return model.TryGetNodeAttribute<JPosition_NodeAttribute_t::value_t>(GRAPH_NODE_ATTTRIBUTE_JUSTIFIED_POSITION);
	}
}