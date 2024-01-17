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
}