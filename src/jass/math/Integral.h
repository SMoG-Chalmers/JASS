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

#pragma once

namespace jass
{
	template <class TFunc> float IntegralApprox(TFunc&& F, float x0, float x1, unsigned int num_steps)
	{
		const float step = (x1 - x0) / num_steps;
		const float half_step = 0.5f * step;
		float sum = 0.0f;
		float x = x0 + half_step;
		for (unsigned int i = 0; i < num_steps; ++i, x += step)
			sum += F(x);
		return sum * step;
	}
}