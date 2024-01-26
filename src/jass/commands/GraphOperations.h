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

#include <qapplib/commands/Operations.h>
#include <qapplib/utils/StreamUtils.h>
#include <jass/GraphModel.hpp>

namespace jass
{
	struct SGraphOpNode
	{
		CGraphModel::node_index_t Index;
		CGraphModel::category_index_t Category;
		float X;
		float Y;
		//bool Select;
	};


	// Insert graph nodes

	void InsertGraphNodesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size);

	template <class TLambda>
	void WriteInsertGraphNodesOp(std::ostream& out, TLambda&& lambda)
	{
		WriteOperation(out, InsertGraphNodesProcessor, [&](std::ostream& out)
			{
				lambda([&](CGraphModel::node_index_t index, CGraphModel::category_index_t category, float x, float y)
					{
						qapp::twrite(out, SGraphOpNode({ index, category, x, y }));
					});
			});
	}


	// Delete graph nodes

	void DeleteGraphNodesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size);

	void WriteDeleteGraphNodesOp(std::ostream& out, const CGraphModel& graph_model, const std::span<const CGraphModel::node_index_t>& node_indices);


	// Insert graph edges

	void InsertGraphEdgesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size);

	template <class TLambda>
	void WriteInsertGraphEdgesOp(std::ostream& out, TLambda&& lambda)
	{
		WriteOperation(out, InsertGraphEdgesProcessor, [&](std::ostream& out)
			{
				lambda([&](const SEdgeDesc& edge)
					{
						qapp::twrite(out, edge);
					});
			});
	}


	// Delete graph edges

	void DeleteGraphEdgesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size);

	template <class TLambda>
	void WriteDeleteGraphEdgesOp(std::ostream& out, TLambda&& lambda)
	{
		WriteOperation(out, DeleteGraphEdgesProcessor, [&](std::ostream& out)
			{
				lambda([&](const SEdgeDesc& edge)
					{
						qapp::twrite(out, edge);
					});
			});
	}
}