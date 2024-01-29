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
#include <jass/commands/CmdSetGraphAttribute.h>
#include <jass/commands/CmdSetNodeAttributes.h>
#include <jass/commands/CmdSetNodeCategory.h>
#include <jass/graphdata/GraphData.h>
#include <jass/graphdata/GraphModelSubGraphView.h>
#include <jass/ui/GraphWidget/CategorySpriteSet.hpp>
#include <jass/ui/GraphWidget/GraphWidget.hpp>
#include <jass/ui/GraphWidget/EdgeGraphLayer.hpp>
#include <jass/ui/GraphWidget/JustifiedEdgeGraphLayer.hpp>
#include <jass/ui/GraphWidget/JustifiedNodeGraphLayer.hpp>
#include <jass/ui/GraphWidget/NodeGraphLayer.hpp>
#include <jass/ui/GraphWidget/ImageGraphLayer.hpp>
#include <jass/ui/GraphWidget/GraphNodeAnalysisTheme.hpp>
#include <jass/ui/GraphWidget/GraphNodeCategoryTheme.hpp>
#include <jass/ui/GraphWidget/PaletteSpriteSet.h>
#include <jass/ui/CategoryView.hpp>
#include <jass/ui/MainWindow.hpp>
#include <jass/ui/SplitWidget.hpp>
#include <jass/StandardNodeAttributes.h>
#include <jass/JassSvgExport.h>

#include "tools/EdgeTool.h"
#include "tools/NodeTool.h"
#include "tools/SelectionTool.h"

#include "Analyses.hpp"
#include "GraphClipboardData.h"
#include "GraphTool.h"
#include "JassEditor.hpp"
#include "JustifiedGraph.h"

#define RES_PATH_PREFIX ":/"

namespace jass
{
	static const QString SUPPORTED_IMAGE_EXTENSIONS_NO_DOT[] = { "bmp", "jpeg", "jpg", "png" };

	//static const QRgb SPECTRAL_PALETTE[] =
	//{
	//	qRgb(0x00, 0x2E, 0x47), 
	//	qRgb(0x5D, 0xA2, 0x83), 
	//	qRgb(0xA6, 0xC4, 0x9C), 
	//	qRgb(0xEF, 0xE6, 0xB4), 
	//	qRgb(0xFD, 0xC1, 0x4A), 
	//	qRgb(0xF5, 0x7E, 0x00), 
	//	qRgb(0xE0, 0x1F, 0x1F)
	//};
	static const QRgb SPECTRAL_PALETTE[] =
	{
		qRgb(0x29, 0x39, 0x9a),
		qRgb(0x00, 0x6d, 0xb8),
		qRgb(0x01, 0xa4, 0xb8),
		qRgb(0x0a, 0xa6, 0x66),
		qRgb(0xa6, 0xd0, 0x4e),
		qRgb(0xff, 0xf3, 0x01),
		qRgb(0xfc, 0xb1, 0x16),
		qRgb(0xf5, 0x80, 0x22),
		qRgb(0xf4, 0x59, 0x24),
		qRgb(0xed, 0x1c, 0x28),
	};

	// Common
	QActionGroup* CJassEditor::s_ToolsActionGroup = nullptr;
	qapp::CWorkbench* CJassEditor::s_Workbench = nullptr;
	QToolBar* CJassEditor::s_Toolbar = nullptr;
	CJassEditor::SToolActionHandles CJassEditor::s_ToolActionHandles;
	std::vector<CJassEditor::STool> CJassEditor::s_Tools;
	CNodeTool* CJassEditor::s_NodeTool = nullptr;
	std::unique_ptr<CGraphTool> CJassEditor::s_JustifiedSelectionTool;
	int CJassEditor::s_CurrentTool = 0;
	CCategoryView* CJassEditor::s_CategoryView = nullptr;
	CJassEditor::SActions CJassEditor::s_Actions;
	CJassEditor::SActionHandles CJassEditor::s_ActionHandles;
	static std::unique_ptr<CPaletteSpriteSet> s_AnalysisSpriteSet;
	static QMenu* s_VisualizationMenu = nullptr;
	static QAction* s_VisualizationMenuAction = nullptr;
	static std::vector<QAction*> s_VisualizationActions;

	class CGraphToolLayer : public CGraphLayer
	{
	public:
		CGraphToolLayer(CGraphWidget& graphWidget, CJassEditor& editor) : CGraphLayer(graphWidget), m_Editor(editor) {}

