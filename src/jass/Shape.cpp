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

#include <array>
#include <numbers>

#include <QtCore/qstring.h>
#include <QtGui/qpainter.h>

#include <jass/ui/ImageFx.h>
#include <jass/Debug.h>
#include <jass/Shape.h>

namespace jass
{
	static const QString UNKNOWN_SHAPE_STRING("?");

	static const QString s_ShapeStrings[] =
	{
		"circle",
		"triangle",
		"square",
		"diamond",
		"pentagon",
		"hexagon",
		"star",
	};

	extern const float CIRCLE_SHAPE_SCALE = .85f;

	const QString& ShapeToString(EShape shape)
	{
		static_assert(std::size(s_ShapeStrings) == (size_t)EShape::_COUNT);
		if ((size_t)shape >= std::size(s_ShapeStrings))
		{
			ASSERT(false && "Unknown shape");
			return UNKNOWN_SHAPE_STRING;
		}
		return s_ShapeStrings[(size_t)shape];
	}

	bool TryGetShapeFromString(const QString& s, EShape& out_shape)
	{
		for (const auto& shapeString : s_ShapeStrings)
		{
			if (shapeString.compare(s, Qt::CaseInsensitive) == 0)
			{
				out_shape = (EShape)(&shapeString - s_ShapeStrings);
				return true;
			}
		}
		return false;
	}

	template <int N>
	constexpr std::array<QPointF, N> NShape(float y_offset = 0, float scale = 1, bool tilted = false)
	{
		std::array<QPointF, N> pts;
		for (int i = 0; i < N; ++i)
		{
			const float angle = ((float)i + (tilted ? .5f : .0f)) * ((float)std::numbers::pi * 2.0f / N);
			pts[i] = { sinf(angle) * scale, y_offset - cosf(angle) * scale };
		}
		return pts;
	}

	template <int N>
	constexpr std::array<QPointF, N * 2> StarShape()
	{
		std::array<QPointF, N * 2> pts;
		for (int i = 0; i < N * 2; ++i)
		{
			const float r = 1.0f - .5f * (i & 1);
			const float angle = (float)i * ((float)std::numbers::pi / N);
			pts[i] = { r * sinf(angle), r * -cosf(angle) };
		}
		return pts;
	}

	static const auto s_TriangleCoords = NShape<3>(0.1f, 1.1f);
	static const auto s_SquareCoords = NShape<4>(0, 1, true);
	static const auto s_DiamondCoords = NShape<4>(0);
	static const auto s_PentagonCoords = NShape<5>();
	static const auto s_HexagonCoords = NShape<6>(0, 1, true);
	static const auto s_StarCoords = StarShape<5>();

	std::span<const QPointF> GetShapePoints(EShape shape)
	{
		switch (shape)
		{
		case EShape::Triangle:
			return s_TriangleCoords;
		case EShape::Square:
			return s_SquareCoords;
		case EShape::Diamond:
			return s_DiamondCoords;
		case EShape::Pentagon:
			return s_PentagonCoords;
		case EShape::Hexagon:
			return s_HexagonCoords;
		case EShape::Star:
			return s_StarCoords;
		}
		return std::span<const QPointF>();
	}

	QImage CreateShapeImage(EShape shape, QRgb color, int outline_width, const QPoint& shadow_offset, int shadow_blur_radius, int size)
	{
		const int shape_size = size - shadow_blur_radius * 2;
		const float radius = .5f * shape_size;

		const auto points = GetShapePoints(shape);

		const QPointF origin = { radius + shadow_blur_radius - shadow_offset.x(), radius + shadow_blur_radius - shadow_offset.y() };
		QImage image(size, size, QImage::Format_ARGB32);

		// Clear
		image.fill(Qt::transparent);

		// Create a QPainter to draw on the QPixmap
		QPainter painter(&image);

		painter.setRenderHint(QPainter::Antialiasing, true);

		const float mid_line_radius = radius - .5f * outline_width;

		QPolygonF polygon;
		if (EShape::Circle != shape)
		{
			polygon.reserve((int)points.size());
			for (const auto& pt : points)
			{
				polygon.append(QPointF(pt.x() * mid_line_radius + origin.x(), pt.y() * mid_line_radius + origin.y()));
			}
		}

		// Set the pen
		QPen pen;
		pen.setColor(Qt::white);
		pen.setWidthF(outline_width);
		pen.setJoinStyle(Qt::RoundJoin);
		painter.setPen(pen);

		// Set the brush
		QBrush brush(QColor::fromRgb(color));
		painter.setBrush(brush);

		if (EShape::Circle == shape)
		{
			const float r = mid_line_radius * CIRCLE_SHAPE_SCALE;
			painter.drawEllipse(QRectF(
				origin.x() - r,
				origin.y() - r,
				r * 2.0f,
				r * 2.0f));
		}
		else
		{
			painter.drawPolygon(polygon);
		}

		DropShadow(image, Qt::black, shadow_blur_radius, shadow_offset.x(), shadow_offset.y(), 2.0f);

		return image; 
	}
}