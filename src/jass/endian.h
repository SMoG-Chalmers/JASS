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

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>

namespace jass
{
    template<std::integral T>
    constexpr T byteswap(T value) noexcept
    {
        static_assert(std::has_unique_object_representations_v<T>, "T may not have padding bits");
        auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
        std::ranges::reverse(value_representation);
        return std::bit_cast<T>(value_representation);
    }

    template<std::integral T>
    constexpr T be(T value) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            return byteswap(value);
        }
        return value;
    }

    constexpr float be(float value) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            auto x = byteswap(*(uint32_t*)&value);
            return *(float*)&x;
        }
        return value;
    }

    constexpr double be(double value) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            auto x = byteswap(*(uint64_t*)&value);
            return *(double*)&x;
        }
        return value;
    }

}