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
#include <qapplib/commands/CommandHistory.hpp>

#include <jass/commands/CmdCreateNode.h>
#include <jass/Debug.h>
#include <jass/ui/GraphWidget/EdgeGraphLayer.hpp>
#include <jass/ui/GraphWidget/NodeGraphLayer.hpp>

#include "NodeTool.h"

namespace jass
{
	const int MOUSE_WHEEL_NOTCH_SIZE = 120;  // Is there no better way of doing this?

	void CNodeTool::Activate(CJassEditor& ctx)
	{
		CGraphTool::Activate(ctx);
		
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

	static int APAN = 0;

	void CNodeTool::Paint(QPainter& painter, const QRect& rc)
	{
		if (m_Stamping && m_NodeLayer)
		{
			painter.setOpacity(.5f);
			m_NodeLayer->NodeSprite(m_CurrentCategory, ENodeSpriteStyle::Normal).Draw(painter, m_StampPos);
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
		if (event.button() == Qt::LeftButton && m_Stamping)
		{
			SNodeDesc desc;
			desc.Index = DataModel().NodeCount();
			desc.Category = m_CurrentCategory;
			desc.Pos = m_StampPos;
			CommandHistory().NewCommand<CCmdCreateNode>(DataModel(), desc);
			SetHoverNode(desc.Index);
			HideStamp();
		}
	}

	void CNodeTool::wheelEvent(QWheelEvent& event)
	{
		auto new_category = (m_CurrentCategory + (event.angleDelta().y() / MOUSE_WHEEL_NOTCH_SIZE)) % 6;
		if (new_category < 0)
		{
			new_category += 6;
		}
		SetCategory(new_category);
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
		if (m_NodeLayer)
		{
			GraphWidget().update(m_NodeLayer->NodeSprite(m_CurrentCategory, ENodeSpriteStyle::Normal).Rect().translated(pt));
		}
	}

	void CNodeTool::HideStamp()
	{
		if (m_Stamping && m_NodeLayer)
		{
			GraphWidget().update(m_NodeLayer->NodeSprite(m_CurrentCategory, ENodeSpriteStyle::Normal).Rect().translated(m_StampPos));
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