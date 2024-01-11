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

#include <qapplib/utils/StreamUtils.h>
#include <jass/Debug.h>
#include <jass/GraphModel.hpp>
#include <jass/GraphUtils.h>
#include <jass/GraphEditor/JassEditor.hpp>
#include "CmdFilpNodes.h"

namespace jass
{
	static std::vector<QPointF> FlippedPositions(CGraphModel& data_model, const bitvec& node_mask, CCmdFilpNodes::EOrientation orientation)
	{
		QPointF ptCenter;
		size_t count = 0;
		node_mask.for_each_set_bit([&](size_t node_index)
			{
				if (count == 0)
					ptCenter = data_model.NodePosition((CGraphModel::node_index_t)node_index);
				else
					ptCenter += data_model.NodePosition((CGraphModel::node_index_t)node_index);
				++count;
			});

		if (count)
		{
			ptCenter = ptCenter * (1.f / count);
		}

		std::vector<QPointF> new_positions;
		new_positions.reserve(count);

		node_mask.for_each_set_bit([&](size_t node_index)
			{
				auto pt = data_model.NodePosition((CGraphModel::node_index_t)node_index);
				if (orientation == CCmdFilpNodes::Horizontal)
				{
					pt.setX(ptCenter.x() * 2 - pt.x());
				}
				else
				{
					pt.setY(ptCenter.y() * 2 - pt.y());
				}
				new_positions.push_back(pt);
			});

		return std::move(new_positions);
	}

	CCmdFilpNodes::CCmdFilpNodes(
		qapp::SCommandCreationContext& ctx, 
		CGraphModel& data_model, 
		const bitvec& node_mask, 
		EOrientation orientation)
		: CCmdMoveNodes(ctx, data_model, node_mask, FlippedPositions(data_model, node_mask, orientation))
	{
	}
}