#include <array>
#include <numbers>

#include <QtCore/qstring.h>
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
}