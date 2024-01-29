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

#include <jass/utils/bitvec.h>
#include "../GraphTool.h"

namespace jass
{
	class CGraphSelectionModel;
	class CNodeGraphLayer;
	class CEdgeGraphLayer;

	class CEdgeTool : public CGraphTool
	{
	public:
		void Activate(const SGraphToolContext& ctx) override;
		void Deactivate() override;
		void mouseMoveEvent(QMouseEvent& event) override;
		void mousePressEvent(QMouseEvent& event) override;
		void mouseReleaseEvent(QMouseEvent& event) override;
		void keyPressEvent(QKeyEvent& event) override;

	private:
		enum EState
		{
			EState_Hover,
			EState_Connecting,
		};

		void SetState(EState state);
		void SetFromNode(CGraphLayer::element_t node);
		void SetToNode(CGraphLayer::element_t node);
		void HideLine();

		CGraphLayer::element_t m_FromNode = CGraphLayer::NO_ELEMENT;
		CGraphLayer::element_t m_ToNode = CGraphLayer::NO_ELEMENT;
		float m_LineWidth = 2.0f;
		QLineF m_Line;
		EState m_State = EState_Hover;
		CNodeGraphLayer* m_NodeLayer = nullptr;
		CEdgeGraphLayer* m_EdgeLayer = nullptr;
		bitvec m_TempBitVec;
	};
}