		void Paint(QPainter& painter, const QRect& rc) override
		{
			auto* tool = (&GraphWidget() == &m_Editor.GraphWidget()) ?
				m_Editor.CurrentTool() : m_Editor.CurrentJustifiedTool();
			if (tool)
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

		connect(&DataModel(), &CGraphModel::NodesInserted, this, &CJassEditor::OnNodesRemapped);
		connect(&DataModel(), &CGraphModel::NodesRemoved,  this, &CJassEditor::OnNodesRemapped);
		connect(&DataModel(), &CGraphModel::EdgesAdded,    this, &CJassEditor::UpdateAnalyses);
		connect(&DataModel(), &CGraphModel::EdgesInserted, this, &CJassEditor::UpdateAnalyses);
		connect(&DataModel(), &CGraphModel::EdgesRemoved,  this, &CJassEditor::UpdateAnalyses);
		connect(&DataModel(), &CGraphModel::AttributeChanged, this, &CJassEditor::UpdateAnalyses);

		m_CategorySpriteSet = std::make_shared<CCategorySpriteSet>(Categories());

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
		s_NodeTool = dynamic_cast<CNodeTool*>(s_Tools.back().Tool.get());
		ASSERT(s_NodeTool);
		AddTool(action_manager, std::make_unique<CEdgeTool>(),      "Edge Tool",   QIcon(RES_PATH_PREFIX "edgetool.png"),   QKeySequence(Qt::Key_E), &s_ToolActionHandles.EdgeTool);
		s_CurrentTool = 0;
		s_Tools[s_CurrentTool].Action->setChecked(true);

		s_JustifiedSelectionTool.reset(new CSelectionTool);

		s_ActionHandles.SetRoot           = action_manager.NewAction(nullptr, "Set Root",                 ":/set_root.png",        QKeySequence(Qt::CTRL + Qt::Key_R), false, &s_Actions.SetRoot);
		s_ActionHandles.ShowJustified     = action_manager.NewAction(nullptr, "Show Justified Graph",     ":/show_jgraph.png",     QKeySequence(Qt::CTRL + Qt::Key_J), false, &s_Actions.ShowJustified);
		s_ActionHandles.GenerateJustified = action_manager.NewAction(nullptr, "Generate Justified Graph", ":/generate_jgraph.png", QKeySequence(Qt::CTRL + Qt::Key_G), false, &s_Actions.GenerateJustified);

		s_Actions.ShowJustified->setCheckable(true);
		s_Actions.ShowJustified->setChecked(false);
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
		s_Toolbar->addSeparator();
		s_Toolbar->addAction(s_Actions.SetRoot);
		s_Toolbar->addAction(s_Actions.ShowJustified);
		s_Toolbar->addAction(s_Actions.GenerateJustified);
		s_Toolbar->setVisible(false);

		// Visualization menu
		s_VisualizationActions.push_back(new QAction("Categories", main_window));
		s_VisualizationActions.push_back(new QAction("Integration", main_window));
		s_VisualizationActions.push_back(new QAction("Depth", main_window));
		s_VisualizationMenu = main_window->Menu("Visualize", &s_VisualizationMenuAction);
		for (size_t i = 0; i < s_VisualizationActions.size(); ++i)
		{
			auto* action = s_VisualizationActions[i];
			action->setCheckable(true);
			auto mode = (EVisualizationMode)i;
			connect(action, &QAction::triggered, [mode]() { CJassEditor::SetVisualizationMode(mode); });
			s_VisualizationMenu->addAction(action);
		}
		s_VisualizationMenuAction->setVisible(false);

		s_CategoryView = &main_window->CategoryView();

		s_AnalysisSpriteSet = std::make_unique<CPaletteSpriteSet>(SPECTRAL_PALETTE, qRgb(0xC0, 0xC0, 0xC0));
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
		m_SplitWidget->AddWidget(m_GraphWidget, 2);
		m_GraphWidget->setVisible(true);
		m_GraphWidget->SetDelegate(this);
		m_GraphWidget->EnableTooltips();
	
		m_GraphWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_GraphWidget, &CGraphWidget::customContextMenuRequested, this, &CJassEditor::OnCustomContextMenuRequested);

		auto graph_node_category_theme = std::make_shared<CGraphNodeCategoryTheme>(DataModel(), Categories(), m_CategorySpriteSet);

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
			auto node_layer = std::make_unique<CNodeGraphLayer>(*m_GraphWidget, *this, graph_node_category_theme);
			m_NodeGraphLayer = node_layer.get();
			m_GraphWidget->AppendLayer(std::move(node_layer));
		}

		{
			auto tool_layer = std::make_unique<CGraphToolLayer>(*m_GraphWidget, *this);
			m_GraphWidget->AppendLayer(std::move(tool_layer));
		}

		m_GraphWidget->addAction(qapp::s_StandardActions.SelectAll);


