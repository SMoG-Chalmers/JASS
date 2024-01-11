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

#include <span>
#include <vector>
#include <jass/Debug.h>

namespace jass
{
	template <class T>
	std::span<T> to_span(std::vector<T>& v)
	{
		return std::span<T>(v.data(), v.size());
	}

	template <class T>
	std::span<const T> to_const_span(const std::vector<T>& v)
	{
		return std::span<const T>(v.data(), v.size());
	}

	// NOTE: insert_indices are indices AFTER expansion, not before.
	template <class T, class TIndices>
	void expand_span(const std::span<T>& s, const TIndices& insert_indices, T empty_value = T())
	{
		auto to = (intptr_t)(s.size() - 1);
		auto from = to - std::ranges::distance(insert_indices);
		auto it_indices = insert_indices.rbegin();
		for (; to > from; --to)
		{
			if (*it_indices == to)
			{
				s[to] = empty_value;
				++it_indices;
			}
			else
			{
				s[to] = std::move(s[from--]);
			}
		}
	}

	template <class T, class TIndex>
	void collapse_span(const std::span<T>& s, const std::span<const TIndex>& remove_indices)
	{
		if (remove_indices.empty())
		{
			return;
		}
		TIndex to = remove_indices.front();
		TIndex remove_index = 0;
		for (TIndex from = to; from < s.size(); ++from)
		{
			if (remove_index < remove_indices.size())
			{
				if (remove_indices[remove_index] == from)
				{
					++remove_index;
					ASSERT((remove_index >= remove_indices.size() || remove_indices[remove_index - 1] < remove_indices[remove_index]) && "remove_indices is not sorted");
					continue;
				}
			}
			s[to++] = std::move(s[from]);
		}
	}

	// NOTE: hole_indices are indices AFTER expansion, not before.
	template <class T>
	void expand(std::vector<T>& v, std::ranges::input_range auto&& hole_indices, T empty_value = T())
	{
		const auto hole_count = std::ranges::distance(hole_indices);
		v.resize(v.size() + hole_count);
		expand_span(to_span(v), hole_indices, empty_value);
	}

	template <class T, class TIndex>
	void collapse(std::vector<T>& v, const std::span<const TIndex>& remove_indices)
	{
		collapse_span(to_span(v), remove_indices);
		v.resize(v.size() - remove_indices.size());
	}

	//template <class T, class TIterator>
	//TIterator erase_all(TIterator begin, TIterator end, const T& value)
	//{
	//	auto it_from = begin;
	//	for (; it_from != end && *it_from != value; ++it_from);
	//	auto it_to = it_from++;
	//	for (; it_from != end; ++it_from)
	//	{
	//		*it_to = std::move(*it_from);
	//		it_to += (size_t)(*it_from != value);
	//	}
	//	return it_to;
	//}

	template <class TIt>
	size_t remove_duplicates_from_ordered(TIt begin, TIt end)
	{
		if (begin == end)
		{
			return 0;
		}
		auto src = begin;
		auto dst = src++;
		for (; src != end && *src != *dst; ++src, ++dst);
		for (; src != end; ++src)
		{
			if (*src != *dst)
			{
				*(++dst) = *src;
			}
		}
		++dst;
		return dst - begin;
	}

	template <class TIndex> 
	void build_index_collapse_table(TIndex count, const std::span<const TIndex>& remove_indices, std::vector<TIndex>& out_table)
	{
		out_table.resize(count);
		TIndex to = 0;
		TIndex remove_index = 0;
		for (TIndex from = 0; from < count; ++from)
		{
			if (remove_index < remove_indices.size() && remove_indices[remove_index] == from)
			{
				++remove_index;
				out_table[from] = (TIndex)-1;
				continue;
			}
			else
			{
				out_table[from] = to++;
			}
		}
	}

	// NOTE: insert_indices are meant for AFTER expansion, not before.
	template <class TIndex>
	void build_index_expand_table(const std::span<const TIndex>& insert_indices, const std::span<TIndex>& out_table)
	{
		TIndex n = 0;
		size_t insert_index = 0;
		for (TIndex to = 0; to < (TIndex)out_table.size(); ++to, ++n)
		{
			for (; insert_index < insert_indices.size() && insert_indices[insert_index] == n; ++insert_index, ++n)
			{
				ASSERT(insert_index + 1 >= insert_indices.size() || insert_indices[insert_index + 1] > insert_indices[insert_index]);  // Verify sorted
			}
			out_table[to] = n;
		}
		
		// Verification only
		for (; insert_index < insert_indices.size(); ++insert_index, ++n)
		{
			ASSERT(insert_indices[insert_index] == n);
		}
		ASSERT(insert_indices.size() + out_table.size() == n);
	}

	template <class TIndex>
	void remap_indices(const std::span<TIndex>& indices, const std::span<const TIndex>& remap_table)
	{
		for (auto& index : indices)
		{
			index = remap_table[index];
		}
	}
}