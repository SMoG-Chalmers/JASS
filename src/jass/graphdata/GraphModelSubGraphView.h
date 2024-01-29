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

#include <unordered_map>
#include <jass/GraphModel.hpp>
#include "GraphView.h"

namespace jass
{
	class CGraphModelSubGraphView: public IGraphView
	{
	public:
		CGraphModelSubGraphView(const CGraphModel& data_model, const bitvec& node_mask);

		size_t AttributeCount() const override;
		void   GetAttribute(size_t index, QString& out_name, QVariant& out_value) const override;
		size_t NodeCount() const override;
		size_t NodeAttributeCount() const override;
		SNodeAttributeDesc NodeAttributeDesc(size_t index) const override;
		void   GetNodeAttributeData(size_t index, void* buffer, size_t size) const override;
		size_t EdgeCount() const override;
		void   GetEdges(std::span<std::pair<uint32_t, uint32_t>> out_edges) const override;

	private:
		const CGraphModel* m_DataModel;
		bitvec m_NodeMask;
		std::unordered_map<uint32_t, uint32_t> m_NodeIndexMap;
	};
}