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

#include "GraphView.h"
#include "GraphBuilder.h"

namespace jass
{
	class CGraphData: public IGraphView, public IGraphBuilder
	{
	public:
		typedef IGraphView::edge_t edge_t;

		struct SNodeAttribute
		{
			SNodeAttributeDesc Desc;
			QByteArray Data;
		};

		// IGraphView interface
		size_t NodeCount() const override;
		size_t NodeAttributeCount() const override;
		SNodeAttributeDesc NodeAttributeDesc(size_t index) const override;
		void   GetNodeAttributeData(size_t index, void* buffer, size_t size) const override;
		size_t EdgeCount() const override;
		void   GetEdges(std::span<edge_t> out_edges) const override;

		// IGraphBuilder interface
		void SetNodeCount(size_t count) override;
		void AddNodeAttribute(const SNodeAttributeDesc& desc, const void* data, size_t size) override;
		void SetEdges(std::span<const edge_t> edges) override;

	private:
		size_t m_NodeCount = 0;
		std::vector<edge_t> m_Edges;
		std::vector<SNodeAttribute> m_NodeAttributes;
	};
}