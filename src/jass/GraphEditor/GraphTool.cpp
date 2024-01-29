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

#include <QtGui/qcursor.h>

#include <jass/ui/GraphWidget/EdgeGraphLayer.hpp>
#include <jass/ui/GraphWidget/NodeGraphLayer.hpp>

#include "GraphTool.h"

namespace jass
{
	CNodeGraphLayer* CGraphTool::NodeLayer()
	{
		for (size_t layer_index = 0; layer_index < GraphWidget().LayerCount(); ++layer_index)
		{
			if (auto* node_layer = dynamic_cast<CNodeGraphLayer*>(&GraphWidget().Layer(layer_index)))
			{
				return node_layer;
			}
		}
		return nullptr;
	}
	
	CEdgeGraphLayer* CGraphTool::EdgeLayer()
	{
		for (size_t layer_index = 0; layer_index < GraphWidget().LayerCount(); ++layer_index)
		{
			if (auto* edge_layer = dynamic_cast<CEdgeGraphLayer*>(&GraphWidget().Layer(layer_index)))
			{
				return edge_layer;
			}
		}
		return nullptr;
	}

	void CGraphTool::SimulateMouseMoveEvent()
	{
		const auto pt = GraphWidget().mapFromGlobal(QCursor::pos());
		if (pt.x() >= GraphWidget().width() || pt.y() >= GraphWidget().height())
		{
			return;
		}
		QMouseEvent evt(QEvent::MouseMove, pt, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
		mouseMoveEvent(evt);
	}
}