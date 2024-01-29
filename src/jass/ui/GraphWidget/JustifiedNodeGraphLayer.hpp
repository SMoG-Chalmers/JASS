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

#include <vector>
#include <jass/GraphModel.hpp>
#include <jass/StandardNodeAttributes.h>
#include "ItemGraphLayer.h"
#include "GraphNodeTheme.hpp"

namespace qapp
{
	class CCommandHistory;
}

namespace jass
{
	class CAnalyses;
	class CCategorySet;
	class CJassEditor;
	class CJassDocument;

	class CJustifiedNodeGraphLayer: public QObject, public CItemGraphLayer
	{
		Q_OBJECT
	public:
		CJustifiedNodeGraphLayer(CGraphWidget& graphWidget, CJassEditor& editor, std::shared_ptr<CGraphNodeTheme>&& theme);
		~CJustifiedNodeGraphLayer();

		void SetTheme(std::shared_ptr<CGraphNodeTheme> theme);

		// CItemGraphLayer overrides
		QRect ItemRect(element_t element) const override;
		void DrawItem(element_t element, QPainter& painter, const QRect& rc) const override;

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
		void OnThemeUpdated();

	private:
		using EElementStyle = CGraphNodeTheme::EStyle;

		QPoint ElementPosition(element_t element) const;
		EElementStyle ElementStyle(element_t element) const;

		inline bool IsNodeSelected(element_t node_index) const;
		inline bool IsNodeHilighted(element_t node_index) const;

		void RebuildNodes();

		CGraphModel& m_GraphModel;
		CGraphSelectionModel& m_SelectionModel;
		CAnalyses& m_Analyses;
		qapp::CCommandHistory& m_CommandHistory;
		JPosition_NodeAttribute_t* m_JPositionNodeAttribute = nullptr;;

		std::shared_ptr<CGraphNodeTheme> m_Theme;

		bitvec m_SelectionMask;
		bitvec m_TempSelectionMask;
		bitvec m_HilightMask;
		bitvec m_MoveElementMask;
		std::vector<QPointF> m_TempPoints;
	};

	inline bool CJustifiedNodeGraphLayer::IsNodeSelected(size_t node_index) const { return m_SelectionMask.get(node_index); }

	inline bool CJustifiedNodeGraphLayer::IsNodeHilighted(size_t node_index) const { return m_HilightMask.get(node_index); }
}