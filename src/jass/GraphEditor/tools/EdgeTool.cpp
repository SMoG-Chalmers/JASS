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

#include <jass/commands/CmdCreateEdge.h>
#include <jass/Debug.h>
#include <jass/ui/GraphWidget/EdgeGraphLayer.hpp>
#include <jass/ui/GraphWidget/NodeGraphLayer.hpp>

#include "EdgeTool.h"

namespace jass
{
	static const QRgb LINE_COLOR = qRgb(0x0a, 0x84, 0xff);

	void CEdgeTool::Activate(CJassEditor& ctx)
	{
		CGraphTool::Activate(ctx);

		m_NodeLayer = NodeLayer();
		m_EdgeLayer = EdgeLayer();

		SimulateMouseMoveEvent();
	}

	void CEdgeTool::Deactivate()
	{
		SetFromNode(CGraphLayer::NO_ELEMENT);
		SetToNode(CGraphLayer::NO_ELEMENT);
		HideLine();

		CGraphTool::Deactivate();
	}

	void CEdgeTool::SetFromNode(CGraphLayer::element_t node)
	{
		if (node == m_FromNode)
		{
			return;
		}

		if (CGraphLayer::NO_ELEMENT != m_FromNode)
		{
			m_NodeLayer->SetHilighted(m_FromNode, false);
		}

		m_FromNode = node;

		if (CGraphLayer::NO_ELEMENT != m_FromNode)
		{
			m_NodeLayer->SetHilighted(m_FromNode, true);
		}
	}

	void CEdgeTool::SetToNode(CGraphLayer::element_t node)
	{
		if (node == m_ToNode)
		{
			return;
		}

		if (CGraphLayer::NO_ELEMENT != m_ToNode)
		{
			m_NodeLayer->SetHilighted(m_ToNode, false);
		}

		m_ToNode = node;

		if (CGraphLayer::NO_ELEMENT != m_ToNode)
		{
			m_NodeLayer->SetHilighted(m_ToNode, true);
		}
	}

	void CEdgeTool::mouseMoveEvent(QMouseEvent& event)
	{
		auto hit_node = m_NodeLayer->HitTest(event.pos());

		if (EState_Hover == m_State)
		{
			SetFromNode(hit_node);
			return;
		}

		if (hit_node == m_FromNode)
		{
			hit_node = CGraphLayer::NO_ELEMENT;
		}

		if (CGraphLayer::NO_ELEMENT != hit_node)
		{
			m_Line.setP2(m_NodeLayer->NodeScreenPos(hit_node));
		}
		else
		{
			m_Line.setP2(QPointF(event.pos()));
		}

		SetToNode(hit_node);

		if (m_EdgeLayer)
		{
			m_EdgeLayer->SetTempLine(m_Line);
		}
	}

	void CEdgeTool::mousePressEvent(QMouseEvent& event)
	{
		if (event.button() == Qt::LeftButton)
		{
			if (CGraphLayer::NO_ELEMENT != m_FromNode)
			{
				m_Line = QLineF(m_NodeLayer->NodeScreenPos(m_FromNode), QPointF(event.pos()));
				if (m_EdgeLayer)
				{
					m_EdgeLayer->SetTempLine(m_Line);
				}
				SetState(EState_Connecting);
			}
			else
			{
				GraphWidget().DeselectAll();
			}
		}
	}

	void CEdgeTool::mouseReleaseEvent(QMouseEvent& event)
	{
		if (EState_Connecting == m_State && event.button() == Qt::LeftButton)
		{
			if (CGraphLayer::NO_ELEMENT != m_ToNode)
			{
				CommandHistory().NewCommandOptional([&](auto& ctx)
					{
						return CCmdCreateEdge::Create(ctx, DataModel(), CGraphModel::node_pair_t(m_FromNode, m_ToNode));
					});
			}

			SetState(EState_Hover);
		}
	}

	void CEdgeTool::keyPressEvent(QKeyEvent& event)
	{
		switch (event.key())
		{
		case Qt::Key_Escape:
			if (EState_Connecting == m_State)
			{
				SetState(EState_Hover);
			}
			break;
		}
	}

	void CEdgeTool::SetState(EState state)
	{
		if (m_State == state)
		{
			return;
		}

		// Cleanup previous state
		switch (m_State)
		{
		case EState_Connecting:
			SetFromNode(CGraphLayer::NO_ELEMENT);
			SetToNode(CGraphLayer::NO_ELEMENT);
			HideLine();
			break;
		}
		
		m_State = state;

		// Initialize next state
		switch (m_State)
		{
		case EState_Hover:
			SimulateMouseMoveEvent();
			break;
		}
	}

	void CEdgeTool::HideLine()
	{
		if (m_EdgeLayer)
		{
			m_EdgeLayer->HideTempLine();
		}
	}
}