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

		jass_editor->SelectionModel().BeginModify();

		if (qapp::EOperation_Do == op)
		{
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
			jass_editor->SelectionModel().DeselectAll();
			for (const auto& node : insert_nodes)
			{
				jass_editor->SelectionModel().SelectNode(node.Index);
			}
		}
		else
		{
			std::vector<CGraphModel::node_index_t> node_indices;
			node_indices.reserve(count);
			qapp::for_each_in_stream<SGraphOpNode>(in, count, [&](const SGraphOpNode& node)
				{
					node_indices.push_back(node.Index);
				});
			jass_editor->DataModel().RemoveNodes(node_indices);
		}

		jass_editor->SelectionModel().EndModify();
	}

	void DeleteGraphNodesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&editor);
		ASSERT(jass_editor);
		auto& data_model = jass_editor->DataModel();

		// Read node indices
		const auto node_count = qapp::tread<CGraphModel::node_index_t>(in);
		std::vector<CGraphModel::node_index_t> node_indices(node_count);
		in.read((char*)node_indices.data(), node_indices.size() * sizeof(CGraphModel::node_index_t));

		jass_editor->SelectionModel().BeginModify();

		if (qapp::EOperation_Do == op)
		{
			// Remove nodes
			data_model.RemoveNodes(node_indices);
		}
		else
		{
			{
				std::vector<SNodeDesc> node_descs(node_count);
				size_t n = 0;
				qapp::for_each_in_stream<CGraphModel::category_index_t>(in, node_count, [&](auto category)
					{
						node_descs[n].Index = node_indices[n];
						node_descs[n].Category = category;
						++n;
					});
				n = 0;
				qapp::for_each_in_stream<QPointF>(in, node_count, [&](const auto& pos)
					{
						node_descs[n++].PositionF = pos;
					});
				data_model.InsertNodes(node_descs);
			}
			
			data_model.BeginModifyNodes();
			for (size_t attribute_index = 0; attribute_index < data_model.NodeAttributeCount(); ++attribute_index)
			{
				auto& node_attribute = data_model.NodeAttribute(attribute_index);
				switch (node_attribute.Type())
				{
				case QVariant::PointF:
					for (const auto node_index : node_indices)
					{
						node_attribute.SetValue<QPointF>(node_index, qapp::tread<QPointF>(in));
					}
					break;
				default:
					throw std::runtime_error("Unsupported node attribute type");
				}
			}
			data_model.EndModifyNodes();

			// Select added nodes
			jass_editor->SelectionModel().DeselectAll();
			for (const auto& node_index : node_indices)
			{
				jass_editor->SelectionModel().SelectNode(node_index);
			}
		}

		jass_editor->SelectionModel().EndModify();
	}

	void WriteDeleteGraphNodesOp(std::ostream& out, const CGraphModel& graph_model, const std::span<const CGraphModel::node_index_t>& node_indices)
	{
		WriteOperation(out, DeleteGraphNodesProcessor, [&](std::ostream& out)
			{
				qapp::twrite(out, (CGraphModel::node_index_t)node_indices.size());
				out.write((char*)node_indices.data(), node_indices.size() * sizeof(CGraphModel::node_index_t));
				for (const auto node_index : node_indices)
				{
					qapp::twrite(out, graph_model.NodeCategory(node_index));
				}
				for (const auto node_index : node_indices)
				{
					qapp::twrite(out, graph_model.NodePosition(node_index));
				}
				for (size_t attribute_index = 0; attribute_index < graph_model.NodeAttributeCount(); ++attribute_index)
				{
					const auto& node_attribute = graph_model.NodeAttribute(attribute_index);
					switch (node_attribute.Type())
					{
					case QVariant::PointF:
						for (const auto node_index : node_indices)
						{
							qapp::twrite(out, node_attribute.Value<QPointF>(node_index));
						}
						break;
					default:
						throw std::runtime_error("Unsupported node attribute type");
					}
				}
			});
	}

	void InsertGraphEdgesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size)
	{
		auto* jass_editor = dynamic_cast<CJassEditor*>(&editor);
		ASSERT(jass_editor);

		const auto count = size / sizeof(SEdgeDesc);

		jass_editor->SelectionModel().BeginModify();

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

		jass_editor->SelectionModel().EndModify();
	}

	void DeleteGraphEdgesProcessor(qapp::IEditor& editor, qapp::EOperation op, std::istream& in, std::streamsize size)
	{
		InsertGraphEdgesProcessor(editor, qapp::EOperation_Do == op ? qapp::EOperation_Undo : qapp::EOperation_Do, in, size);
	}
}