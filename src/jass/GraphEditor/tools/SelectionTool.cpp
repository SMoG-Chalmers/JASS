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

#include <QtGui/qpainter.h>

#include <qapplib/Workbench.hpp>

#include <jass/Debug.h>
#include <jass/GraphEditor/JassEditor.hpp>
#include <jass/utils/range_utils.h>

#include "SelectionTool.h"

namespace jass
{
	static const QRgb SELECTION_OVERLAY_FILL_COLOR = qRgba(0x00, 0x78, 0xD7, 0x40);
	static const QRgb SELECTION_OVERLAY_FRAME_COLOR = qRgba(0x00, 0x78, 0xD7, 0xFF);

	void CSelectionTool::Activate(const SGraphToolContext& ctx)
	{
		CGraphTool::Activate(ctx);

		SetState(EState_Hover);

		SimulateMouseMoveEvent();
	}

	void CSelectionTool::Deactivate()
	{
		SetState(EState_Hover);
		RemoveHilight();

		GraphWidget().setCursor(Qt::ArrowCursor);

		CGraphTool::Deactivate();
	}

	void CSelectionTool::Paint(QPainter& painter, const QRect& rc)
	{
		if (m_SelectionRect.isValid())
		{
			QBrush brush(QColor::fromRgba(SELECTION_OVERLAY_FILL_COLOR));
			painter.setBrush(brush);
			painter.setPen(Qt::NoPen);
			painter.drawRect(m_SelectionRect);
		}
	}

	void CSelectionTool::mouseMoveEvent(QMouseEvent& event)
	{
		switch (m_State)
		{
		case EState_Hover:
			{
				size_t layer_index = -1;
				CGraphLayer::element_t element = CGraphLayer::NO_ELEMENT;
				if (GraphWidget().HitTest(event.pos(), layer_index, element))
				{
					SetHilightedElement(layer_index, element);
				}
				else
				{
					RemoveHilight();
				}
				UpdateCursor(event.modifiers());
		}
			break;
		case EState_BoxSelection:
			// Safety check
			if (!(event.buttons() & Qt::LeftButton))
			{
				SetState(EState_Hover);
				return;
			}
			UpdateSelectionRect(event.pos());
			break;
		case EState_Move:
			{
				const auto delta_move_screen = event.pos() - m_RefPoint;
				m_RefPoint += delta_move_screen;
				auto& layer = GraphWidget().Layer(m_HilightedLayerIndex);
				layer.MoveElements(delta_move_screen);
			}
			break;
		}

		// Hovering

	}

	void CSelectionTool::mousePressEvent(QMouseEvent& event)
	{
		// We first simulate a move event, to make sure hover state is correct. In some cases,
		// for example when context menu is showing, move events are not processed, which is why
		// this is needed.
		mouseMoveEvent(event);


		if (event.button() == Qt::LeftButton)
		{
			if (m_SelectionLayer != (size_t)-1 && event.modifiers() & Qt::ControlModifier)
			{
				m_PreviousSelectionLayer = m_SelectionLayer;
				GraphWidget().Layer(m_SelectionLayer).GetSelection(m_PreviousSelectionMask);
			}
			else
			{
				m_PreviousSelectionLayer = -1;
				m_PreviousSelectionMask.clear();
			}
				
			if (HasHilightedElement())
			{
				auto* layer = &GraphWidget().Layer(m_HilightedLayerIndex);
				layer->GetSelection(m_TempSelectionMask);
				if (event.modifiers() & Qt::ControlModifier)
				{
					m_TempSelectionMask.toggle(m_HilightedElement);
					SetSelection(m_HilightedLayerIndex, m_TempSelectionMask);
				}
				else if (!m_TempSelectionMask.get(m_HilightedElement))
				{
					m_TempSelectionMask.clearAll();
					m_TempSelectionMask.set(m_HilightedElement);
					SetSelection(m_HilightedLayerIndex, m_TempSelectionMask);
				}
			}
			else if (!(event.modifiers() & Qt::ControlModifier))
			{
				DeselectAll();
			}

			if (!(event.modifiers() & Qt::ControlModifier) && HasHilightedElement() && CanMoveLayerElements(m_HilightedLayerIndex))
			{
				m_RefPoint = event.pos();
				auto& layer = GraphWidget().Layer(m_HilightedLayerIndex);
				layer.GetSelection(m_TempSelectionMask);
				layer.BeginMoveElements(m_TempSelectionMask);
				SetState(EState_Move);
			}
			else
			{
				m_RefPoint = event.pos();
				SetState(EState_BoxSelection);
			}
		}
		else if (event.buttons() == Qt::RightButton)
		{
			if (HasHilightedElement())
			{
				auto* layer = &GraphWidget().Layer(m_HilightedLayerIndex);
				layer->GetSelection(m_TempSelectionMask);
				if (!m_TempSelectionMask.get(m_HilightedElement))
				{
					m_TempSelectionMask.clearAll();
					m_TempSelectionMask.set(m_HilightedElement);
					SetSelection(m_HilightedLayerIndex, m_TempSelectionMask);
				}
			}
			else if (!(event.modifiers() & Qt::ControlModifier))
			{
				DeselectAll();
			}
		}
	}

