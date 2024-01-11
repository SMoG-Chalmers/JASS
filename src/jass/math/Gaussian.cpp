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


#include "Gaussian.h"
#include "Integral.h"

namespace jass
{
	void GenerateGaussianKernel(float sigma_range, const std::span<float>& out_values)
	{
		const unsigned int INTEGRAL_STEPS = 10;

		if (0 == out_values.size())
			return;

		const unsigned int radius = (unsigned int)out_values.size() - 1;

		const float step = sigma_range / (0.5f + radius);

		float x = 0.5f * step;
		out_values[0] = 2.0f * IntegralApprox(GaussianFunc, 0.0f, x, INTEGRAL_STEPS);

		for (unsigned int i = 1; i < (unsigned int)out_values.size(); ++i, x += step)
		{
			out_values[i] = IntegralApprox(GaussianFunc, x, x + step, INTEGRAL_STEPS);
		}

		// Normalize
		float sum = 0.5f * out_values[0];
		for (size_t i = 1; i < out_values.size(); ++i)
		{
			sum += out_values[i];
		}
		const float normalize_multiplier = 0.5f / sum;
		for (auto& value : out_values)
		{
			value *= normalize_multiplier;
		}
	}
}