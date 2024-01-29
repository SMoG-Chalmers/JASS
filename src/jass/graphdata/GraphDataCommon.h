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

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

// Standard graph attribute names
#define GRAPH_ATTTRIBUTE_ROOT_NODE "root-node"

// Standard graph node attribute names
#define GRAPH_NODE_ATTTRIBUTE_POSITION    "position"
#define GRAPH_NODE_ATTTRIBUTE_CATEGORY    "category"
#define GRAPH_NODE_ATTTRIBUTE_JUSTIFIED_POSITION "justified-position"

namespace jass
{
	struct SNodeAttributeDesc
	{
		QString Name;
		QVariant::Type Type;
	};

}