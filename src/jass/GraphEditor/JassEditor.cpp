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


#include <QtCore/qfileinfo.h>
#include <QtCore/qbuffer.h>
#include <QtWidgets/qaction.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qmenu.h>

#include <jass/utils/range_utils.h>
#include <jass/Debug.h>
#include <jass/JassDocument.hpp>

#include <qapplib/actions/StandardActions.h>
#include <qapplib/commands/CommandHistory.hpp>
#include <qapplib/Workbench.hpp>

#include <jass/commands/CmdAddCategory.h>
#include <jass/commands/CmdAddGraphElements.h>
#include <jass/commands/CmdDeleteCategories.h>
#include <jass/commands/CmdDeleteGraphElements.h>
#include <jass/commands/CmdDuplicate.h>
#include <jass/commands/CmdFilpNodes.h>
#include <jass/commands/CmdModifyCategory.h>
#include <jass/commands/CmdSetBackgroundImage.h>
#include <jass/commands/CmdSetNodeCategory.h>
#include <jass/graphdata/GraphData.h>
#include <jass/graphdata/GraphModelImmutableDirectedGraphAdapter.h>
#include <jass/graphdata/GraphModelSubGraphView.h>
#include <jass/ui/GraphWidget/GraphWidget.hpp>
#include <jass/ui/GraphWidget/EdgeGraphLayer.hpp>
#include <jass/ui/GraphWidget/NodeGraphLayer.hpp>
#include <jass/ui/GraphWidget/ImageGraphLayer.hpp>
#include <jass/ui/CategoryView.hpp>
#include <jass/ui/MainWindow.hpp>
#include <jass/ui/SplitWidget.hpp>

#include "tools/EdgeTool.h"
#include "tools/NodeTool.h"
#include "tools/SelectionTool.h"

#include "Analyses.hpp"
#include "GraphClipboardData.h"
#include "GraphTool.h"
#include "JassEditor.hpp"

#define RES_PATH_PREFIX ":/"

namespace jass
{
	static const QString SUPPORTED_IMAGE_EXTENSIONS_NO_DOT[] = { "bmp", "jpeg", "jpg", "png" };

	// Common
	QActionGroup* CJassEditor::s_ToolsActionGroup = nullptr;
	qapp::CWorkbench* CJassEditor::s_Workbench = nullptr;
	QToolBar* CJassEditor::s_Toolbar = nullptr;
	CJassEditor::SToolActionHandles CJassEditor::s_ToolActionHandles;
	std::vector<CJassEditor::STool> CJassEditor::s_Tools;
	int CJassEditor::s_CurrentTool = 0;
	CCategoryView* CJassEditor::s_CategoryView = nullptr;
	CJassEditor::SActions CJassEditor::s_Actions;
	CJassEditor::SActionHandles CJassEditor::s_ActionHandles;

	class CGraphToolLayer : public CGraphLayer
	{
	public:
		CGraphToolLayer(CGraphWidget& graphWidget, CJassEditor& editor) : CGraphLayer(graphWidget), m_Editor(editor) {}

		void Paint(QPainter& painter, const QRect& rc) override
		{
			if (auto* tool = m_Editor.CurrentTool())
			{
				tool->Paint(painter, rc);
			}
		}

	private:
		CJassEditor& m_Editor;
	};

	CJassEditor::CJassEditor(CJassDocument& document)
		: m_Document(document)
		, m_CommandHistory(new qapp::CCommandHistory(*this, qapp::CPagePool::DefaultPagePool()))
		, m_SelectionModel(new CGraphSelectionModel(document.GraphModel()))
		, m_Analyses(new CAnalyses)
	{
		connect(m_CommandHistory.get(), &qapp::CCommandHistory::DirtyChanged, this, &CJassEditor::OnCommandHistoryDirtyChanged);

		connect(&DataModel(), &CGraphModel::NodesInserted, this, &CJassEditor::UpdateAnalyses);
		connect(&DataModel(), &CGraphModel::NodesRemoved,  this, &CJassEditor::UpdateAnalyses);
		connect(&DataModel(), &CGraphModel::EdgesAdded,    this, &CJassEditor::UpdateAnalyses);
		connect(&DataModel(), &CGraphModel::EdgesInserted, this, &CJassEditor::UpdateAnalyses);
		connect(&DataModel(), &CGraphModel::EdgesRemoved,  this, &CJassEditor::UpdateAnalyses);

		UpdateAnalyses();
	}
	
