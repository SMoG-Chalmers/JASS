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

#include <jass/ui/GraphWidget/GraphWidget.hpp>
#include "JassEditor.hpp"

namespace jass
{
	class CGraphModel;
	class CGraphSelectionModel;
	class CGraphWidget;
	class CJassEditor;
	class CEdgeGraphLayer;
	class CNodeGraphLayer;

	class CGraphTool : public CInputEventProcessor
	{
	public:
		virtual ~CGraphTool() {}

		virtual void Activate(CJassEditor& ctx) { m_Editor = &ctx; }
		virtual void Deactivate() { m_Editor = nullptr; }
		virtual void Paint(QPainter& painter, const QRect& rc) {}

		inline CGraphWidget& GraphWidget();
		inline const CGraphWidget& GraphWidget() const;
		inline CGraphModel& DataModel();
		inline CGraphSelectionModel& SelectionModel();
		inline qapp::CCommandHistory& CommandHistory();
		
		CNodeGraphLayer* NodeLayer();
		CEdgeGraphLayer* EdgeLayer();

	protected:
		void SimulateMouseMoveEvent();

	private:
		CJassEditor* m_Editor = nullptr;
	};

	inline CGraphWidget& CGraphTool::GraphWidget() { return m_Editor->GraphWidget(); }
	inline const CGraphWidget& CGraphTool::GraphWidget() const { return m_Editor->GraphWidget(); }
	inline CGraphModel& CGraphTool::DataModel() { return m_Editor->DataModel(); }
	inline CGraphSelectionModel& CGraphTool::SelectionModel() { return m_Editor->SelectionModel(); }
	inline qapp::CCommandHistory& CGraphTool::CommandHistory() { return m_Editor->CommandHistory(); }
}