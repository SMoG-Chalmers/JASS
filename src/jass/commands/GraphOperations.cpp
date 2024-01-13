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

#include <jass/Debug.h>
#include <jass/GraphEditor/JassEditor.hpp>

#include "GraphOperations.h"

namespace jass
{
	void InsertGraphNodesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&editor);
		ASSERT(jass_editor);

		const auto count = size / sizeof(SGraphOpNode);

		if (qapp::EOperation_Do == op)
		{
			// Deselect all
			jass_editor->SelectionModel().BeginModify();
			jass_editor->SelectionModel().DeselectAll();
			jass_editor->SelectionModel().EndModify();

			std::vector<SNodeDesc> insert_nodes;
			insert_nodes.reserve(count);
			qapp::for_each_in_stream<SGraphOpNode>(in, count, [&](const SGraphOpNode& node)
				{
					SNodeDesc node_desc;
					node_desc.Index = node.Index;
					node_desc.Category = node.Category;
					node_desc.PositionF = QPointF(node.X, node.Y);
					insert_nodes.push_back(node_desc);
				});
			jass_editor->DataModel().InsertNodes(insert_nodes);

			// Select added nodes
			jass_editor->SelectionModel().BeginModify();
			jass_editor->SelectionModel().DeselectAllNodes();
			for (const auto& node : insert_nodes)
			{
				jass_editor->SelectionModel().SelectNode(node.Index);
			}
			jass_editor->SelectionModel().EndModify();
		}
		else
		{
			// Deselect all
			jass_editor->SelectionModel().BeginModify();
			jass_editor->SelectionModel().DeselectAll();
			jass_editor->SelectionModel().EndModify();

			std::vector<CGraphModel::node_index_t> node_indices;
			node_indices.reserve(count);
			qapp::for_each_in_stream<SGraphOpNode>(in, count, [&](const SGraphOpNode& node)
				{
					node_indices.push_back(node.Index);
				});
			
			jass_editor->DataModel().RemoveNodes(node_indices);
		}
	}

	void DeleteGraphNodesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size)
	{
		InsertGraphNodesProcessor(editor, qapp::EOperation_Do == op ? qapp::EOperation_Undo : qapp::EOperation_Do, in, size);
	}

	void InsertGraphEdgesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&editor);
		ASSERT(jass_editor);

		const auto count = size / sizeof(SEdgeDesc);

		if (qapp::EOperation_Do == op)
		{
			std::vector<SEdgeDesc> edges;
			edges.reserve(count);
			qapp::for_each_in_stream<SEdgeDesc>(in, count, [&](const auto& edge)
				{
					edges.push_back(edge);
				});
			jass_editor->DataModel().InsertEdges(edges);
		}
		else
		{
			std::vector<CGraphModel::edge_index_t> edge_indices;
			edge_indices.reserve(count);
			qapp::for_each_in_stream<SEdgeDesc>(in, count, [&](const auto& edge)
				{
					edge_indices.push_back(edge.Index);
				});
			jass_editor->DataModel().RemoveEdges(edge_indices);
		}
	}

	void DeleteGraphEdgesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size)
	{
		InsertGraphEdgesProcessor(editor, qapp::EOperation_Do == op ? qapp::EOperation_Undo : qapp::EOperation_Do, in, size);
	}
}