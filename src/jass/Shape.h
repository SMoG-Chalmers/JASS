#pragma once

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
}