	CJassEditor::~CJassEditor()
	{

	}

	void CJassEditor::InitCommon(qapp::CWorkbench& workbench, qapp::CActionManager& action_manager, CMainWindow* main_window)
	{
		s_Workbench = &workbench;

		s_ToolsActionGroup = new QActionGroup(main_window);
		AddTool(action_manager, std::make_unique<CSelectionTool>(), "Select Tool", QIcon(RES_PATH_PREFIX "selecttool.png"), QKeySequence(Qt::Key_Q), &s_ToolActionHandles.SelectTool);
		AddTool(action_manager, std::make_unique<CNodeTool>(),      "Node Tool",   QIcon(RES_PATH_PREFIX "nodetool.png"),   QKeySequence(Qt::Key_W), &s_ToolActionHandles.NodeTool);
		AddTool(action_manager, std::make_unique<CEdgeTool>(),      "Edge Tool",   QIcon(RES_PATH_PREFIX "edgetool.png"),   QKeySequence(Qt::Key_E), &s_ToolActionHandles.EdgeTool);
		s_CurrentTool = 0;
		s_Tools[s_CurrentTool].Action->setChecked(true);

		s_ActionHandles.FlipHorizontal = action_manager.NewAction(nullptr, "Flip Horizontal",         ":/flip_horizontal.png", QKeySequence(Qt::Key_H), false, &s_Actions.FlipHorizontal);
		s_ActionHandles.FlipVertical   = action_manager.NewAction(nullptr, "Flip Vertical",           ":/flip_vertical.png",   QKeySequence(Qt::Key_V), false, &s_Actions.FlipVertical);
		s_ActionHandles.AddImage       = action_manager.NewAction(nullptr, "Load Background Image",   ":/image_add.png",       QKeySequence(), false, &s_Actions.AddImage);
		s_ActionHandles.RemoveImage    = action_manager.NewAction(nullptr, "Remove Background Image", ":/image_remove.png",    QKeySequence(), false, &s_Actions.RemoveImage);

		// Toolbar
		s_Toolbar = main_window->addToolBar("Map");
		//mainWindow->SetInitiallyHidden(s_Toolbar);
		s_Toolbar->setObjectName("JassToolBar");
		for (const auto& tool : s_Tools)
			s_Toolbar->addAction(tool.Action);
		s_Toolbar->addSeparator();
		s_Toolbar->addAction(s_Actions.FlipHorizontal);
		s_Toolbar->addAction(s_Actions.FlipVertical);
		s_Toolbar->addSeparator();
		s_Toolbar->addAction(s_Actions.AddImage);
		s_Toolbar->addAction(s_Actions.RemoveImage);
		s_Toolbar->setVisible(false);

		s_CategoryView = &main_window->CategoryView();
	}

	void CJassEditor::AddTool(qapp::CActionManager& action_manager, std::unique_ptr<CGraphTool> tool, QString title, const QIcon& icon, const QKeySequence& keys, qapp::HAction* ptrOutActionHandle)
	{
		ASSERT(s_ToolsActionGroup);

		auto* action = new QAction(icon, title, s_ToolsActionGroup);
		action->setShortcut(keys);
		action->setToolTip(title + " (" + keys.toString() + ")");
		action->setCheckable(true);

		if (ptrOutActionHandle)
		{
			*ptrOutActionHandle = action_manager.RegisterAction(action);
		}

		connect(action, &QAction::triggered, std::bind(&CJassEditor::OnSelectTool, (int)s_Tools.size()));

		RegisterTool(std::move(tool), title, action);
	}

