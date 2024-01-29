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

#include <stdarg.h>
#include <stdio.h>

#include <QtCore/qdebug.h>
#include <QtCore/qdatetime.h>

#include "Debug.h"

namespace jass
{
	void Log(EErrorLevel level, const char* domain, const char* fmt, ...)
	{
		char buf[8192];
		va_list args;
		va_start(args, fmt);

		char* at = buf;

		// Add time stamp
		const auto currentTime = QDateTime::currentDateTime();
		const auto date = currentTime.date();
		const auto time = currentTime.time();
		snprintf(at, sizeof(buf) - (at - buf), "%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d: ",
			date.year(), date.month(), date.day(),
			time.hour(), time.minute(), time.second(), time.msec());
		at += strlen(at);

		// Format text
#ifdef _MSC_VER
		vsnprintf_s(at, sizeof(buf) - (at - buf), sizeof(buf) - (at - buf), fmt, args);
#else
		vsnprintf(at, sizeof(buf) - (at - buf), fmt, args);
#endif
		

		// Append line break
//#ifdef _MSC_VER
//		strcat_s(at, sizeof(buf) - (at - buf), "\n");
//#else
//		strcat(at, "\n");
//#endif

		qDebug() << buf;
	}

	int FormatString(char* buffer, size_t buffer_size, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		return vsnprintf(buffer, buffer_size, fmt, args);
	}
}
