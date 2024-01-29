/*
Copyright Ioanna Stavroulaki 2023

This file is part of JASS.

JASS is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version.

JASS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along 
with JASS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <QtCore/qiodevice.h>
#include <QtGui/qrgb.h>
#include <jass/JassDocument.hpp>
#include <jass/Shape.h>
#include <jass/StandardNodeAttributes.h>
#include "JassSvgExport.h"

namespace jass
{
	static void qio_fwrite(QIODevice& out, const char* format, ...);

	extern const char* DROP_SHADOW_FILTER;

	void CreateSymbol(QIODevice& out, const std::string_view& row_prefix, const char* name, EShape shape, QRgb color, float radius, float scale, float line_width, float line_width2);
	void CreateShape(QIODevice& out, const std::string_view& row_prefix, const QPointF& pos, EShape shape, QRgb color, float radius, float scale, float line_width, float line_width2);

	void ExportJassToSVG(QIODevice& out, const CJassDocument& doc)
	{
		const float SYMBOL_SCALE = 8;
		const float SYMBOL_LINE_WIDTH = 2;
		const float SYMBOL_LINE_WIDTH2 = SYMBOL_LINE_WIDTH* .5f;
		const float SYMBOL_PADDING = 0;  // 5  // for shadow
		const float SYMBOL_RADIUS = SYMBOL_SCALE + SYMBOL_LINE_WIDTH * .5f + SYMBOL_LINE_WIDTH2 + SYMBOL_PADDING;
		const float IMAGE_MARGIN = 10;
		const float GRAPH_SPACING = 50;  // Spacing between the two graphs

		const auto& data_model = doc.GraphModel();

		auto* justified_position_node_attribute = TryGetJPositionNodeAttribute(data_model);

		// Normal Bounding box
		QRectF bbNormal(0, 0, 0, 0);
		if (data_model.NodeCount())
		{
			bbNormal = QRectF(data_model.NodePosition(0), data_model.NodePosition(0));
			for (CGraphModel::node_index_t node_index = 0; node_index < data_model.NodeCount(); ++node_index)
			{
				const auto pos = data_model.NodePosition(node_index);
				bbNormal.setLeft(std::min(pos.x(), bbNormal.left()));
				bbNormal.setTop(std::min(pos.y(), bbNormal.top()));
				bbNormal.setRight(std::max(pos.x(), bbNormal.right()));
				bbNormal.setBottom(std::max(pos.y(), bbNormal.bottom()));
			}
			if (!bbNormal.isEmpty())
			{
				bbNormal.adjust(-SYMBOL_RADIUS, -SYMBOL_RADIUS, SYMBOL_RADIUS, SYMBOL_RADIUS);
			}
		}

		// Justified Bounding box
		QRectF bbJustified(0, 0, 0, 0);
		if (data_model.NodeCount() && justified_position_node_attribute)
		{
			bbJustified = QRectF(justified_position_node_attribute->Value(0), justified_position_node_attribute->Value(0));
			for (CGraphModel::node_index_t node_index = 0; node_index < data_model.NodeCount(); ++node_index)
			{
				const auto pos = justified_position_node_attribute->Value(node_index);
				bbJustified.setLeft(std::min(pos.x(), bbJustified.left()));
				bbJustified.setTop(std::min(pos.y(), bbJustified.top()));
				bbJustified.setRight(std::max(pos.x(), bbJustified.right()));
				bbJustified.setBottom(std::max(pos.y(), bbJustified.bottom()));
			}
			if (!bbJustified.isEmpty())
			{
				bbJustified.adjust(-SYMBOL_RADIUS, -SYMBOL_RADIUS, SYMBOL_RADIUS, SYMBOL_RADIUS);
			}
		}

		// Determine image size based on bounding box and padding
		int width = (int)std::ceil(IMAGE_MARGIN + bbNormal.width() + IMAGE_MARGIN);
		if (bbJustified.width() > 0)
		{
			width += GRAPH_SPACING + bbJustified.width();
		}
		const int height = (int)std::ceil(IMAGE_MARGIN + std::max(bbNormal.height(), bbJustified.height()) + IMAGE_MARGIN);

		qio_fwrite(out, "<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\"  xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n", width, height);

		out.write("\t<defs>\n");
		out.write("\t\t<style>\n");
		out.write("\t\t\t.edge {\n");
		out.write("\t\t\t\tstroke: black;\n");
		out.write("\t\t\t\tstroke-width: 2;\n");
		out.write("\t\t\t}\n");
		out.write("\t\t</style>\n");
		//out.write(DROP_SHADOW_FILTER);
		out.write("\t</defs>\n");

		const auto& categories = doc.Categories();

		//out.write("\n\t<!-- Symbol Definitions -->\n");
		//for (size_t i = 0; i < categories.Size(); ++i)
		//{
		//	char name[16];
		//	snprintf(name, sizeof(name), "cat-%d", (int)i);
		//	CreateSymbol(out, "\t", name, categories.Shape(i), categories.Color(i), SYMBOL_RADIUS, SYMBOL_SCALE, SYMBOL_LINE_WIDTH);
		//}

		// Normal Graph
		{
			const QPointF line_offset(IMAGE_MARGIN - bbNormal.left(), IMAGE_MARGIN - bbNormal.top());
			const QPointF symbol_offset = line_offset - QPointF(SYMBOL_RADIUS, SYMBOL_RADIUS);

			// Edges
			out.write("\n\t<!-- Edges -->\n");
			for (CGraphModel::edge_index_t edge_index = 0; edge_index < data_model.EdgeCount(); ++edge_index)
			{
				const auto node_pair = data_model.EdgeNodePair(edge_index);
				const auto p0 = data_model.NodePosition(node_pair.first) + line_offset;
				const auto p1 = data_model.NodePosition(node_pair.second) + line_offset;
				qio_fwrite(out, "\t<line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" class=\"edge\" />\n", p0.x(), p0.y(), p1.x(), p1.y());
			}

			// Nodes
			out.write("\n\t<!-- Nodes -->\n");
			for (CGraphModel::node_index_t node_index = 0; node_index < data_model.NodeCount(); ++node_index)
			{
				const auto pos = data_model.NodePosition(node_index) + symbol_offset;
				const auto category = (uint32_t)data_model.NodeCategory(node_index);
				//char name[16];
				//snprintf(name, sizeof(name), "cat-%d", category >= categories.Size() ? 0 : category);
				//qio_fwrite(out, "\t<use xlink:href=\"#%s\" x=\"%.1f\" y=\"%.1f\" />\n", name, pos.x() - SYMBOL_RADIUS, pos.y() - SYMBOL_RADIUS);
				CreateShape(out, "\t", pos, categories.Shape(category), categories.Color(category), SYMBOL_RADIUS, SYMBOL_SCALE, SYMBOL_LINE_WIDTH, SYMBOL_LINE_WIDTH * .5f);
			}
		}

		// Justified Graph
		if (justified_position_node_attribute)
		{
			const QPointF line_offset(IMAGE_MARGIN + bbNormal.width() + GRAPH_SPACING - bbJustified.left(), IMAGE_MARGIN - bbJustified.top());
			const QPointF symbol_offset = line_offset - QPointF(SYMBOL_RADIUS, SYMBOL_RADIUS);

			// Edges
			out.write("\n\t<!-- Edges -->\n");
			for (CGraphModel::edge_index_t edge_index = 0; edge_index < data_model.EdgeCount(); ++edge_index)
			{
				const auto node_pair = data_model.EdgeNodePair(edge_index);
				auto p0 = justified_position_node_attribute->Value(node_pair.first);
				auto p1 = justified_position_node_attribute->Value(node_pair.second);
				if (p0.y() < 0 || p1.y() < 0)
				{
					continue;
				}
				p0 += line_offset;
				p1 += line_offset;
				qio_fwrite(out, "\t<line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" class=\"edge\" />\n", p0.x(), p0.y(), p1.x(), p1.y());
			}

			// Nodes
			out.write("\n\t<!-- Nodes -->\n");
			for (CGraphModel::node_index_t node_index = 0; node_index < data_model.NodeCount(); ++node_index)
			{
				auto pos = justified_position_node_attribute->Value(node_index);
				if (pos.y() < 0)
				{
					continue;
				}
				pos += symbol_offset;
				const auto category = (uint32_t)data_model.NodeCategory(node_index);
				//char name[16];
				//snprintf(name, sizeof(name), "cat-%d", category >= categories.Size() ? 0 : category);
				//qio_fwrite(out, "\t<use xlink:href=\"#%s\" x=\"%.1f\" y=\"%.1f\" />\n", name, pos.x() - SYMBOL_RADIUS, pos.y() - SYMBOL_RADIUS);
				CreateShape(out, "\t", pos, categories.Shape(category), categories.Color(category), SYMBOL_RADIUS, SYMBOL_SCALE, SYMBOL_LINE_WIDTH, SYMBOL_LINE_WIDTH * .5f);
			}
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

	void CreateSymbol(QIODevice& out, const std::string_view& row_prefix, const char* name, EShape shape, QRgb color, float radius, float scale, float line_width, float line_width2)
	{
		out.write(row_prefix.data(), row_prefix.size());
		qio_fwrite(out, "<symbol id=\"%s\" width=\"%f\" height=\"%f\">\n", name, radius * 2, radius * 2);

		CreateShape(out, row_prefix, QPointF(0, 0), shape, color, radius, scale, line_width, line_width2);

		out.write(row_prefix.data(), row_prefix.size());
		out.write("</symbol>\n");
	}

	void CreateShape(QIODevice& out, const std::string_view& row_prefix, const QPointF& pos, EShape shape, QRgb color, float radius, float scale, float line_width, float line_width2)
	{
		if (shape == EShape::Circle)
		{
			if (line_width2 > 0)
			{
				out.write(row_prefix.data(), row_prefix.size());
				qio_fwrite(out, "<circle cx=\"%f\" cy=\"%.2f\" r=\"%.2f\" fill=\"none\" stroke=\"black\" stroke-width=\"%f\" />\n",
					pos.x() + radius, pos.y() + radius, scale * CIRCLE_SHAPE_SCALE,
					line_width + line_width2 * 2);
			}
			out.write(row_prefix.data(), row_prefix.size());
			qio_fwrite(out, "<circle cx=\"%f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%.2x%.2x%.2x\" stroke=\"white\" stroke-width=\"%f\"/>\n",  // filter=\"url(#drop-shadow)\"
				pos.x() + radius, pos.y() + radius, scale * CIRCLE_SHAPE_SCALE,
				qRed(color), qGreen(color), qBlue(color), line_width);
		}
		else
		{
			const auto pts = GetShapePoints(shape);
			if (line_width2 > 0)
			{
				out.write(row_prefix.data(), row_prefix.size());
				out.write("<polygon points=\"");
				for (size_t i = 0; i < pts.size(); ++i)
				{
					qio_fwrite(out, i ? " %f,%f" : "%f,%f", pos.x() + radius + pts[i].x() * scale, pos.y() + radius + pts[i].y() * scale);
				}
				qio_fwrite(out, "\" fill=\"none\" stroke=\"black\" stroke-width=\"%f\" stroke-linejoin=\"round\"/>\n",  // filter=\"url(#drop-shadow)\"
					line_width + line_width2 * 2);
			}

			out.write(row_prefix.data(), row_prefix.size());
			out.write("<polygon points=\"");
			for (size_t i = 0; i < pts.size(); ++i)
			{
				qio_fwrite(out, i ? " %f,%f" : "%f,%f", pos.x() + radius + pts[i].x() * scale, pos.y() + radius + pts[i].y() * scale);
			}
			qio_fwrite(out, "\" fill=\"#%.2x%.2x%.2x\" stroke=\"white\" stroke-width=\"%f\" stroke-linejoin=\"round\"/>\n",  // filter=\"url(#drop-shadow)\"
				qRed(color), qGreen(color), qBlue(color), line_width);
		}
	}

	const char* DROP_SHADOW_FILTER =
R"(		<filter id="drop-shadow" x="-50%" y="-50%" width="200%" height="200%">
			<feDropShadow dx="1" dy="1" stdDeviation="1.5" />
		</filter>
)";

}