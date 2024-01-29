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