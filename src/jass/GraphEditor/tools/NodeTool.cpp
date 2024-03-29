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

#include <QtGui/qpainter.h>

#include <qapplib/Workbench.hpp>
#include <qapplib/commands/CommandHistory.hpp>

#include <jass/commands/CmdCreateNode.h>
#include <jass/Debug.h>
#include <jass/GraphEditor/CategorySet.hpp>
#include <jass/ui/GraphWidget/EdgeGraphLayer.hpp>
#include <jass/ui/GraphWidget/NodeGraphLayer.hpp>
#include <jass/ui/GraphWidget/CategorySpriteSet.hpp>

#include "NodeTool.h"

namespace jass
{
	const int MOUSE_WHEEL_NOTCH_SIZE = 120;  // Is there no better way of doing this?

	CNodeTool::CNodeTool()
	{
	}

	CNodeTool::~CNodeTool()
	{
	}

	void CNodeTool::SetSpriteSet(std::shared_ptr<CCategorySpriteSet> sprites)
	{
		m_Sprites = std::move(sprites);
	}

	void CNodeTool::Activate(const SGraphToolContext& ctx)
	{
		CGraphTool::Activate(ctx);
		
		CheckCategory();

		m_Stamping = false;
		m_NodeLayer = NodeLayer();

		SimulateMouseMoveEvent();
	}

	void CNodeTool::Deactivate()
	{
		HideStamp();
		SetHoverNode(CGraphLayer::NO_ELEMENT);

		CGraphTool::Deactivate();
	}

	void CNodeTool::Paint(QPainter& painter, const QRect& rc)
	{
		if (m_Stamping && m_NodeLayer && m_Sprites)
		{
			painter.setOpacity(.5f);
			m_Sprites->DrawSprite(
				m_Sprites->SpriteIndex(m_CurrentCategory, CCategorySpriteSet::EStyle::Normal),
				painter, 
				m_StampPos);
			painter.setOpacity(1);
		}
	}

	void CNodeTool::leaveEvent(QEvent& event)
	{
		HideStamp();
		SetHoverNode(CGraphLayer::NO_ELEMENT);
	}

	void CNodeTool::mouseMoveEvent(QMouseEvent& event)
	{
		HideStamp();

		auto hit_node = m_NodeLayer->HitTest(event.pos());

		SetHoverNode(hit_node);

		if (CGraphLayer::NO_ELEMENT == hit_node)
		{
			ShowStamp(event.pos());
		}
	}

	void CNodeTool::mousePressEvent(QMouseEvent& event)
	{
		// We first simulate a move event, to make sure hover state is correct. In some cases,
		// for example when context menu is showing, move events are not processed, which is why
		// this is needed.
		mouseMoveEvent(event);

		if (event.button() == Qt::LeftButton)
		{
			if (m_Stamping)
			{
				SNodeDesc desc;
				desc.Index = DataModel().NodeCount();
				desc.Category = m_CurrentCategory;
				desc.PositionF = GraphWidget().ModelFromScreen(m_StampPos);
				CommandHistory().NewCommand<CCmdCreateNode>(DataModel(), desc);
				SetHoverNode(desc.Index);
				HideStamp();
			}
		}
		else if (event.buttons() == Qt::RightButton)
		{
			if (m_HoverNode == CGraphLayer::NO_ELEMENT)
			{
				GraphWidget().DeselectAll();
			}
			else
			{
				m_NodeLayer->GetSelection(m_TempBitVec);
				if (!m_TempBitVec.get(m_HoverNode))
				{
					m_TempBitVec.clearAll();
					m_TempBitVec.set(m_HoverNode);
					GraphWidget().DeselectAll();
					m_NodeLayer->SetSelection(m_TempBitVec);
				}
			}
		}
	}

	void CNodeTool::wheelEvent(QWheelEvent& event)
	{
		if (!Categories().Size())
		{
			return;
		}
		auto new_category = (m_CurrentCategory + (event.angleDelta().y() / MOUSE_WHEEL_NOTCH_SIZE)) % (int)Categories().Size();
		if (new_category < 0)
		{
			new_category += (int)Categories().Size();
		}
		SetCategory(new_category);
	}

	void CNodeTool::keyPressEvent(QKeyEvent& event)
	{
		if (event.key() >= Qt::Key_1 && event.key() <= Qt::Key_9)
		{
			if (!Categories().Size())
			{
				return;
			}
			SetCategory(std::min((int)(event.key() - Qt::Key_1), (int)Categories().Size() - 1));
		}
	}

	void CNodeTool::CheckCategory()
	{
		m_CurrentCategory = Categories().Size() ? std::min((int)Categories().Size() - 1, m_CurrentCategory) : -1;
	}

	void CNodeTool::SetCategory(int category)
	{
		if (m_CurrentCategory == category)
		{
			return;
		}
		if (!m_Stamping)
		{
			m_CurrentCategory = category;
			return;
		}
		HideStamp();
		m_CurrentCategory = category;
		ShowStamp(m_StampPos);
	}

	void CNodeTool::ShowStamp(const QPoint& pt)
	{
		m_Stamping = true;
		m_StampPos = pt;
		if (m_Sprites)
		{
			const auto sprite_index = m_Sprites->SpriteIndex(m_CurrentCategory, CCategorySpriteSet::EStyle::Normal);
			GraphWidget().update(m_Sprites->SpriteRect(sprite_index).translated(pt));
		}
	}

	void CNodeTool::HideStamp()
	{
		if (m_Stamping && m_Sprites)
		{
			const auto sprite_index = m_Sprites->SpriteIndex(m_CurrentCategory, CCategorySpriteSet::EStyle::Normal);
			GraphWidget().update(m_Sprites->SpriteRect(sprite_index).translated(m_StampPos));
		}
		m_Stamping = false;
	}

	void CNodeTool::SetHoverNode(CGraphLayer::element_t node)
	{
		if (m_NodeLayer && m_HoverNode != CGraphLayer::NO_ELEMENT)
		{
			m_NodeLayer->SetHilighted(m_HoverNode, false);
		}

		m_HoverNode = node;

		if (m_NodeLayer && m_HoverNode != CGraphLayer::NO_ELEMENT)
		{
			m_NodeLayer->SetHilighted(m_HoverNode, true);
		}
	}
}