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
#include <QtGui/qpixmap.h>

#include <jass/GraphModel.hpp>
#include "GraphWidget.hpp"

namespace jass
{
	class CJassDocument;

	struct ivec2
	{
		int x;
		int y;
	};

	struct SShapeSpriteDesc
	{
		EShape  Shape;
		float   Radius;
		float   OutlineWidth;
		float   OutlineWidth2 = 0;
		QPointF ShadowOffset;
		float   ShadowBlurRadius;
		QColor  FillColor;
		QColor  OutlineColor;
		QColor  OutlineColor2;
		QColor  ShadowColor;
		ivec2   Offset = { 0,0 };
	};

	enum class ENodeSpriteStyle
	{
		Normal,
		Selected,
		Hilighted,
	};

	class CNodeGraphLayer: public QObject, public CGraphLayer
	{
		Q_OBJECT
	public:
		struct SSprite
		{
			QPoint  Origin;
			QPixmap Pixmap;

			QRect Rect() const;
			void Draw(QPainter& painter, const QPoint& at) const;
		};

		CNodeGraphLayer(CGraphModel& graph_model, CGraphSelectionModel& selection_model);

		QPoint NodeScreenPos(element_t node) const;

		inline const SSprite& NodeSprite(CGraphModel::category_index_t category, ENodeSpriteStyle style) const;

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
		static const uint8_t SPRITE_COUNT_PER_CATEGORY = 3;

		struct SNode
		{
			QPoint   Position;
			uint32_t Category;
		};

		bool IsNodeSelected(const SNode& node) const;
		inline bool IsNodeHilighted(size_t node_index) const;

		const SSprite& NodeSprite(const SNode& node) const;

		QRect NodeRect(const SNode& node) const;

		SSprite CreateSprite(const SShapeSpriteDesc& desc);

		QPoint NodeScreenPosFromModelPos(const QPointF& model_pos) const;
			
		void RebuildNodes();

		CGraphModel& m_GraphModel;
		CGraphSelectionModel& m_SelectionModel;

		int m_HitRadius = 5;
		std::vector<SSprite> m_Sprites;
		std::vector<SNode> m_Nodes;
		bitvec m_SelectionMask;
		bitvec m_TempSelectionMask;
		bitvec m_HilightMask;
	};

	inline const CNodeGraphLayer::SSprite& CNodeGraphLayer::NodeSprite(CGraphModel::category_index_t category, ENodeSpriteStyle style) const
	{
		auto sprite_category = (category == CGraphModel::NO_CATEGORY) ? 0 : (category % 6);
		return m_Sprites[sprite_category * SPRITE_COUNT_PER_CATEGORY + (uint32_t)style];
	}

	inline bool CNodeGraphLayer::IsNodeHilighted(size_t node_index) const
	{
		return m_HilightMask.get(node_index);
	}
}