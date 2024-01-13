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


#include "java/JavaObjectSerializationStream.h"

#include "Debug.h"
#include "endian.h"
#include "GraphModel.hpp"
#include "JassDocument.hpp"
#include "LegacyJassFileFormat.h"

namespace jass
{
	class JavaAwtGeomEllipsed2D_Double : public CJavaObject
	{
	public:
		double width;
		double height;
		double x;
		double y;

		class Factory : public jass::CJavaObjectFactoryT<JavaAwtGeomEllipsed2D_Double>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				auto& self = (JavaAwtGeomEllipsed2D_Double&)obj;
				self.width = in.readField<double>();
				self.height = in.readField<double>();
				self.x = in.readField<double>();
				self.y = in.readField<double>();
			}
		};
	};

	class JavaAwtGeomLine2D_Double : public CJavaObject
	{
	public:
		double x1;
		double y1;
		double x2;
		double y2;

		class Factory : public jass::CJavaObjectFactoryT<JavaAwtGeomLine2D_Double>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				auto& self = (JavaAwtGeomLine2D_Double&)obj;
				self.x1 = in.readField<double>();
				self.y1 = in.readField<double>();
				self.x2 = in.readField<double>();
				self.y2 = in.readField<double>();
			}
		};
	};

	class JavaAwtRectangle: public CJavaObject
	{
	public:
		int width;
		int height;
		int x;
		int y;

		class Factory : public jass::CJavaObjectFactoryT<JavaAwtRectangle>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				auto& self = (JavaAwtRectangle&)obj;
				self.width = in.readField<int32_t>();
				self.height = in.readField<int32_t>();
				self.x = in.readField<int32_t>();
				self.y = in.readField<int32_t>();
			}
		};
	};

	class JavaAwtColor: public CJavaObject
	{
	public:
		uint32_t value;

		class Factory : public jass::CJavaObjectFactoryT<JavaAwtColor>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				auto& self = (JavaAwtColor&)obj;
				in.readField<float>(); // alpha
				self.value = in.readField<uint32_t>();
				in.readObject();  // color space
				in.skipArray();  // frgbvalue[]
				in.skipArray();  // fvalue[]
			}
		};
	};


	class LinkedList : public CJavaObject
	{
	public:
		std::vector<CJavaObject*> Objects;

		class Factory : public jass::CJavaObjectFactoryT<LinkedList>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				auto& self = (LinkedList&)obj;
				const auto count = in.read<uint32_t>();
				self.Objects.reserve(count);
				for (size_t i = 0; i < count; ++i)
				{
					self.Objects.push_back(in.readObject());
				}
			}
		};
	};

	class Category : public CJavaObject
	{
	public:
		JavaAwtColor* color;
		const char* name;
		int index;

		class Factory : public jass::CJavaObjectFactoryT<Category>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				auto& self = (Category&)obj;
				self.color = in.readObjectT<JavaAwtColor>();
				self.name = in.readString().c_str();
				self.index = m_Count++;
			}

		private:
			int m_Count = 0;
		};
	};

	class JavaAwtPoint : public CJavaObject
	{
	public:
		int x;
		int y;

		class Factory : public jass::CJavaObjectFactoryT<JavaAwtPoint>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				auto& self = (JavaAwtPoint&)obj;
				self.x = in.readField<int32_t>();
				self.y = in.readField<int32_t>();
			}
		};
	};

	class Node : public JavaAwtGeomEllipsed2D_Double
	{
	public:
		bool selected;
		bool visited;
		double zoomFactor;
		LinkedList* analysisValues;
		Category* category;
		LinkedList* edges;
		const char* name;
		JavaAwtPoint* point;
		int index;

		class Factory : public jass::CJavaObjectFactoryT<Node>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				m_ClassDesc->SuperClass->ObjectFactory->Load(obj, in);
				auto& self = (Node&)obj;
				self.selected = in.readField<bool>();
				self.visited = in.readField<bool>();
				self.zoomFactor = in.readField<double>();
				// 73 71 00 7E 00 08 77 04 00 00 00 00 78 70 73 71 00 7E 00 08 77 04 00 00 00 00 78 74 00 1C 30 31 
				self.analysisValues = in.readObjectT<LinkedList>();
				self.category = in.readObjectT<Category>();
				self.edges = in.readObjectT<LinkedList>();
				self.name = in.readString().c_str();
				self.point = in.readObjectT<JavaAwtPoint>();
				self.index = m_Count++;
			}

		private:
			int m_Count = 0;
		};
	};

	class Edge : public JavaAwtGeomLine2D_Double
	{
	public:
		int weight;
		Node* node1;
		Node* node2;

		class Factory : public jass::CJavaObjectFactoryT<Edge>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				m_ClassDesc->SuperClass->ObjectFactory->Load(obj, in);
				auto& self = (Edge&)obj;
				self.weight = in.readField<int>();
				self.node1 = in.readObjectT<Node>();
				self.node2 = in.readObjectT<Node>();
			}
		};
	};

	class Graph : public CJavaObject
	{
	public:
		bool directed;
		LinkedList* nodes;
		Node* root;
		Node* rootToBe;

		class Factory : public jass::CJavaObjectFactoryT<Graph>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				auto& self = (Graph&)obj;
				self.directed = in.readField<bool>();
				self.nodes = in.readObjectT<LinkedList>();
				self.root = in.readObjectT<Node>();
				self.rootToBe = in.readObjectT<Node>();
			}
		};
	};

	class ReadWriteGraph: public Graph
	{
	public:
		LinkedList* selectedEdges;

		class Factory : public jass::CJavaObjectFactoryT<ReadWriteGraph>
		{
		public:
			void Load(CJavaObject& obj, CJavaObjectSerializationStream& in)
			{
				m_ClassDesc->SuperClass->ObjectFactory->Load(obj, in);
				auto& self = (ReadWriteGraph&)obj;
				self.selectedEdges = in.readObjectT<LinkedList>();
			}
		};
	};

	bool IsLegacyJassFile(const std::span<const uint8_t>& initial_bytes)
	{
		uint8_t EXPECTED_BYTES[] = { 0xAC, 0xED, 0x00, 0x05 };

		return initial_bytes.size() >= sizeof(EXPECTED_BYTES) && memcmp(initial_bytes.data(), EXPECTED_BYTES, sizeof(EXPECTED_BYTES)) == 0;
	}

	void LoadLegacyJassFile(QIODevice& in, CJassDocument& out_document)
	{
		jass::CJavaObjectSerializationStream jstream(in);

		jstream.AddObjectFactory("java.util.LinkedList", std::make_unique<jass::LinkedList::Factory>());
		jstream.AddObjectFactory("java.awt.Point", std::make_unique<jass::JavaAwtPoint::Factory>());
		jstream.AddObjectFactory("java.awt.Rectangle", std::make_unique<jass::JavaAwtRectangle::Factory>());
		jstream.AddObjectFactory("java.awt.geom.Ellipse2D$Double", std::make_unique<jass::JavaAwtGeomEllipsed2D_Double::Factory>());
		jstream.AddObjectFactory("java.awt.geom.Line2D$Double", std::make_unique<jass::JavaAwtGeomLine2D_Double::Factory>());
		jstream.AddObjectFactory("java.awt.Color", std::make_unique<jass::JavaAwtColor::Factory>());
		jstream.AddObjectFactory("Graph", std::make_unique<jass::Graph::Factory>());
		jstream.AddObjectFactory("ReadWriteGraph", std::make_unique<jass::ReadWriteGraph::Factory>());
		jstream.AddObjectFactory("Node", std::make_unique<jass::Node::Factory>());
		jstream.AddObjectFactory("Edge", std::make_unique<jass::Edge::Factory>());
		jstream.AddObjectFactory("Category", std::make_unique<jass::Category::Factory>());

		auto s = jstream.readString();
		uint32_t maybe_version = jstream.read<uint32_t>();
		auto* rect = jstream.readObjectT<jass::JavaAwtRectangle>();
		auto* graph = jstream.readObjectT<jass::ReadWriteGraph>();

		CGraphModel& graphModel = out_document.GraphModel();
		graphModel.AddNodes(graph->nodes->Objects.size());

		std::vector<Category*> categories;

		{
			graphModel.BeginModifyNodes();
			size_t edge_count = 0;
			for (auto* obj : graph->nodes->Objects)
			{
				auto* node = (Node*)obj;

				graphModel.SetNodeName(node->index, node->name);
				graphModel.SetNodePosition(node->index, CGraphModel::position_t(node->point->x, node->point->y));
				if (node->category)
				{
					graphModel.SetNodeCategory(node->index, node->category->index);
				}

				edge_count += node->edges->Objects.size();
			}
			ASSERT(!(edge_count & 1));  
			edge_count /= 2;  // Every edge was counted twice
			graphModel.EndModifyNodes();

			std::vector<SEdgeDesc> edges;
			edges.reserve(edge_count);
			for (auto* node_obj : graph->nodes->Objects)
			{
				auto* node = (Node*)node_obj;
				for (auto* edge_obj : node->edges->Objects)
				{
					auto* edge = (Edge*)edge_obj;
					if (edge->node1->index < edge->node2->index)
					{
						SEdgeDesc edge_desc;
						edge_desc.Index = (uint32_t)edges.size();
						edge_desc.Node0 = (CGraphModel::node_index_t)edge->node1->index;
						edge_desc.Node1 = (CGraphModel::node_index_t)edge->node2->index;
						edges.push_back(edge_desc);
					}
				}
			}
			graphModel.InsertEdges(edges);
		}
	}
}