		m_JustifiedGraphWidget = new CGraphWidget(m_SplitWidget);
		m_SplitWidget->AddWidget(m_JustifiedGraphWidget, 1);
		m_JustifiedGraphWidget->setVisible(false);
		m_JustifiedGraphWidget->SetDelegate(this);
		m_JustifiedGraphWidget->EnableTooltips();

		{
			auto layer = std::make_unique<CJustifiedEdgeGraphLayer>(*m_JustifiedGraphWidget, DataModel(), SelectionModel());
			m_JustifiedGraphWidget->AppendLayer(std::move(layer));
		}

		{
			auto layer = std::make_unique<CJustifiedNodeGraphLayer>(*m_JustifiedGraphWidget, *this, graph_node_category_theme);
			m_JustifiedGraphWidget->AppendLayer(std::move(layer));
		}

		{
			auto layer = std::make_unique<CGraphToolLayer>(*m_JustifiedGraphWidget, *this);
			m_JustifiedGraphWidget->AppendLayer(std::move(layer));
		}

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

		ctx.Enable(s_ActionHandles.ShowJustified);

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
			ctx.Enable(s_ActionHandles.SetRoot);
			ctx.Enable(s_ActionHandles.GenerateJustified);
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
		else if (action_handle == s_ActionHandles.ShowJustified)
		{
			m_JustifiedGraphWidget->setVisible(s_Actions.ShowJustified->isChecked());
			return true;
		}
		else if (action_handle == s_ActionHandles.GenerateJustified)
		{
			GenerateJustifiedGraph();
			if (!m_JustifiedGraphWidget->isVisible())
			{
				m_JustifiedGraphWidget->setVisible(true);
				s_Actions.ShowJustified->setChecked(true);
			}
			return true;
		}
		else if (action_handle == s_ActionHandles.SetRoot)
		{
			SetSelectedNodeAsRoot();
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

		// Activate graph widget tool
		s_NodeTool->SetSpriteSet(m_CategorySpriteSet);
		s_Tools[s_CurrentTool].Tool.get()->Activate({ this, m_GraphWidget });
		m_GraphWidget->SetInputProcessor(s_Tools[s_CurrentTool].Tool.get());

		// Activate justified graph widget tool
		s_JustifiedSelectionTool->Activate({ this, m_JustifiedGraphWidget });
		m_JustifiedGraphWidget->SetInputProcessor(s_JustifiedSelectionTool.get());
		s_Actions.ShowJustified->setChecked(m_JustifiedGraphWidget->isVisible());

		// Hook up Category view
		s_CategoryView->SetCategories(&m_Document.Categories());
		connect(s_CategoryView, &CCategoryView::AddCategory, this, &CJassEditor::OnAddCategory);
		connect(s_CategoryView, &CCategoryView::RemoveCategories, this, &CJassEditor::OnRemoveCategories);
		connect(s_CategoryView, &CCategoryView::ModifyCategory, this, &CJassEditor::OnModifyCategory);

		// Visualization
		for (size_t i = 0; i < s_VisualizationActions.size(); ++i)
		{
			s_VisualizationActions[i]->setChecked((EVisualizationMode)i == m_VisualizationMode);
		}
		s_VisualizationMenuAction->setVisible(true);
	}

	void CJassEditor::OnDeactivate()
	{
		// Visualization
		s_VisualizationMenuAction->setVisible(false);

		// Disconnect Category view
		s_CategoryView->disconnect(this);
		s_CategoryView->SetCategories(nullptr);

		// Deactivate justified graph widget tool
		m_JustifiedGraphWidget->SetInputProcessor(nullptr);
		s_JustifiedSelectionTool->Deactivate();

		// Deactivate graph widget tool
		m_GraphWidget->SetInputProcessor(nullptr);
		s_Tools[s_CurrentTool].Tool.get()->Deactivate();

		s_Toolbar->setVisible(false);

		CEditor::OnDeactivate();
	}

	void CJassEditor::OnSaved()
	{
		m_CommandHistory->SetCleanAtCurrentPosition();
	}

	void CJassEditor::Export(QIODevice& out, const qapp::SDocumentTypeDesc& format)
	{
		ExportJassToSVG(out, m_Document, m_NodeGraphLayer ? m_NodeGraphLayer->Theme() : nullptr);
	}

	QString CJassEditor::ToolTipText(CGraphWidget& graph_widget, size_t layer_index, CGraphLayer::element_t element)
	{
		auto* layer = &graph_widget.Layer(layer_index);

		if (dynamic_cast<CNodeGraphLayer*>(layer) || dynamic_cast<CJustifiedNodeGraphLayer*>(layer))
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

	CAnalyses& CJassEditor::Analyses()
	{
		return *m_Analyses;
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

			contextMenu.addSeparator();

			if (SelectionModel().SelectedNodeCount() == 1)
			{
				contextMenu.addAction(s_Actions.SetRoot);
			}
		}

		contextMenu.exec(m_GraphWidget->mapToGlobal(pos));
	}

