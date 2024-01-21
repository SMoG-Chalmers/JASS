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

#include <QtCore/QByteArray>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>

#include <zipxlib/zip_reader.h>
#include <zipxlib/zip_writer.h>

#include <qapplib/io/QIODeviceStreamAdapter.h>
#include <qapplib/utils/StreamUtils.h>
#include <qapplib/utils/StringUtils.h>
#include <qapplib/Color.h>

#include <jass/graphdata/JsonGraphData.h>
#include <jass/graphdata/GraphModelGraphBuilder.h>
#include <jass/graphdata/GraphModelGraphView.h>
#include <jass/GraphEditor/CategorySet.hpp>
#include <jass/utils/JsonUtils.h>

#include "Debug.h"
#include "GraphModel.hpp"
#include "JassDocument.hpp"
#include "JassFileFormat.h"

#pragma comment (lib, "zlib" XMN_CONF ".lib")

namespace jass
{
	const QString GRAPH_KEY = "graph";
	const QString BACKGROUND_IMAGE_KEY = "background_image";
	const QString CATEGORIES_KEY = "categories";

	bool IsJassFile(const std::span<const uint8_t>& initial_bytes)
	{
		uint8_t EXPECTED_BYTES[] = { 0x50, 0x4B, 0x03, 0x04 };

		return initial_bytes.size() >= sizeof(EXPECTED_BYTES) && memcmp(initial_bytes.data(), EXPECTED_BYTES, sizeof(EXPECTED_BYTES)) == 0;
	}

	void FromJson(const QJsonValue& jsonValue, CCategorySet& out_categories);

	void LoadJassMain(QJsonDocument& json_doc, zipx::zip_reader& reader, CJassDocument& out_jass_doc)
	{
		auto root = json_doc.object();
		
		{
			auto graph = root.find(GRAPH_KEY)->toObject();
			CGraphModelGraphBuilder gbuilder(out_jass_doc.GraphModel());
			GraphFromJson(gbuilder, graph);
		}

		auto background_image_path = root.find(BACKGROUND_IMAGE_KEY)->toString();
		if (!background_image_path.isNull())
		{
			zipx::zip_index::SFileInfo file_info;
			auto image_stream = reader.open(background_image_path.toStdString().c_str(), &file_info);
			if (image_stream)
			{
				QByteArray image_bytes(file_info.m_Size, 0);
				qapp::read_exact(*image_stream.get(), image_bytes.data(), image_bytes.size());
				out_jass_doc.SetImage(std::move(image_bytes), QFileInfo(background_image_path).completeSuffix());
			}
		}

		auto categories = root.find(CATEGORIES_KEY);
		if (categories != root.end())
		{
			FromJson(categories.value(), out_jass_doc.Categories());
		}
	}

	void LoadJassFile(QIODevice& in, CJassDocument& out_jass_doc)
	{
		qapp::QIODeviceIStream in_adapter(in);
		zipx::zip_reader reader(in_adapter);

		// Load Index
		QJsonDocument json_doc;
		{
			zipx::zip_index::SFileInfo file_info;
			auto f = reader.open("jass.json", &file_info);
			QByteArray data(file_info.m_Size, 0);
			qapp::read_exact(*f, data.data(), file_info.m_Size);
			QJsonParseError err;
			json_doc = QJsonDocument::fromJson(data, &err);
			if (json_doc.isNull())
			{
				throw std::runtime_error(err.errorString().toStdString().c_str());
			}
		}
		LoadJassMain(json_doc, reader, out_jass_doc);
	}

	QJsonValue ToJson(const CCategorySet& categories);

	void SaveJassFile(QIODevice& out, const CJassDocument& document)
	{
		qapp::QIODeviceOStream out_adapter(out);
		zipx::zip_writer writer(out_adapter);

		QString backgroundImageFileName;
		if (!document.ImageData().isEmpty())
		{
			backgroundImageFileName = "background." + document.ImageExtensionNoDot();
			auto& out = writer.begin_file(backgroundImageFileName.toStdString().c_str(), true);
			out.write((char*)document.ImageData().data(), document.ImageData().size());
			writer.end_file();
		}

		{
			QJsonObject root;

			root[GRAPH_KEY] = ToJson(CGraphModelGraphView(document.GraphModel()));
			
			if (!backgroundImageFileName.isEmpty())
			{
				root[BACKGROUND_IMAGE_KEY] = backgroundImageFileName;
			}

			root[CATEGORIES_KEY] = ToJson(document.Categories());

			auto jsonUtf8 = QJsonDocument(root).toJson();
			auto& out = writer.begin_file("jass.json", true);
			out.write((char*)jsonUtf8.data(), jsonUtf8.size());
			writer.end_file();
		}
	}

	QJsonValue ToJson(const CCategorySet& categories)
	{
		QJsonArray arr;
		for (size_t category_index = 0; category_index < categories.Size(); ++category_index)
		{
			auto obj = QJsonObject();
			obj["name"]  = categories.Name(category_index);
			const auto color = categories.Color(category_index);
			char buf[8];
			snprintf(buf, sizeof(buf), "#%.2x%.2x%.2x", qRed(color), qGreen(color), qBlue(color));
			obj["color"] = buf;
			obj["shape"] = ShapeToString(categories.Shape(category_index));
			arr.append(obj);
		}
		return arr;
	}

	void FromJson(const QJsonValue& jsonValue, CCategorySet& out_categories)
	{
		if (!jsonValue.isArray())
		{
			throw std::runtime_error("Category array expected");
		}
		for (auto it : jsonValue.toArray())
		{
			if (!it.isObject())
			{
				throw std::runtime_error("Category object expected");
			}
			auto obj = it.toObject();

			const auto name = GetOptionalJsonValue(obj, "name", QString(""));
			EShape shape;
			if (!TryGetShapeFromString(GetOptionalJsonValue(obj, "shape", QString("circle")), shape))
			{
				shape = EShape::Circle;
			}
			QRgb color;
			if (!qapp::TryParseColor(qapp::to_string_view(GetOptionalJsonValue<QString>(obj, "color", QString("#000"))), color))
			{
				color = qRgb(0, 0, 0);
			}
			out_categories.AddCategory(name, color, shape);
		}
	}
}