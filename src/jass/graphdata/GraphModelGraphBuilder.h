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

#include <jass/GraphModel.hpp>
#include "GraphBuilder.h"

namespace jass
{
	class CGraphModelGraphBuilder : public IGraphBuilder
	{
	public:
		CGraphModelGraphBuilder(CGraphModel& data_model);

		// IGraphBuilder interface
		void SetAttribute(const QString& name, const QVariant& value) override;
		void SetNodeCount(size_t count) override;
		void AddNodeAttribute(const SNodeAttributeDesc& desc, const void* data, size_t size) override;
		void SetEdges(std::span<const edge_t> edges) override;

	private:
		CGraphModel* m_DataModel;
	};
}