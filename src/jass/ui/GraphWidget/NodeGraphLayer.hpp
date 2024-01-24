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

#include <vector>
#include <jass/GraphModel.hpp>
#include "GraphWidget.hpp"
#include "NodeSpriteSet.h"

namespace jass
{
	class CCategorySet;
	class CJassDocument;

	class CNodeGraphLayer: public QObject, public CGraphLayer
	{
		Q_OBJECT
	public:
		CNodeGraphLayer(CGraphWidget& graphWidget, CGraphModel& graph_model, CCategorySet& categories, CGraphSelectionModel& selection_model);

		QPoint NodeScreenPos(element_t node) const;

		inline CNodeSpriteSet& Sprites() { return m_Sprites; }

		// CGraphLayer overrides
		void Paint(QPainter& painter, const QRect& rc) override;
		element_t HitTest(const QPoint& pt) override;
		bool RangedHitTest(const QRect& rc, bitvec& out_hit_elements) const override;
		void SetHilighted(element_t edge, bool hilighted) override;
		void GetSelection(bitvec& out_selection_mask) const override;
		void SetSelection(const bitvec& selection_mask) const override;
		void OnViewChanged(const QRect& rc, float screen_to_model_scale) override;

	private Q_SLOTS:
		void OnSelectionChanged();
		void OnNodesRemoved(const CGraphModel::const_node_indices_t& node_indices);
		void OnNodesInserted(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table);
		void OnNodesModified(const bitvec& node_mask);

	private:
		struct SNode
		{
			QRect LastRect;  // Rect of last time it was drawn
		};

		bool IsNodeSelected(const SNode& node) const;
		inline bool IsNodeHilighted(size_t node_index) const;

		size_t NodeSpriteIndex(const SNode& node) const;

		QRect NodeRect(const SNode& node) const;

		void RebuildNodes();

		CGraphModel& m_GraphModel;
		CCategorySet& m_Categories;
		CGraphSelectionModel& m_SelectionModel;

		int m_HitRadius = 5;
		unsigned int m_CategoryCount = 0;
		CNodeSpriteSet m_Sprites;
		std::vector<SNode> m_Nodes;
		bitvec m_SelectionMask;
		bitvec m_TempSelectionMask;
		bitvec m_HilightMask;
	};

	inline bool CNodeGraphLayer::IsNodeHilighted(size_t node_index) const
	{
		return m_HilightMask.get(node_index);
	}
}