	void CSelectionTool::mouseReleaseEvent(QMouseEvent& event)
	{
		switch (m_State)
		{
		case EState_BoxSelection:
			if (event.button() == Qt::LeftButton)
			{
				SetState(EState_Hover);
			}
			break;
		case EState_Move:
			GraphWidget().Layer(m_HilightedLayerIndex).EndMoveElements(true);
			SetState(EState_Hover);
			break;
		}
	}

	void CSelectionTool::keyPressEvent(QKeyEvent& event)
	{
		switch (event.key())
		{
		case Qt::Key_Escape:
			if (EState_Move == m_State)
			{
				// Cancel move
				GraphWidget().Layer(m_HilightedLayerIndex).EndMoveElements(false);
			}
			SetState(EState_Hover);
			break;
		}

		UpdateCursor(event.modifiers());
	}

	void CSelectionTool::keyReleaseEvent(QKeyEvent& event)
	{
		UpdateCursor(event.modifiers());
	}

	void CSelectionTool::RemoveHilight()
	{
		SetHilightedElement(-1, CGraphLayer::NO_ELEMENT);
	}
	
	void CSelectionTool::SetHilightedElement(size_t layer_index, CGraphLayer::element_t element)
	{
		if (m_HilightedLayerIndex == layer_index && m_HilightedElement == element)
		{
			return;
		}

		if (HasHilightedElement())
		{
			GraphWidget().Layer(m_HilightedLayerIndex).SetHilighted(m_HilightedElement, false);
		}

		m_HilightedLayerIndex = layer_index;
		m_HilightedElement = element;

		bool can_move = false;

		if (HasHilightedElement())
		{
			GraphWidget().Layer(m_HilightedLayerIndex).SetHilighted(m_HilightedElement, true);
			can_move = CanMoveLayerElements(m_HilightedLayerIndex);
		}
	}

	void CSelectionTool::SetState(EState state)
	{
		if (m_State == state)
		{
			return;
		}

		// Cleanup previous state
		switch (m_State)
		{
		case EState_BoxSelection:
			HideSelectionRect();
			break;
		}
		
		m_State = state;

	}

	void CSelectionTool::UpdateCursor(Qt::KeyboardModifiers modifiers)
	{
		Qt::CursorShape cursorShape = Qt::ArrowCursor;

		if (EState_Hover == m_State && HasHilightedElement() && CanMoveLayerElements(m_HilightedLayerIndex) && !(modifiers & Qt::ControlModifier))
		{
			cursorShape = Qt::SizeAllCursor;
		}
		else if (EState_Move == m_State)
		{
			cursorShape = Qt::SizeAllCursor;
		}

		GraphWidget().setCursor(cursorShape);
	}


	void CSelectionTool::UpdateSelectionRect(const QPoint& pt)
	{
		const QRect newRect(
			std::min(pt.x(), m_RefPoint.x()),
			std::min(pt.y(), m_RefPoint.y()),
			std::abs(pt.x() - m_RefPoint.x()),
			std::abs(pt.y() - m_RefPoint.y()));

		GraphWidget().update(newRect.united(m_SelectionRect));

		m_SelectionRect = newRect;

		size_t layer_index = -1;
		m_TempSelectionMask.clear();
		if (m_PreviousSelectionLayer != (size_t)-1)
		{
			layer_index = m_PreviousSelectionLayer;
			GraphWidget().Layer(layer_index).RangedHitTest(m_SelectionRect, m_TempSelectionMask);
			m_TempSelectionMask.bitwise_xor(m_PreviousSelectionMask, m_TempSelectionMask);
		}
		else
		{
			GraphWidget().RangedHitTest(m_SelectionRect, layer_index, m_TempSelectionMask);
		}
		SetSelection(layer_index, m_TempSelectionMask);
	}

	void CSelectionTool::HideSelectionRect()
	{
		if (!m_SelectionRect.isValid())
		{
			return;
		}
		GraphWidget().update(m_SelectionRect);
		m_SelectionRect = QRect();
	}

	void CSelectionTool::DeselectAll()
	{
		m_SelectionLayer = (size_t)-1;
		GraphWidget().DeselectAll();
	}

	void CSelectionTool::SetSelection(size_t layer_index, const bitvec& selection_mask)
	{
		if (layer_index != m_SelectionLayer)
		{
			if (m_SelectionLayer != (size_t)-1)
			{
				auto& layer = GraphWidget().Layer(m_SelectionLayer);
				layer.SetSelection(bitvec());
			}
			m_SelectionLayer = layer_index;
		}

		if (m_SelectionLayer != (size_t)-1)
		{
			GraphWidget().Layer(layer_index).SetSelection(selection_mask);
		}
	}

	bool CSelectionTool::CanMoveLayerElements(size_t layer_index) const
	{
		return GraphWidget().Layer(layer_index).CanMoveElements();
	}
}