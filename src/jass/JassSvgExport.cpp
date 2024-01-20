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

#include <QtCore/qiodevice.h>
#include <QtGui/qrgb.h>
#include <jass/JassDocument.hpp>
#include <jass/Shape.h>
#include "JassSvgExport.h"

namespace jass
{
	static void qio_fwrite(QIODevice& out, const char* format, ...);

	extern const char* DROP_SHADOW_FILTER;

	size_t CreateSymbol(QIODevice& out, const std::string_view& row_prefix, const char* name, EShape shape, QRgb color, float radius, float scale, float line_width);

	void ExportJassToSVG(QIODevice& out, const CJassDocument& doc)
	{
		const float SYMBOL_SCALE = 8;
		const float SYMBOL_LINE_WIDTH = 3;
		const float SYMBOL_PADDING = 5;  // for shadow
		const float SYMBOL_RADIUS = SYMBOL_SCALE + SYMBOL_LINE_WIDTH * .5f + SYMBOL_PADDING;
		const float IMAGE_PADDING = 10;

		const auto& data_model = doc.GraphModel();

		// Bounding box
		QRectF bb(0, 0, 0, 0);
		if (data_model.NodeCount())
		{
			bb = QRectF(data_model.NodePosition(0), data_model.NodePosition(0));
			for (CGraphModel::node_index_t node_index = 0; node_index < data_model.NodeCount(); ++node_index)
			{
				const auto pos = data_model.NodePosition(node_index);
				bb.setLeft(std::min(pos.x(), bb.left()));
				bb.setTop(std::min(pos.y(), bb.top()));
				bb.setRight(std::max(pos.x(), bb.right()));
				bb.setBottom(std::max(pos.y(), bb.bottom()));
			}
		}

		const QPointF coordinate_offset(IMAGE_PADDING + SYMBOL_RADIUS - bb.left(), IMAGE_PADDING + SYMBOL_RADIUS - bb.top());

		// Determine image size based on bounding box and padding
		const int width = (int)std::ceil(bb.width() + (IMAGE_PADDING + SYMBOL_RADIUS) * 2);
		const int height = (int)std::ceil(bb.height() + (IMAGE_PADDING + SYMBOL_RADIUS) * 2);

		qio_fwrite(out, "<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\"  xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n", width, height);

		out.write("\t<defs>\n");
		out.write("\t\t<style>\n");
		out.write("\t\t\t.edge {\n");
		out.write("\t\t\t\tstroke: black;\n");
		out.write("\t\t\t\tstroke-width: 2;\n");
		out.write("\t\t\t}\n");
		out.write("\t\t</style>\n");
		out.write(DROP_SHADOW_FILTER);
		out.write("\t</defs>\n");

		out.write("\n\t<!-- Symbol Definitions -->\n");
		auto& categories = doc.Categories();
		for (size_t i = 0; i < categories.Size(); ++i)
		{
			char name[16];
			snprintf(name, sizeof(name), "cat-%d", (int)i);
			CreateSymbol(out, "\t", name, categories.Shape(i), categories.Color(i), SYMBOL_RADIUS, SYMBOL_SCALE, SYMBOL_LINE_WIDTH);
		}

		// Edges
		out.write("\n\t<!-- Edges -->\n");
		for (CGraphModel::edge_index_t edge_index = 0; edge_index < data_model.EdgeCount(); ++edge_index)
		{
			const auto node_pair = data_model.EdgeNodePair(edge_index);
			const auto p0 = data_model.NodePosition(node_pair.first) + coordinate_offset;
			const auto p1 = data_model.NodePosition(node_pair.second) + coordinate_offset;
			qio_fwrite(out, "\t<line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" class=\"edge\" />\n", p0.x(), p0.y(), p1.x(), p1.y());
		}

		// Nodes
		out.write("\n\t<!-- Nodes -->\n");
		for (CGraphModel::node_index_t node_index = 0; node_index < data_model.NodeCount(); ++node_index)
		{
			const auto pos = data_model.NodePosition(node_index) + coordinate_offset;
			const auto category = (uint32_t)data_model.NodeCategory(node_index);
			char name[16];
			snprintf(name, sizeof(name), "cat-%d", category >= categories.Size() ? 0 : category);
			qio_fwrite(out, "\t<use xlink:href=\"#%s\" x=\"%.1f\" y=\"%.1f\" />\n", name, pos.x() - SYMBOL_RADIUS, pos.y() - SYMBOL_RADIUS);
		}

		out.write("</svg>");
	}

	static void qio_fwrite(QIODevice& out, const char* format, ...)
	{
		char buf[8192];

		va_list args;
		va_start(args, format);

#ifdef _MSC_VER
		const int size = vsnprintf_s(buf, sizeof(buf), sizeof(buf), format, args);
#else
		const int size = vsnprintf(buf, sizeof(buf), format, args);
#endif
		if (size >= sizeof(buf))
		{
			throw std::runtime_error("Buffer too small!");
		}

		if (size != (int)out.write(buf, size))
		{
			throw std::runtime_error("I/O error!");
		}
	}

	size_t CreateSymbol(QIODevice& out, const std::string_view& row_prefix, const char* name, EShape shape, QRgb color, float radius, float scale, float line_width)
	{
		out.write(row_prefix.data(), row_prefix.size());
		qio_fwrite(out, "<symbol id=\"%s\" width=\"%f\" height=\"%f\">\n", name, radius * 2, radius * 2);

		if (shape == EShape::Circle)
		{
			out.write(row_prefix.data(), row_prefix.size());
			qio_fwrite(out, "\t<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"#%.2x%.2x%.2x\" stroke=\"white\" stroke-width=\"%f\" filter=\"url(#drop-shadow)\"/>\n",
				radius, radius, scale * CIRCLE_SHAPE_SCALE,
				qRed(color), qGreen(color), qBlue(color), line_width);
		}
		else
		{
			const auto pts = GetShapePoints(shape);
			out.write(row_prefix.data(), row_prefix.size());
			out.write("\t<polygon points=\"");
			for (size_t i = 0; i < pts.size(); ++i)
			{
				qio_fwrite(out, i ? " %f,%f" : "%f,%f", radius + pts[i].x() * scale, radius + pts[i].y() * scale);
			}
			qio_fwrite(out, "\" fill=\"#%.2x%.2x%.2x\" stroke=\"white\" stroke-width=\"%f\" stroke-linejoin=\"round\" filter=\"url(#drop-shadow)\"/>\n", 
				qRed(color), qGreen(color), qBlue(color), line_width);
		}

		out.write(row_prefix.data(), row_prefix.size());
		out.write("</symbol>\n");

		return 0;
	}

	const char* DROP_SHADOW_FILTER = 
R"(		<filter id="drop-shadow" x="-50%" y="-50%" width="200%" height="200%">
			<feDropShadow dx="1" dy="1" stdDeviation="1.5" />
		</filter>
)";

}