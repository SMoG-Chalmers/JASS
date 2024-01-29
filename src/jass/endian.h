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

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstdint>


namespace std
{
    template<std::integral T>
    constexpr T byteswap(T value) noexcept
    {
        static_assert(std::has_unique_object_representations_v<T>, "T may not have padding bits");
        auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
        std::ranges::reverse(value_representation);
        return std::bit_cast<T>(value_representation);
    }
}

namespace jass
{
    template<std::integral T>
    constexpr T be(T value) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            return std::byteswap(value);
        }
        return value;
    }

    inline float be(float value) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            auto x = std::byteswap(*(uint32_t*)&value);
            return *(float*)&x;
        }
        return value;
    }

    inline double be(double value) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            auto x = std::byteswap(*(uint64_t*)&value);
            return *(double*)&x;
        }
        return value;
    }

}
