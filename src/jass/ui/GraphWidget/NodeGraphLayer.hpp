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
#include "SpriteGraphLayer.h"
#include "NodeSpriteSet.h"

namespace qapp
{
	class CCommandHistory;
}

namespace jass
{
	class CCategorySet;
	class CJassEditor;
	class CJassDocument;

	class CNodeGraphLayer: public QObject, public CSpriteGraphLayer
	{
		Q_OBJECT
	public:
		CNodeGraphLayer(CGraphWidget& graphWidget, CJassEditor& editor);

		// CSpriteGraphLayer overrides
		QPoint ItemPosition(element_t element) const override;
		size_t ItemSpriteIndex(element_t element) const override;

		inline CNodeSpriteSet& Sprites() { return m_Sprites; }

		// CGraphLayer overrides
		void SetHilighted(element_t edge, bool hilighted) override;
		void GetSelection(bitvec& out_selection_mask) const override;
		void SetSelection(const bitvec& selection_mask) const override;
		void OnViewChanged(const QRect& rc, float screen_to_model_scale) override;

		bool CanMoveElements() const override;
		void BeginMoveElements(const bitvec& element_mask) override;
		void MoveElements(const QPoint& delta) override;
		void EndMoveElements(bool apply) override;

	private Q_SLOTS:
		void OnSelectionChanged();
		void OnNodesRemoved(const CGraphModel::const_node_indices_t& node_indices);
		void OnNodesInserted(const CGraphModel::const_node_indices_t& node_indices, const CGraphModel::node_remap_table_t& remap_table);
		void OnNodesModified(const bitvec& node_mask);

	private:
		inline bool IsNodeSelected(element_t node_index) const;
		inline bool IsNodeHilighted(element_t node_index) const;

		void RebuildNodes();

		CGraphModel& m_GraphModel;
		CGraphSelectionModel& m_SelectionModel;
		qapp::CCommandHistory& m_CommandHistory;

		CNodeSpriteSet m_Sprites;
		bitvec m_SelectionMask;
		bitvec m_TempSelectionMask;
		bitvec m_HilightMask;
		bitvec m_MoveElementMask;
		std::vector<QPointF> m_TempPoints;
	};

	inline bool CNodeGraphLayer::IsNodeSelected(size_t node_index) const { return m_SelectionMask.get(node_index); }

	inline bool CNodeGraphLayer::IsNodeHilighted(size_t node_index) const { return m_HilightMask.get(node_index); }
}