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
#include "JassSvgExport.h"

namespace jass
{
	static void qio_fwrite(QIODevice& out, const char* format, ...);

	void ExportJassToSVG(QIODevice& out, const CJassDocument& out_document)
	{
		int width = 640, height = 480;

		qio_fwrite(out, "<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\">\n", width, height);

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
}
