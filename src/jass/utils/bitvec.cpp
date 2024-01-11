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

#include "bitvec.h"

namespace jass
{
	bitvec& bitvec::bitwise_xor(const bitvec& a, const bitvec& b)
	{
		resize(std::max(a.size(), b.size()));
		const auto n = std::min(a.m_Words.size(), b.m_Words.size());
		for (size_t i = 0; i < n; ++i)
		{
			m_Words[i] = a.m_Words[i] ^ b.m_Words[i];
		}
		for (size_t i = n; i < a.m_Words.size(); ++i)
		{
			m_Words[i] = a.m_Words[i];
		}
		for (size_t i = n; i < b.m_Words.size(); ++i)
		{
			m_Words[i] = b.m_Words[i];
		}
		return *this;
	}

	bitvec& bitvec::bitwise_or(const bitvec& a, const bitvec& b)
	{
		resize(std::max(a.size(), b.size()));
		const auto n = std::min(a.m_Words.size(), b.m_Words.size());
		for (size_t i = 0; i < n; ++i)
		{
			m_Words[i] = a.m_Words[i] | b.m_Words[i];
		}
		for (size_t i = n; i < a.m_Words.size(); ++i)
		{
			m_Words[i] = a.m_Words[i];
		}
		for (size_t i = n; i < b.m_Words.size(); ++i)
		{
			m_Words[i] = b.m_Words[i];
		}
		return *this;
	}
}