	void CJassEditor::OnNodesRemapped(const CGraphModel::const_node_indices_t& node_indices, const  CGraphModel::node_remap_table_t& remap_table)
	{
		const auto root_node_attribute_index = DataModel().FindAttribute(GRAPH_ATTTRIBUTE_ROOT_NODE);
		if (root_node_attribute_index != CGraphModel::NO_ATTRIBUTE)
		{
			const auto root_node_index = DataModel().AttributeValue(root_node_attribute_index).toInt();
			if (root_node_index >= 0 && root_node_attribute_index < remap_table.size())
			{
				const auto new_root_node_index = remap_table[root_node_index];
				DataModel().SetAttribute(
					root_node_attribute_index, 
					(CGraphModel::NO_NODE == new_root_node_index) ? (int)-1 : (int)new_root_node_index);
			}
		}

		UpdateAnalyses();
	}

	void CJassEditor::UpdateAnalyses()
	{
		m_Analyses->EnqueueUpdate(DataModel());
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

	void CJassEditor::SetSelectedNodeAsRoot()
	{
		// Get first selected node
		const auto new_root_node_index = SelectionModel().FirstSelected();
		if (CGraphModel::NO_NODE == new_root_node_index)
		{
			return;
		}
		const auto attribute_index = DataModel().FindAttribute(GRAPH_ATTTRIBUTE_ROOT_NODE);
		if (attribute_index == CGraphModel::NO_ATTRIBUTE)
		{
			return;
		}
		CommandHistory().NewCommand<CCmdSetGraphAttribute>(
			attribute_index,
			(int)new_root_node_index);
	}

	void CJassEditor::GenerateJustifiedGraph()
	{
		auto* jposition_attribute = TryGetJPositionNodeAttribute(DataModel());
		if (!jposition_attribute)
		{
			return;
		}

		const auto depth_metric_index = m_Analyses->FindMetricIndex("Depth");
		if (depth_metric_index < 0)
		{
			return;
		}

		std::vector<JPosition_NodeAttribute_t::value_t> jpositions(DataModel().NodeCount());

		jass::GenerateJustifiedGraph(DataModel(), SelectionModel().NodeMask(), m_Analyses->MetricValues(depth_metric_index), jpositions);
		
		// Create a mask for the nodes for which the j-position value changes, and pack the array
		// to only contain values for the canged ones.
		bitvec diff_mask;
		diff_mask.resize(DataModel().NodeCount());
		{
			size_t n = 0;
			for (size_t node_index = 0; node_index < jpositions.size(); ++node_index)
			{
				if (jposition_attribute->Value(node_index) != jpositions[node_index])
				{
					diff_mask.set(node_index);
					jpositions[n++] = jpositions[node_index];
				}
			}
			jpositions.resize(n);
		}

		CommandHistory().NewCommand<CCmdSetNodeAttributes<JPosition_NodeAttribute_t::value_t>>(jposition_attribute, diff_mask, jpositions);
	}

	void CJassEditor::SetVisualizationMode(EVisualizationMode mode)
	{
		auto* editor = dynamic_cast<CJassEditor*>(s_Workbench->CurrentEditor());
		if (!editor || editor->m_VisualizationMode == mode)
		{
			return;
		}

		ASSERT(editor->m_NodeGraphLayer);

		switch (mode)
		{
		case EVisualizationMode::Categories:
			editor->m_NodeGraphLayer->SetTheme(std::make_shared<CGraphNodeCategoryTheme>(editor->DataModel(), editor->Categories(), editor->m_CategorySpriteSet));
			break;
		case EVisualizationMode::Integration:
			{
				auto analysis_theme = std::make_shared<CGraphNodeAnalysisTheme>(editor->DataModel(), editor->Analyses(), editor->Categories(), *s_AnalysisSpriteSet);
				analysis_theme->SetMetric("Integration", false);
				editor->m_NodeGraphLayer->SetTheme(analysis_theme);
		}
			break;
		case EVisualizationMode::Depth:
			{
				auto analysis_theme = std::make_shared<CGraphNodeAnalysisTheme>(editor->DataModel(), editor->Analyses(), editor->Categories(), *s_AnalysisSpriteSet);
				analysis_theme->SetMetric("Depth", true);
				editor->m_NodeGraphLayer->SetTheme(analysis_theme);
		}
			break;
		}

		s_VisualizationActions[(size_t)editor->m_VisualizationMode]->setChecked(false);
		s_VisualizationActions[(size_t)mode]->setChecked(true);
		
		editor->m_VisualizationMode = mode;
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
			new_tool->Activate({ jass_editor, jass_editor->m_GraphWidget });
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