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

#pragma once

#include <algorithm>
#include <QtCore/qline.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>

namespace jass
{
	inline QPoint QPointFromRoundedQPointF(const QPointF& pt)
	{
		return QPoint((int)std::round(pt.x()), (int)std::round(pt.y()));
	}

	inline QRect QRectFromLine(const QLine& line)
	{
		return QRect(
			std::min(line.p1().x(), line.p2().x()),
			std::min(line.p1().y(), line.p2().y()),
			std::abs(line.p2().x() - line.p1().x()),
			std::abs(line.p2().y() - line.p1().y()));
	}

	inline QRectF QRectFFromLine(const QLineF& line)
	{
		return QRectF(
			std::min(line.p1().x(), line.p2().x()),
			std::min(line.p1().y(), line.p2().y()),
			std::abs(line.p2().x() - line.p1().x()),
			std::abs(line.p2().y() - line.p1().y()));
	}

	inline bool Intersects(const QRectF& rc, const QLineF& line)
	{
		// Test AABB intersection
		const auto line_bb = QRectFFromLine(line);
		if (line_bb.right() < rc.left() ||
			line_bb.left() > rc.right() ||
			line_bb.bottom() < rc.top() ||
			line_bb.top() > rc.bottom())
		{
			return false;
		}

		// Zero length?
		if (line.isNull())
			return true;

		// v = Vector from p0 -> p1
		const auto v = line.p2() - line.p1();

		auto sn = [](const QPointF& a, const QPointF& b) -> float { return a.x() * b.y() - a.y() * b.x(); };

		// Test if all corners of rect are on same side of line
		const auto rc_rel = rc.adjusted(-line.p1().x(), -line.p1().y(), -line.p1().x(), -line.p1().y());
		const auto s = sn(v, rc_rel.topLeft());
		return
			s * sn(v, rc_rel.topRight()) <= 0 ||
			s * sn(v, rc_rel.bottomRight()) <= 0 ||
			s * sn(v, rc_rel.bottomLeft()) <= 0;
	}

	inline qreal SquaredDistanceFromPointToInfiniteLine(const QPointF& point, const QLineF& line)
	{
		qreal a = line.dy();
		qreal b = -line.dx();
		qreal c = -a * line.x1() - b * line.y1();

		// Calculate the squared distance using the formula
		qreal numerator = a * point.x() + b * point.y() + c;
		qreal denominator = a * a + b * b;

		return (numerator * numerator) / denominator;
	}

	inline qreal SquaredDistanceFromPointToLineSegment(const QPointF& point, const QLineF& line)
	{
		const auto vP0 = point - line.p1();

		if (line.isNull())
		{
			return QPointF::dotProduct(vP0, vP0);
		}

		const auto vL = line.p2() - line.p1();
		const auto t = QPointF::dotProduct(vL, vP0);
		if (t < 0)
			return QPointF::dotProduct(vP0, vP0);
		const auto vP1 = line.p2() - point;
		if (QPointF::dotProduct(vL, vP1) < 0)
			return QPointF::dotProduct(vP1, vP1);

		const auto lenLsqr = QPointF::dotProduct(vL, vL);
		const auto lenP0sqr = QPointF::dotProduct(vP0, vP0);
		const auto dist_sqr = lenP0sqr - (t * t) / lenLsqr;

		return std::max(dist_sqr, (qreal)0);
	}

	inline qreal DistanceFromPointToLineSegment(const QPointF& pt, const QLineF& line)
	{
		return std::sqrt(SquaredDistanceFromPointToLineSegment(pt, line));
	}
}