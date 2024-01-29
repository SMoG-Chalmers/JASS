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

#include <cmath>
#include <limits>
#include "Integration.h"

namespace jass
{
	template <typename T>
	inline constexpr T log(T b, T v)
	{
		return ::log(v) / ::log(b);
	}

	float CalculateIntegrationScore(unsigned int N, float TD, float& out_MD, float& out_RA, float& out_RRA)
	{
		if (N < 2)
		{
			out_MD  = std::numeric_limits<float>::quiet_NaN();
			out_RA  = std::numeric_limits<float>::quiet_NaN();
			out_RRA = std::numeric_limits<float>::quiet_NaN();
			return std::numeric_limits<float>::quiet_NaN();
		}
		out_MD = TD / (float)(N - 1);
		out_RA = 2.0f * (out_MD - 1.0f) / (N - 2);
		const float D = 2.0f * ((log(2.0f, (float)(N + 2) / 3) - 1.0f) * N + 1.0f) / ((N - 1) * (N - 2));
		out_RRA = out_RA / D;
		return 1.0f / out_RRA;
	}
}