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

#include <jass/utils/bitvec.h>
#include "../GraphTool.h"

namespace qapp
{
	class CWorkbench;
};

namespace jass
{
	class CGraphSelectionModel;

	class CSelectionTool : public CGraphTool
	{
	public:
		void Activate(const SGraphToolContext& ctx) override;
		void Deactivate() override;
		void Paint(QPainter& painter, const QRect& rc) override;
		void mouseMoveEvent(QMouseEvent& event) override;
		void mousePressEvent(QMouseEvent& event) override;
		void mouseReleaseEvent(QMouseEvent& event) override;
		void keyPressEvent(QKeyEvent& event) override;
		void keyReleaseEvent(QKeyEvent& event) override;

	private:
		enum EState
		{
			EState_Hover,
			EState_BoxSelection,
			EState_Move,
		};
		
		inline bool HasHilightedElement() const;
		void RemoveHilight();
		void SetHilightedElement(size_t layer_index, CGraphLayer::element_t element);

		void SetState(EState state);
		void UpdateSelectionRect(const QPoint& pt);
		void UpdateCursor(Qt::KeyboardModifiers modifiers);
		void HideSelectionRect();

		void DeselectAll();
		void SetSelection(size_t layer_index, const bitvec& selection_mask);

		bool CanMoveLayerElements(size_t layer_index) const;
		
		size_t m_HilightedLayerIndex = -1;
		CGraphLayer::element_t m_HilightedElement = CGraphLayer::NO_ELEMENT;

		CGraphSelectionModel* m_SelectionModel = nullptr;
		QPoint m_RefPoint;
		QRect m_SelectionRect;
		EState m_State = EState_Hover;
		size_t m_SelectionLayer = -1;
		size_t m_PreviousSelectionLayer = -1;
		bitvec m_PreviousSelectionMask;
		bitvec m_TempSelectionMask;
	};

	inline bool CSelectionTool::HasHilightedElement() const
	{ 
		return CGraphLayer::NO_ELEMENT != m_HilightedElement; 
	}
}