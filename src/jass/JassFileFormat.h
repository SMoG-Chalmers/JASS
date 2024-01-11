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

class QIODevice;

namespace jass
{
	class CJassDocument;

	const uint32_t JASS_FILE_ID_BYTE_COUNT = 4;

	bool IsJassFile(const std::span<const uint8_t>& initial_bytes);

	void LoadJassFile(QIODevice&, CJassDocument& out_document);

	void SaveJassFile(QIODevice&, const CJassDocument& out_document);
}