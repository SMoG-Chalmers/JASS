#include <cmath>
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