	void CJassEditor::RegisterTool(std::unique_ptr<CGraphTool> tool, QString title, QAction* action)
	{
		s_Tools.resize(s_Tools.size() + 1);
		s_Tools.back().Title = title;
		s_Tools.back().Action = action;
		s_Tools.back().Tool = std::move(tool);
	}

	const QString CJassEditor::Title() const
	{
		return m_Document.Title();
	}

	bool CJassEditor::Dirty() const
	{
		return m_CommandHistory->Dirty();
	}

	bool CJassEditor::CanClose() const
	{
		return true;
	}

	std::unique_ptr<QWidget> CJassEditor::CreateWidget(QWidget* parent)
	{
		ASSERT(!m_SplitWidget);
		ASSERT(!m_GraphWidget);

		m_SplitWidget = new CSplitWidget(parent);

		m_GraphWidget = new CGraphWidget(m_SplitWidget);
		m_SplitWidget->AddWidget(m_GraphWidget);
		m_GraphWidget->setVisible(true);
		m_GraphWidget->SetDelegate(this);
		m_GraphWidget->EnableTooltips();
	
		m_GraphWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_GraphWidget, &CGraphWidget::customContextMenuRequested, this, &CJassEditor::OnCustomContextMenuRequested);

		{
			auto image_layer = std::make_unique<CImageGraphLayer>(*m_GraphWidget);
			m_ImageLayer = image_layer.get();
			m_GraphWidget->AppendLayer(std::move(image_layer));
			SetBackgroundImage(m_Document.ImageData(), m_Document.ImageExtensionNoDot());
		}

		{
			auto edge_layer = std::make_unique<CEdgeGraphLayer>(*m_GraphWidget, DataModel(), SelectionModel());
			m_GraphWidget->AppendLayer(std::move(edge_layer));
		}

		{
			auto node_layer = std::make_unique<CNodeGraphLayer>(*m_GraphWidget, *this);
			m_GraphWidget->AppendLayer(std::move(node_layer));
		}

		{
			auto tool_layer = std::make_unique<CGraphToolLayer>(*m_GraphWidget, *this);
			m_GraphWidget->AppendLayer(std::move(tool_layer));
		}

		m_GraphWidget->addAction(qapp::s_StandardActions.SelectAll);

		m_JustifiedGraphWidget = new CGraphWidget(m_SplitWidget);
		m_SplitWidget->AddWidget(m_JustifiedGraphWidget);
		m_JustifiedGraphWidget->setVisible(true);

