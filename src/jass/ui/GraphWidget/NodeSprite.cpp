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

#include <QtGui/qpainter.h>
#include <jass/ui/ImageFx.h>
#include "NodeSprite.h"

namespace jass
{
	void CreateNodeSprite(const SNodeSpriteDesc& desc, QPixmap& out_pixmap, QPoint& out_origin)
	{
		const auto points = GetShapePoints(desc.Shape);

		const float radius = desc.Radius;

		const QPointF shadowOffset = { std::round(desc.ShadowOffset.x()), std::round(desc.ShadowOffset.y()) };  // Limitation in DropShadow function

		const float radius_ceil = std::ceil(radius + desc.OutlineWidth * .5f + desc.OutlineWidth2);
		const float shadow_blur_radius_ceil = std::ceil(desc.ShadowBlurRadius);

		const QPointF bb_min = { shadowOffset.x() - radius_ceil - shadow_blur_radius_ceil, shadowOffset.y() - radius_ceil - shadow_blur_radius_ceil };
		const QPointF bb_max = { shadowOffset.x() + radius_ceil + shadow_blur_radius_ceil, shadowOffset.y() + radius_ceil + shadow_blur_radius_ceil };

		const QPoint origin = { -(int)std::floor(bb_min.x()), -(int)std::floor(bb_min.y()) };
		const QPoint dim = { origin.x() + (int)std::ceil(bb_max.x()), origin.y() + (int)std::ceil(bb_max.y())};
		QImage image(dim.x(), dim.y(), QImage::Format_ARGB32);

		// Clear
		image.fill(Qt::transparent);

		// Create a QPainter to draw on the QPixmap
		QPainter painter(&image);

		painter.setRenderHint(QPainter::Antialiasing, true);

		QPolygonF polygon;
		if (EShape::Circle != desc.Shape)
		{
			polygon.reserve((int)points.size());
			for (const auto& pt : points)
			{
				polygon.append(QPointF(pt.x() * radius + (float)origin.x(), pt.y() * radius + (float)origin.y()));
			}
		}

		if (desc.OutlineWidth2 > 0.0f)
		{
			QPen pen;
			pen.setColor(desc.OutlineColor2); // Set the color of the circle
			pen.setWidthF(desc.OutlineWidth + desc.OutlineWidth2 * 2);       // Set the width of the circle outline
			//pen.setCapStyle(Qt::RoundCap);
			pen.setJoinStyle(Qt::RoundJoin);
			painter.setPen(pen);

			painter.setBrush(Qt::NoBrush);

			if (EShape::Circle == desc.Shape)
			{
				painter.drawEllipse(QRectF(
					(float)origin.x() - radius,
					(float)origin.y() - radius,
					radius * CIRCLE_SHAPE_SCALE * 2.0f,
					radius * CIRCLE_SHAPE_SCALE * 2.0f));
			}
			else
			{
				painter.drawPolygon(polygon);
			}
		}

		// Set the pen
		QPen pen;
		pen.setColor(desc.OutlineColor);
		pen.setWidthF(desc.OutlineWidth);
		pen.setJoinStyle(Qt::RoundJoin);
		painter.setPen(pen);

		// Set the brush
		QBrush brush(desc.FillColor);
		painter.setBrush(brush);

		if (EShape::Circle == desc.Shape)
		{
			painter.drawEllipse(QRectF(
				(float)origin.x() - radius,
				(float)origin.y() - radius,
				radius * CIRCLE_SHAPE_SCALE * 2.0f,
				radius * CIRCLE_SHAPE_SCALE * 2.0f));
		}
		else
		{
			painter.drawPolygon(polygon);
		}

		DropShadow(image, desc.ShadowColor, desc.ShadowBlurRadius, (int)shadowOffset.x(), (int)shadowOffset.y(), 2.0f);

		out_pixmap = QPixmap::fromImage(image);
		out_origin = { origin.x() - desc.Offset.x(), origin.y() - desc.Offset.y() };
	}
}