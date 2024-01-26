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

#include "JustifiedGraph.h"

namespace jass
{
	void GenerateJustifiedGraph(
		const CGraphModel& graph_model, 
		const bitvec& node_mask, 
		const std::span<const float>& depths, 
		const std::span<QPointF>& out_points)
	{
		const float MAX_DEPTH_ALLOWED = 255;
		int n = 0;
		int max_depth = 0;
		for (size_t i = 0; i < out_points.size(); ++i)
		{
			if (node_mask.get(i) && !std::isnan(depths[i]) && depths[i] >= 0 && depths[i] <= MAX_DEPTH_ALLOWED)
			{
				const auto rounded_depth = std::round(depths[i]);
				max_depth = std::max(max_depth, (int)rounded_depth);
				out_points[i] = QPoint(0, rounded_depth);
			}
			else
			{
				out_points[i] = QPoint(0, -1);
			}
		}

		int max_row_count = 0;

		std::vector<int> per_row_count(max_depth + 1);
		for (const auto& pt : out_points)
		{
			if (pt.y() < 0)
			{
				continue;
			}
			max_row_count = std::max(max_row_count, ++per_row_count[(int)pt.y()]);
		}

		const float xoffset = 1.0f + .5f * max_row_count;

		for (auto& pt : out_points)
		{
			if (pt.y() < 0)
			{
				continue;
			}
			auto& row_val = per_row_count[(int)pt.y()];

			pt.setX(xoffset - .5f * row_val);
			row_val -= 2.0f;
			pt.setY((float)(max_depth + 1) - pt.y());
		}

		for (auto& pt : out_points)
		{
			if (pt.y() >= 0)
			{
				pt = pt * 50;
			}
		}
	}
}