		return std::unique_ptr<QWidget>(m_SplitWidget);
	}

	void CJassEditor::UpdateActions(qapp::CActionUpdateContext& ctx)
	{
		if (m_CommandHistory->CanUndo())
			ctx.Enable(qapp::s_StandardActionHandles.Undo);
		if (m_CommandHistory->CanRedo())
			ctx.Enable(qapp::s_StandardActionHandles.Redo);
		ctx.Enable(qapp::s_StandardActionHandles.SelectAll);
		ctx.Enable(qapp::s_StandardActionHandles.Paste);

		const bool hasSelectedNodes = SelectionModel().AnyNodesSelected();
		const bool hasSelectedEdges = SelectionModel().AnyEdgesSelected();

		if (hasSelectedNodes || hasSelectedEdges)
		{
			ctx.Enable(qapp::s_StandardActionHandles.Delete);
			ctx.Enable(qapp::s_StandardActionHandles.Cut);
		}

		if (hasSelectedNodes)
		{
			ctx.Enable(qapp::s_StandardActionHandles.Copy);
			ctx.Enable(qapp::s_StandardActionHandles.Duplicate);
			ctx.Enable(s_ActionHandles.FlipHorizontal);
			ctx.Enable(s_ActionHandles.FlipVertical);
		}

		ctx.Enable(s_ActionHandles.AddImage);
		if (!m_Document.ImageData().isNull())
		{
			ctx.Enable(s_ActionHandles.RemoveImage);
		}

		ctx.Enable(s_ToolActionHandles.SelectTool);
		ctx.Enable(s_ToolActionHandles.NodeTool);
		ctx.Enable(s_ToolActionHandles.EdgeTool);
	}

	bool CJassEditor::OnAction(qapp::HAction action_handle)
	{
		if (action_handle == qapp::s_StandardActionHandles.Undo && m_CommandHistory->CanUndo())
		{
			m_CommandHistory->Undo();
			return true;
		}
		else if (action_handle == qapp::s_StandardActionHandles.Redo && m_CommandHistory->CanRedo())
		{
			m_CommandHistory->Redo();
			return true;
		}
		else if (action_handle == qapp::s_StandardActionHandles.Delete)
		{
			m_CommandHistory->NewCommandOptional([&](auto& ctx)
				{
					return CCmdDeleteGraphElements::Create(ctx, DataModel(), SelectionModel());
				});
			return true;
		}
		else if (action_handle == s_ActionHandles.FlipHorizontal || action_handle == s_ActionHandles.FlipVertical)
		{
			if (SelectionModel().AnyNodesSelected())
			{
				m_CommandHistory->NewCommand<CCmdFilpNodes>(
					DataModel(),
					SelectionModel().NodeMask(),
					(action_handle == s_ActionHandles.FlipHorizontal) ? CCmdFilpNodes::Horizontal : CCmdFilpNodes::Vertical);
			}
			return true;
		}
		else if (action_handle == qapp::s_StandardActionHandles.Duplicate)
		{
			if (SelectionModel().AnyNodesSelected())
			{
				m_CommandHistory->NewCommand<CCmdDuplicate>(
					DataModel(),
					SelectionModel().NodeMask());
			}
			return true;
		}
		else if (action_handle == s_ActionHandles.AddImage)
		{
			QString filter;
			for (const auto& extension_no_dot : SUPPORTED_IMAGE_EXTENSIONS_NO_DOT)
			{
				filter += filter.isEmpty() ? "*." : " *.";
				filter += extension_no_dot;
			}
			auto path = QFileDialog::getOpenFileName(QApplication::activeWindow(), "Select Background Image", s_Workbench->LastDirectory(), QString("Image Files (%1)").arg(filter));
			if (path.isEmpty())
			{
				return true;
			}
			QFile file(path);
			if (!file.open(QIODevice::ReadOnly))
			{
				// TODO: Report error
				return true;
			}
			m_CommandHistory->NewCommand<CCmdSetBackgroundImage>(*this, file.readAll(), QFileInfo(path).completeSuffix());
			return true;
		}
		else if (action_handle == s_ActionHandles.RemoveImage)
		{
			m_CommandHistory->NewCommand<CCmdSetBackgroundImage>(*this, QByteArray(), QString());
			return true;
		}
		else if (action_handle == qapp::s_StandardActionHandles.Cut)
		{
			CGraphModelSubGraphView subGraphView(DataModel(), SelectionModel().NodeMask());
			SetGraphClipboardData(subGraphView);
			m_CommandHistory->NewCommandOptional([&](auto& ctx)
				{
					return CCmdDeleteGraphElements::Create(ctx, DataModel(), SelectionModel());
				});
			return true;
		}
		else if (action_handle == qapp::s_StandardActionHandles.Copy)
		{
			CGraphModelSubGraphView subGraphView(DataModel(), SelectionModel().NodeMask());
			SetGraphClipboardData(subGraphView);
			return true;
		}
		else if (action_handle == qapp::s_StandardActionHandles.Paste)
		{
			CGraphData graphData;
			if (!TryGetGraphClipboardData(graphData))
			{
				return true;
			}
			CommandHistory().NewCommand<CCmdAddGraphElements>(DataModel(), graphData);
			return true;
		}
		else if (action_handle == qapp::s_StandardActionHandles.SelectAll)
		{
			SelectionModel().BeginModify();
			SelectionModel().DeselectAllEdges();
			auto mask = SelectionModel().NodeMask();
			mask.clearAll();
			mask.set_all();
			SelectionModel().SetNodeMask(mask);
			SelectionModel().EndModify();
			return true;
		}
		return false;
	}

	void CJassEditor::OnActivate()
	{
		CEditor::OnActivate();

		s_Toolbar->setVisible(true);

		s_Tools[s_CurrentTool].Tool.get()->Activate(*this);
		m_GraphWidget->SetInputProcessor(s_Tools[s_CurrentTool].Tool.get());

		// Hook up Category view
		s_CategoryView->SetCategories(&m_Document.Categories());
		connect(s_CategoryView, &CCategoryView::AddCategory, this, &CJassEditor::OnAddCategory);
		connect(s_CategoryView, &CCategoryView::RemoveCategories, this, &CJassEditor::OnRemoveCategories);
		connect(s_CategoryView, &CCategoryView::ModifyCategory, this, &CJassEditor::OnModifyCategory);
	}

	void CJassEditor::OnDeactivate()
	{
		// Disconnect Category view
		s_CategoryView->disconnect(this);
		s_CategoryView->SetCategories(nullptr);

		m_GraphWidget->SetInputProcessor(nullptr);
		s_Tools[s_CurrentTool].Tool.get()->Deactivate();

		s_Toolbar->setVisible(false);

		CEditor::OnDeactivate();
	}

	void CJassEditor::OnSaved()
	{
		m_CommandHistory->SetCleanAtCurrentPosition();
	}

	QString CJassEditor::ToolTipText(size_t layer_index, CGraphLayer::element_t element)
	{
		if (auto* node_layer = dynamic_cast<CNodeGraphLayer*>(&m_GraphWidget->Layer(layer_index)))
		{
			const auto node_index = (CGraphModel::node_index_t)element;

			QString s;
			s.reserve(128);
			s += "<table>";

			s += QString("<tr><td>Name:</td><td>%1</td></tr>").arg(DataModel().NodeName(node_index));

			s += "<tr><td>Category:< / td><td>";
			const auto category = (size_t)DataModel().NodeCategory(node_index);
			s += category < m_Document.Categories().Size() ? m_Document.Categories().Name(category) : QString("None");
			s += "</td></tr>";

			const auto metric_count = m_Analyses->MetricCount();
			for (size_t metric_index = 0; metric_index < metric_count; ++metric_index)
			{
				s += "<tr><td>";
				s += m_Analyses->MetricName(metric_index);
				s += ":</td><td>";
				const auto value = m_Analyses->MetricValue(metric_index, node_index);
				s += std::isnan(value) ? QString("-") : QString("%1").arg(value);
				s += "</td></tr>";
			}

			s += "</table>";
			return s;
		}
		return QString();
	}

	CGraphModel& CJassEditor::DataModel()
	{
		return m_Document.GraphModel();
	}

	CCategorySet& CJassEditor::Categories()
	{
		return m_Document.Categories();
	}

	CGraphSelectionModel& CJassEditor::SelectionModel()
	{
		return *m_SelectionModel;
	}

	void CJassEditor::SetBackgroundImage(const QByteArray& image_data, QString extension_no_dot)
	{
		m_Document.SetImage(image_data, extension_no_dot);
		if (m_ImageLayer)
		{
			QPixmap pixmap;
			if (!pixmap.loadFromData(image_data))
			{
				// TODO: Log error
				//QMessageBox
			}
			m_ImageLayer->SetImage(std::move(pixmap));
		}
	}

	void CJassEditor::SetCategoryForSelectedNodes(int category)
	{
		CommandHistory().NewCommand<CCmdSetNodeCategory>(
			DataModel(),
			SelectionModel().NodeMask(),
			(CGraphModel::category_index_t)category);
		//DataModel().BeginModifyNodes();
		//SelectionModel().NodeMask().for_each_set_bit([&](size_t node_index)
		//{
		//	DataModel().SetNodeCategory((CGraphModel::node_index_t)node_index, category);
		//});
		//DataModel().EndModifyNodes();
	}

	void CJassEditor::OnCommandHistoryDirtyChanged(bool dirty)
	{
		emit DirtyChanged(dirty);
	}

	void CJassEditor::OnCustomContextMenuRequested(const QPoint& pos)
	{
		QMenu contextMenu;
		contextMenu.addAction(qapp::s_StandardActions.Cut);
		contextMenu.addAction(qapp::s_StandardActions.Copy);
		contextMenu.addAction(qapp::s_StandardActions.Paste);
		contextMenu.addAction(qapp::s_StandardActions.Duplicate);
		contextMenu.addAction(qapp::s_StandardActions.Delete);

		auto& categories = m_Document.Categories();
		if (m_SelectionModel->AnyNodesSelected() && categories.Size() > 0)
		{
			int current_category = -2;
			m_SelectionModel->NodeMask().for_each_set_bit([&](size_t node_index)
				{
					const int category = DataModel().NodeCategory((CGraphModel::node_index_t)node_index);
					if (-2 == current_category) 
					{
						current_category = category;
					}
					else if (category != current_category)
					{
						current_category = -1;
					}
				});

			auto category_menu = new QMenu("Set Category");
			for (size_t category_index = 0; category_index < categories.Size(); ++category_index)
			{
				auto* action = new QAction(categories.Icon(category_index), categories.Name(category_index));
				connect(action, &QAction::triggered, [this, category_index]() { SetCategoryForSelectedNodes((int)category_index); });
				if ((size_t)current_category == category_index)
				{
					action->setCheckable(true);
					action->setChecked(true);
				}
				category_menu->addAction(action);
			}
			contextMenu.addSeparator();
			contextMenu.addMenu(category_menu);
		}

		contextMenu.exec(m_GraphWidget->mapToGlobal(pos));
	}

	void CJassEditor::UpdateAnalyses()
	{
		m_Analyses->EnqueueUpdate(CGraphModelImmutableDirectedGraphAdapter(DataModel()));
	}

	void CJassEditor::OnRemoveCategories(const QModelIndexList& indexes)
	{
		auto* arr = (size_t*)alloca(indexes.size() * sizeof(size_t));
		for (int i = 0; i < indexes.size(); ++i)
		{
			arr[i] = (size_t)indexes[i].row();
		}
		std::sort(arr, arr + indexes.size());  // Must be sorted
		CommandHistory().NewCommand<CCmdDeleteCategories>(std::span<const size_t>(arr, (size_t)indexes.size()));
	}

	void CJassEditor::OnAddCategory(const QString& name, QRgb color, EShape shape)
	{
		CommandHistory().NewCommand<CCmdAddCategory>(name, color, shape);
	}

	void CJassEditor::OnModifyCategory(int index, const QString& name, QRgb color, EShape shape)
	{
		CommandHistory().NewCommand<CCmdModifyCategory>((size_t)index, name, color, shape);
	}

	void CJassEditor::OnSelectTool(int tool_index)
	{
		if (s_CurrentTool == tool_index)
		{
			return;
		}

		s_Tools[s_CurrentTool].Tool->Deactivate();

		s_CurrentTool = tool_index;

		if (auto* jass_editor = dynamic_cast<CJassEditor*>(s_Workbench->CurrentEditor()))
		{
			auto* new_tool = CurrentTool();
			new_tool->Activate(*jass_editor);
			jass_editor->m_GraphWidget->SetInputProcessor(new_tool);
		}
	}


	///////////////////////
	// CJassEditor::STool

	CJassEditor::STool::STool() {}

	CJassEditor::STool::STool(STool&& rhs)
		: Title(std::move(rhs.Title))
		, Action(rhs.Action)
		, Tool(std::move(rhs.Tool))
	{
		rhs.Action = nullptr;
	}

	CJassEditor::STool::~STool() {}

	void CJassEditor::STool::operator=(STool&& rhs)
	{
		Title = std::move(rhs.Title);
		Action = rhs.Action;
		rhs.Action = nullptr;
		Tool = std::move(rhs.Tool);
	}
}

#include <moc_JassEditor.cpp>