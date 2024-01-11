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

#include <stdexcept>

#ifdef _DEBUG
	#include <cassert>
	#define ASSERT(x) assert(x)
	#define VERIFY(x) assert(x)
#else
	#define ASSERT(x)
	#define VERIFY(x) x
#endif

#ifdef _DEBUG
	#define LOG_VERBOSE(fmt, ...) jass::Log(jass::EErrorLevel_Verbose, 0x0, fmt, ##__VA_ARGS__)
#else
	#define LOG_VERBOSE(fmt, ...)
#endif

#define LOG_INFO(fmt, ...)    jass::Log(jass::EErrorLevel_Info, 0x0, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) jass::Log(jass::EErrorLevel_Warning, 0x0, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   jass::Log(jass::EErrorLevel_Error, 0x0, fmt, ##__VA_ARGS__)

#define JASS_FORMAT_EXCEPTION throw jass::TFormatException<std::runtime_error>

namespace jass
{
	enum EErrorLevel
	{
		EErrorLevel_Verbose,
		EErrorLevel_Info,
		EErrorLevel_Warning,
		EErrorLevel_Error,
	};

	void SetLogFilePath(const char* path);

	void Log(EErrorLevel level, const char* domain, const char* fmt, ...);

	int FormatString(char* buffer, size_t buffer_size, const char* fmt, ...);

	template <typename TException, typename ... TArgs>
	TException TFormatException(const char* fmt, TArgs&&... args)
	{
		char buffer[4096];
		FormatString(buffer, sizeof(buffer), fmt, std::forward<TArgs>(args)...);
		return TException(buffer);
	}
}