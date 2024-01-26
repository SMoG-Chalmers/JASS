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

#include <jass/GraphModel.hpp>
#include "GraphView.h"

namespace jass
{
	class CGraphModelGraphView : public IGraphView
	{
	public:
		CGraphModelGraphView(const CGraphModel& data_model);

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
	};
}