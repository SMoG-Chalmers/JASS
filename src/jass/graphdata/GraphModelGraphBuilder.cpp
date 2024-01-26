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

#include <jass/Debug.h>
#include "GraphModelGraphBuilder.h"

namespace jass
{
	static const SNodeAttributeDesc s_NodeAttributeDescs[] =
	{
		{ GRAPH_NODE_ATTTRIBUTE_POSITION, QVariant::PointF },
		{ GRAPH_NODE_ATTTRIBUTE_CATEGORY, QVariant::UInt },
	};

	CGraphModelGraphBuilder::CGraphModelGraphBuilder(CGraphModel& data_model)
		: m_DataModel(&data_model)
	{
	}

	void CGraphModelGraphBuilder::SetAttribute(const QString& name, const QVariant& value)
	{
		auto attribute_index = m_DataModel->FindAttribute(name);
		if (CGraphModel::NO_ATTRIBUTE != attribute_index)
		{
			m_DataModel->SetAttribute(attribute_index, value);
		}
	}

	void CGraphModelGraphBuilder::SetNodeCount(size_t count)
	{
		ASSERT(m_DataModel->NodeCount() == 0);
		m_DataModel->AddNodes(count);
	}

	void CGraphModelGraphBuilder::AddNodeAttribute(const SNodeAttributeDesc& desc, const void* data, size_t size)
	{
		if (auto* node_attribute = m_DataModel->FindNodeAttribute(desc.Name))
		{
			if (node_attribute->Type() != desc.Type)
			{
				LOG_ERROR("Data type mismatch for node attribute '%s'.", desc.Name.toStdString().c_str());
				return;
			}
			node_attribute->Init(data, size);
			return;
		}

		if (desc.Name == GRAPH_NODE_ATTTRIBUTE_POSITION)
		{
			const auto* pts = (const QPointF*)data;
			ASSERT(m_DataModel->NodeCount() * sizeof(*pts) == size);
			m_DataModel->BeginModifyNodes();
			for (CGraphModel::node_index_t i = 0; i < m_DataModel->NodeCount(); ++i)
			{
				m_DataModel->SetNodePosition(i, pts[i]);
			}
			m_DataModel->EndModifyNodes();
		}
		else if (desc.Name == GRAPH_NODE_ATTTRIBUTE_CATEGORY)
		{
			const auto* categories = (const uint32_t*)data;
			ASSERT(m_DataModel->NodeCount() * sizeof(*categories) == size);
			m_DataModel->BeginModifyNodes();
			for (CGraphModel::node_index_t i = 0; i < m_DataModel->NodeCount(); ++i)
			{
				m_DataModel->SetNodeCategory(i, categories[i]);
			}
			m_DataModel->EndModifyNodes();
		}
	}

	void CGraphModelGraphBuilder::SetEdges(std::span<const edge_t> edges)
	{
		ASSERT(m_DataModel->EdgeCount() == 0);
		m_DataModel->AddEdges(edges);
	}
}