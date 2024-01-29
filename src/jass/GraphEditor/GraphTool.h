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

#include <jass/ui/GraphWidget/GraphWidget.hpp>
#include "JassEditor.hpp"

namespace jass
{
	class CCategorySet;
	class CGraphModel;
	class CGraphSelectionModel;
	class CGraphWidget;
	class CJassEditor;
	class CEdgeGraphLayer;
	class CNodeGraphLayer;

	struct SGraphToolContext
	{
		CJassEditor* JassEditor = nullptr;
		CGraphWidget* GraphWidget = nullptr;
	};

	class CGraphTool : public CInputEventProcessor
	{
	public:
		virtual ~CGraphTool() {}

		virtual void Activate(const SGraphToolContext& ctx) { m_Editor = ctx.JassEditor; m_GraphWidget = ctx.GraphWidget; }
		virtual void Deactivate() { m_Editor = nullptr; }
		virtual void Paint(QPainter& painter, const QRect& rc) {}

		inline CGraphWidget& GraphWidget();
		inline const CGraphWidget& GraphWidget() const;
		inline CGraphModel& DataModel();
		inline CCategorySet& Categories();
		inline CGraphSelectionModel& SelectionModel();
		inline qapp::CCommandHistory& CommandHistory();
		
		CNodeGraphLayer* NodeLayer();
		CEdgeGraphLayer* EdgeLayer();

	protected:
		void SimulateMouseMoveEvent();

	private:
		CJassEditor* m_Editor = nullptr;
		CGraphWidget* m_GraphWidget = nullptr;
	};

	inline CGraphWidget& CGraphTool::GraphWidget() { return *m_GraphWidget; }
	inline const CGraphWidget& CGraphTool::GraphWidget() const { return *m_GraphWidget; }
	inline CGraphModel& CGraphTool::DataModel() { return m_Editor->DataModel(); }
	inline CCategorySet& CGraphTool::Categories() { return m_Editor->Categories(); }
	inline CGraphSelectionModel& CGraphTool::SelectionModel() { return m_Editor->SelectionModel(); }
	inline qapp::CCommandHistory& CGraphTool::CommandHistory() { return m_Editor->CommandHistory(); }
}