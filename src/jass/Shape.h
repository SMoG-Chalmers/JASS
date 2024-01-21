#pragma once

#include <span>
#include <QtGui/qicon.h>
#include <QtGui/qimage.h>
#include <QtCore/qpoint.h>

class QPoint;
class QString;

namespace jass
{
	enum class EShape
	{
		Circle,
		Triangle,
		Square,
		Diamond,
		Pentagon,
		Hexagon,
		Star,
		_COUNT,
	};

	const QString& ShapeToString(EShape shape);

	bool TryGetShapeFromString(const QString& s, EShape& out_shape);

	extern const float CIRCLE_SHAPE_SCALE;

	std::span<const QPointF> GetShapePoints(EShape shape);

	QImage CreateShapeImage(EShape shape, QRgb color, int outline_width, const QPoint& shadow_offset, int shadow_blur_radius, int size);
}