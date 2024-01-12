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

#include <QtCore/qpoint.h>
#include <QtCore/qjsonarray.h>
#include <jass/utils/range_utils.h>

#include "JsonGraphData.h"
#include "GraphView.h"
#include "GraphBuilder.h"

namespace jass
{
	QJsonArray ToJsonNodeAttribute(const IGraphView& graph_view, size_t node_attribute_index, const SNodeAttributeDesc& desc)
	{
		QJsonArray arr;
		switch (desc.Type)
		{
		case QVariant::UInt:
			{
				std::vector<uint32_t> data(graph_view.NodeCount());
				graph_view.GetNodeAttributeData(node_attribute_index, to_span(data));
				for (auto value : data)
				{
					arr.append((int)value);
				}
			}
			break;
		case QVariant::PointF:
			{
				std::vector<QPointF> data(graph_view.NodeCount());
				graph_view.GetNodeAttributeData(node_attribute_index, to_span(data));
				for (const auto& value : data)
				{
					arr.append(std::round(value.x() * 10) * .1);
					arr.append(std::round(value.y() * 10) * .1);
				}
			}
			break;
		default:
			throw std::runtime_error("Unsupported node attribute data type");
		}
		return arr;
	}

	QJsonObject ToJson(const IGraphView& graph_view)
	{
		QJsonObject root;

		root["node_count"] = (int)graph_view.NodeCount();

		{
			QJsonArray node_attributes;
			for (size_t node_attribute_index = 0; node_attribute_index < graph_view.NodeAttributeCount(); ++node_attribute_index)
			{
				const auto desc = graph_view.NodeAttributeDesc(node_attribute_index);
				QJsonObject nodeAttribute;
				nodeAttribute["name"] = desc.Name;
				nodeAttribute["type"] = qapp::QVariantTypeToString(desc.Type);
				nodeAttribute["data"] = ToJsonNodeAttribute(graph_view, node_attribute_index, desc);
				node_attributes.append(nodeAttribute);
			}
			root["node_attributes"] = node_attributes;
		}

		{
			std::vector<IGraphView::edge_t> edges(graph_view.EdgeCount());
			graph_view.GetEdges(edges);
			QJsonArray edgesArray;
			for (const auto& edge : edges)
			{
				edgesArray.append((int)edge.first);
				edgesArray.append((int)edge.second);
			}
			root["edges"] = edgesArray;
		}

		return root;
	}

	template <typename T> inline QJsonValue::Type QJsonValueType();
	template <>	inline QJsonValue::Type QJsonValueType<int>() { return QJsonValue::Double; }
	template <>	inline QJsonValue::Type QJsonValueType<QString>() { return QJsonValue::String; }
	template <>	inline QJsonValue::Type QJsonValueType<QJsonObject>() { return QJsonValue::Object; }
	template <>	inline QJsonValue::Type QJsonValueType<QJsonArray>() { return QJsonValue::Array; }

	template <typename T> inline T JsonValue(const QJsonValue& jsonValue);
	template <>	inline int         JsonValue<int>(const QJsonValue& jsonValue) { return jsonValue.toInt(); }
	template <>	inline QString     JsonValue<QString>(const QJsonValue& jsonValue) { return jsonValue.toString(); }
	template <>	inline QJsonObject JsonValue<QJsonObject>(const QJsonValue& jsonValue) { return jsonValue.toObject(); }
	template <>	inline QJsonArray  JsonValue<QJsonArray>(const QJsonValue& jsonValue) { return jsonValue.toArray(); }

	template <typename T>
	inline bool TryGetJsonValue(const QJsonObject& obj, const QString& key, T& out_value)
	{
		auto it = obj.find(key);
		if (it == obj.end() || QJsonValueType<T>() != it->type())
		{
			return false;
		}
		out_value = JsonValue<T>(*it);
		return true;
	}

	template <typename T>
	inline T GetRequiredJsonValue(const QJsonObject& obj, const QString& key)
	{
		T value;
		if (!TryGetJsonValue(obj, key, value))
		{
			throw std::runtime_error(QString("Missing field \"%1\"").arg(key).toStdString().c_str());
		}
		return value;
	}

