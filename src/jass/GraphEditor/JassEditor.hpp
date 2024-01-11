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

#include <memory>
#include <qapplib/Editor.hpp>
#include <qapplib/actions/ActionManager.hpp>
#include <jass/ui/InputEventProcessor.h>

class QActionGroup;
class QMainWindow;
class QToolBar;

namespace qapp
{
	class CCommandHistory;
	class CWorkbench;
}

namespace jass
{
	class CGraphModel;
	class CGraphSelectionModel;
	class CGraphTool;
	class CGraphWidget;
	class CJassDocument;
	class CJassEditor;
	class CImageGraphLayer;

	class CJassEditor: public qapp::CEditor
	{
		Q_OBJECT
	public:
		CJassEditor(CJassDocument& document);
		~CJassEditor();

		// Common
		static void InitCommon(qapp::CWorkbench& workbench, qapp::CActionManager& action_manager, QMainWindow* mainWindow);
		static void AddTool(qapp::CActionManager& action_manager, std::unique_ptr<CGraphTool> tool, QString title, const QIcon& icon, const QKeySequence& keys, qapp::HAction* ptrOutActionHandle);
		static void RegisterTool(std::unique_ptr<CGraphTool> tool, QString title, QAction* action);

		// qapp::CEditor overrides
		const QString Title() const override;
		bool Dirty() const override;
		bool CanClose() const override;
		std::unique_ptr<QWidget> CreateWidget(QWidget* parent) override;
		void UpdateActions(qapp::CActionUpdateContext& ctx) override;
		void OnAction(qapp::HAction action_handle) override;
		void OnActivate() override;
		void OnDeactivate() override;
		void OnSaved() override;

		inline CGraphWidget& GraphWidget() { return *m_GraphWidget; }

		inline CJassDocument& JassDocument() { return m_Document; }

		CGraphModel& DataModel();

		CGraphSelectionModel& SelectionModel();

		inline qapp::CCommandHistory& CommandHistory() { return *m_CommandHistory; }

		inline static CGraphTool* CurrentTool() { return s_Tools[s_CurrentTool].Tool.get(); }

		void SetBackgroundImage(const QByteArray& image_data, QString extension_no_dot);

	private Q_SLOTS:
		void OnCommandHistoryDirtyChanged(bool dirty);

	private:
		CJassDocument& m_Document;
		CGraphWidget* m_GraphWidget = nullptr;
		std::unique_ptr<CGraphSelectionModel> m_SelectionModel;
		std::unique_ptr<qapp::CCommandHistory> m_CommandHistory;

		CImageGraphLayer* m_ImageLayer = nullptr;

		// Common
		static void OnSelectTool(int tool_index);

		struct SToolActionHandles
		{
			qapp::HAction SelectTool;
			qapp::HAction MoveTool;
			qapp::HAction NodeTool;
			qapp::HAction EdgeTool;
		};

		struct STool
		{
			STool();
			STool(const STool&) = delete;
			STool(STool&& rhs);
			~STool();

			void operator=(const STool&) = delete;
			void operator=(STool&& rhs);

			QString Title;
			QAction* Action;
			std::unique_ptr<CGraphTool> Tool;
		};


		// Common

		static qapp::CWorkbench* s_Workbench;
		static QActionGroup* s_ToolsActionGroup;
		static QToolBar* s_Toolbar;
		static SToolActionHandles s_ToolActionHandles;
		static std::vector<STool> s_Tools;
		static int s_CurrentTool;

		struct SActions
		{
			QAction* Duplicate = 0;
			QAction* FlipHorizontal = 0;
			QAction* FlipVertical = 0;
			QAction* AddImage = 0;
			QAction* RemoveImage = 0;
		};
		static SActions s_Actions;

		struct SActionHandles
		{
			qapp::HAction Duplicate = 0;
			qapp::HAction FlipHorizontal = 0;
			qapp::HAction FlipVertical = 0;
			qapp::HAction AddImage = 0;
			qapp::HAction RemoveImage = 0;
		};
		static SActionHandles s_ActionHandles;
	};
}