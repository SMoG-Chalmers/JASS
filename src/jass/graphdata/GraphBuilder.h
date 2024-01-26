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

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

#include "GraphDataCommon.h"

namespace jass
{
	class IGraphBuilder
	{
	public:
		typedef std::pair<uint32_t, uint32_t> edge_t;

		virtual void SetAttribute(const QString& name, const QVariant& value) = 0;
		virtual void SetNodeCount(size_t count) = 0;
		virtual void AddNodeAttribute(const SNodeAttributeDesc& desc, const void* data, size_t size) = 0;
		virtual void SetEdges(std::span<const edge_t> edges) = 0;
	};
}