	void GraphFromJson(IGraphBuilder& gbuilder, const QJsonObject& root)
	{
		const size_t node_count = GetRequiredJsonValue<int>(root, "node_count");
		gbuilder.SetNodeCount(node_count);
		
		for (auto it : GetRequiredJsonValue<QJsonArray>(root, "node_attributes"))
		{
			if (!it.isObject())
			{
				throw std::runtime_error("Node attribute object expected");
			}
			auto node_attribute_obj = it.toObject();


			SNodeAttributeDesc desc;
			desc.Name = GetRequiredJsonValue<QString>(node_attribute_obj, "name");
			desc.Type = qapp::QVariantTypeFromString(GetRequiredJsonValue<QString>(node_attribute_obj, "type"));
			auto dataArray = GetRequiredJsonValue<QJsonArray>(node_attribute_obj, "data");

			auto verify_node_attribute_array_size = [&](size_t size)
			{
				if ((size_t)dataArray.size() != size)
				{
					throw std::runtime_error(QString("Expected node attribute \"%1\" to be of size \"%2\".").arg(desc.Name).arg((int)size).toStdString().c_str());
				}
			};

			switch (desc.Type)
			{
			case QVariant::UInt:
				{
					verify_node_attribute_array_size(node_count);
					std::vector<uint32_t> data(node_count);
					for (int i = 0; i < (int)node_count; ++i)
					{
						data[i] = dataArray[i].toInt();
					}
					gbuilder.AddNodeAttribute(desc, data.data(), data.size() * sizeof(uint32_t));
				}
				break;
			case QVariant::PointF:
				{
					verify_node_attribute_array_size(node_count * 2);
					std::vector<QPointF> data(node_count);
					for (int i = 0; i < (int)node_count; ++i)
					{
						data[i] = QPointF(dataArray[i * 2].toDouble(), dataArray[i * 2 + 1].toDouble());
					}
					gbuilder.AddNodeAttribute(desc, data.data(), data.size() * sizeof(QPointF));
				}
				break;
			default:
				throw std::runtime_error("Unsupported node attribute data type");
			}
		}

		{
			auto edgesArray = GetRequiredJsonValue<QJsonArray>(root, "edges");
			std::vector<IGraphBuilder::edge_t> edges(edgesArray.size() / 2);
			for (int i = 0; i < (int)edges.size(); ++i)
			{
				edges[i] = std::make_pair((uint32_t)edgesArray[i * 2].toInt(), (uint32_t)edgesArray[i * 2 + 1].toInt());
			}
			gbuilder.SetEdges(edges);
		}
	}

	//template <class TGraphDataView>
	//void graph_data_to_json(const TGraphDataView& gview, std::ostream& out)
	//{
	//	char buf[16];
	//	auto write_integer = [&](int value)   -> const char* { snprintf(buf, sizeof(buf), "%d", value); return buf; };
	//	auto write_float = [&](float value) -> const char* { snprintf(buf, sizeof(buf), "%.2f", value); return buf; };

	//	out << "{";
	//	out << "\n\t\"node_count\" : " << write_integer((int)gview.node_count());
	//	out << "\n\t\"edges\" : [";
	//	auto edges = gview.edges();
	//	for (const auto& edge : gview.edges())
	//	{
	//		if (&edge != edges.data())
	//		{
	//			out << ",";
	//		}
	//		out << "\n\t\t" << write_integer((int)edge.first) << ", " << write_integer((int)edge.second);
	//	}
	//	out << "\n\t]";
	//	out << "\n\t\"node_attributes\" : {";
	//	auto descs = gview.node_attribute_descs();
	//	for (const auto& desc : descs)
	//	{
	//		if (&desc != descs.data())
	//		{
	//			out << ",";
	//		}
	//		out << "\n\t\t\"" << desc.Name << "\" : [";
	//		switch (desc.DataType)
	//		{
	//		case graph_data::EDataType_UInt32:
	//		{
	//			const auto values = gview.node_attributes<uint32_t>(&desc - descs.data());
	//			for (const auto& value : values)
	//			{
	//				if (&value != values.data())
	//				{
	//					out << ", ";
	//				}
	//				out << write_integer(value);
	//			}
	//		}
	//		break;
	//		case graph_data::EDataType_QPointF:
	//		{
	//			const auto points = gview.node_attributes<QPointF>(&desc - descs.data());
	//			for (const auto& pt : points)
	//			{
	//				if (&pt != points.data())
	//				{
	//					out << ", ";
	//				}
	//				out << write_float(pt.x()) << ", " << write_float(pt.y());
	//			}
	//		}
	//		break;
	//		}
	//		out << "]";
	//	}
	//	out << "\n\t}";

	//	out << "\n}";
	//}
}
