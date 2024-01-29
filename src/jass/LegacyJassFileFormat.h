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

#include <span>

class QIODevice;

namespace jass
{
	class CJassDocument;

	const uint32_t LEGACY_JASS_FILE_ID_BYTE_COUNT = 4;

	bool IsLegacyJassFile(const std::span<const uint8_t>& initial_bytes);

	void LoadLegacyJassFile(QIODevice&, CJassDocument& out_document);
}