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

#include <atomic>
#include <bit>
#include <stdexcept>
#include <vector>
#include <qapplib/Debug.h>

namespace jass
{
	class bitvec
	{
	public:
		static const size_t npos = (size_t)-1;

		bitvec() {}
		bitvec(const bitvec& rhs) : m_Size(rhs.m_Size), m_Words(rhs.m_Words) {}
		bitvec(bitvec&& rhs) noexcept : m_Size(rhs.m_Size), m_Words(std::move(rhs.m_Words)) { rhs.m_Size = 0; }

		inline void operator=(const bitvec& rhs);
		inline void operator=(bitvec&& rhs) noexcept;

		inline void clear();

		inline void clearAll();

		inline void set_all();

		inline size_t size() const;

		inline bool empty() const;

		inline void resize(size_t size);

		inline void set(size_t index);

		inline void set(size_t index, bool set);

		inline void clear(size_t index);

		inline bool get(size_t index) const;

		inline bool toggle(size_t index);

		inline bool operator[](size_t index) const;

		inline size_t count_set_bits() const;

		bitvec& bitwise_xor(const bitvec& a, const bitvec& b);

		bitvec& bitwise_or(const bitvec& a, const bitvec& b);

		inline size_t find_next_set_bit(size_t start_index = 0) const;

		template <class TLambda>
		void for_each_set_bit(TLambda&& fn) const;

	private:
		inline void clear_overflow_bits();

		typedef size_t word_t;
		const word_t WORD_BIT_COUNT = sizeof(word_t) * 8;
		
		size_t m_Size = 0;
		std::vector<word_t> m_Words;
	};

	inline void bitvec::operator=(const bitvec& rhs)
	{
		m_Size = rhs.m_Size;
		m_Words.resize(rhs.m_Words.size());
		memcpy(m_Words.data(), rhs.m_Words.data(), m_Words.size() * sizeof(word_t));
	} 

	inline void bitvec::operator=(bitvec&& rhs) noexcept
	{
		m_Size = rhs.m_Size;
		rhs.m_Size = 0;
		m_Words = std::move(rhs.m_Words);
	}

	inline void bitvec::clear()
	{
		m_Size = 0;
		m_Words.clear();
	}

	inline void bitvec::clearAll()
	{
		memset(m_Words.data(), 0, m_Words.size() * sizeof(word_t));
	}

	inline void bitvec::set_all()
	{
		memset(m_Words.data(), 0xFF, m_Words.size() * sizeof(word_t));
		clear_overflow_bits();
	}

	inline size_t bitvec::size() const
	{
		return m_Size;
	}

	inline bool bitvec::empty() const
	{
		return size() == 0;
	}

	inline void bitvec::resize(size_t size)
	{
		m_Words.resize((size + WORD_BIT_COUNT - 1) / WORD_BIT_COUNT);
		m_Size = size;
		clear_overflow_bits();
	}

	inline void bitvec::set(size_t index)
	{
		m_Words[index / WORD_BIT_COUNT] |= ((word_t)1 << (index % WORD_BIT_COUNT));
	}

	inline void bitvec::set(size_t index, bool set)
	{
		if (set)
			this->set(index);
		else
			this->clear(index);
	}

	inline void bitvec::clear(size_t index)
	{
		m_Words[index / WORD_BIT_COUNT] &= ~((word_t)1 << (index % WORD_BIT_COUNT));
	}

	inline bool bitvec::get(size_t index) const
	{
		return m_Words[index / WORD_BIT_COUNT] & ((word_t)1 << (index % WORD_BIT_COUNT));
	}

	inline bool bitvec::toggle(size_t index)
	{
		const word_t mask = ((word_t)1 << (index % WORD_BIT_COUNT));
		return 0 != ((m_Words[index / WORD_BIT_COUNT] ^= mask) & mask);
	}

	inline bool bitvec::operator[](size_t index) const
	{
		return get(index);
	}

	inline size_t bitvec::count_set_bits() const
	{
		size_t count = 0;
		for (auto word : m_Words)
		{
			count += std::popcount(word);
		}
		return count;
	}

	size_t bitvec::find_next_set_bit(size_t start_index) const
	{
		if (start_index >= m_Size)
		{
			return bitvec::npos;
		}
		auto word_index = start_index >> WORD_BIT_COUNT;
		auto w = m_Words[word_index] & ((size_t)-1 << (start_index % WORD_BIT_COUNT));
		while (0 == w)
		{
			++word_index;
			if (word_index >= m_Words.size())
			{
				return bitvec::npos;
			}
			w = m_Words[word_index];
		}
		return (w << WORD_BIT_COUNT) + std::countr_zero(w);
	}

	template <class TLambda>
	void bitvec::for_each_set_bit(TLambda&& fn) const
	{
		size_t n = 0;
		for (auto w : m_Words)
		{
			while (w)
			{
				const auto bit_index = std::countr_zero(w);
				w &= ~((word_t)1 << bit_index);
				fn(n + (size_t)bit_index);
			}
			n += WORD_BIT_COUNT;
		}
	}

	inline void bitvec::clear_overflow_bits()
	{
		if (!m_Words.empty())
		{
			m_Words.back() &= ((word_t)-1 >> (WORD_BIT_COUNT - (m_Size % WORD_BIT_COUNT)));
		}
	}


	// bitvec_set_bit_indices_view

	template <typename TIndex = size_t>
	class bitvec_set_bit_indices_view
	{
	public:
		bitvec_set_bit_indices_view(const bitvec& bits) : m_Bits(bits) {}

		class Iterator
		{
		public:
			Iterator(const bitvec& bits, size_t index) : m_Bits(bits) {}

			TIndex operator*() const
			{
				return (TIndex)m_Index;
			}

			Iterator& operator++()
			{
				QAPP_ASSERT(m_Index != bitvec::npos);
				m_Index = m_Bits.find_next_set_bit(m_Index);
				return *this;
			}

			bool operator!=(const Iterator& other) const
			{
				return m_Index != other.m_Index;
			}

		private:
			const bitvec& m_Bits;
			size_t m_Index;
		};

		Iterator begin() const
		{
			return Iterator(m_Bits, m_Bits.find_next_set_bit());
		}

		Iterator end() const
		{
			return Iterator(m_Bits, bitvec::npos);
		}

	private:
		const bitvec& m_Bits;
	};
}