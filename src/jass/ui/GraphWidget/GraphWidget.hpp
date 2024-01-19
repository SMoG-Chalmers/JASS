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

#include <stack>
#include <vector>
#include <QtWidgets/qwidget.h>
#include <jass/ui/InputEventProcessor.h>
#include <jass/utils/bitvec.h>

class QTimer;
class QToolTip;

namespace jass
{
	class bitvec;
	class CGraphWidget;

	class CGraphLayerContext
	{
	public:
		CGraphLayerContext(QWidget& widget) : m_Widget(widget) {}

		inline void update(const QRect& r) { m_Widget.update(r); }

	private:
		QWidget& m_Widget;
	};

	class CGraphLayer
	{
	public:
		typedef uintptr_t element_t;
		static const element_t NO_ELEMENT = (element_t)-1;

		CGraphLayer(CGraphWidget& graphWidget) : m_GraphWidget(graphWidget) {}
		virtual ~CGraphLayer() {}

		virtual void Paint(QPainter& painter, const QRect& rc) {}
		virtual element_t HitTest(const QPoint& pt) { return NO_ELEMENT; }
		virtual bool RangedHitTest(const QRect& rc, bitvec& out_hit_elements) const { return false; }
		virtual void SetHilighted(element_t edge, bool hilighted) {}
		virtual void GetSelection(bitvec& out_selection_mask) const { out_selection_mask.clear(); }
		virtual void SetSelection(const bitvec& selection_mask) const { }
		virtual void OnViewChanged(const QRect& rc, float screen_to_model_scale) {}

		inline void Update();
		inline void Update(const QRect& rc);

		inline CGraphWidget& GraphWidget() { return m_GraphWidget; }
		inline const CGraphWidget& GraphWidget() const { return m_GraphWidget; }

	private:
		CGraphWidget& m_GraphWidget;
	};

	class IGraphWidgetDelegate
	{
	public:
		virtual QString ToolTipText(size_t layer_index, CGraphLayer::element_t element) = 0;
	};

	class CGraphWidget : public QWidget
	{
		Q_OBJECT
	public:
		typedef CGraphLayer::element_t element_t;

		CGraphWidget(QWidget* parent);
		~CGraphWidget();

		void SetDelegate(IGraphWidgetDelegate* dlgt);

		void EnableTooltips(bool enable = true);

		size_t LayerCount() const;

		bool HitTest(const QPoint& pt, size_t& out_layer_index, element_t& out_element) const;
		bool RangedHitTest(const QRect& rc, size_t& out_layer_index, bitvec& out_hit_elements) const;

		void DeselectAll();

		CGraphLayer& Layer(size_t index);
		const CGraphLayer& Layer(size_t index) const;

		void InsertLayer(size_t index, std::unique_ptr<CGraphLayer>&& layer);

		void AppendLayer(std::unique_ptr<CGraphLayer>&& layer);

		std::unique_ptr<CGraphLayer> RemoveLayer(size_t index);

		inline const QPoint& ScreenTranslation() const;
		inline const QPointF& ModelTranslation() const;
		void SetScreenTranslation(const QPoint& translation);

		inline float ScreenToModelScale() const;
		inline float ModelToScreenScale() const;
		void SetScreenToModelScale(float scale);  // 2 --> 200% zoom

		inline QPointF ScreenFromModel(const QPointF& pt_model) const;
		inline QPointF ModelFromScreen(const QPoint& pt_screen) const;
		inline QPointF ModelFromScreen(const QPointF& pt_screen) const;
		inline QRectF ModelFromScreen(const QRect& rc_screen) const;
		inline QRectF ModelFromScreen(const QRectF& rc_screen) const;

		void SetInputProcessor(CInputEventProcessor* input_processor);

		// Events
		bool event(QEvent* ev) override;
		void paintEvent(QPaintEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

	private Q_SLOTS:
		void OnTooltipTimer();

	private:
		enum class EState
		{
			Idle,
			Panning,
		};

		void NotifyViewChanged();

		void UpdateTooltip(const QMouseEvent& event);

		void CancelTooltip();

		bool TryShowTooltip(const QPoint& pos);
		
		IGraphWidgetDelegate* m_Delegate = nullptr;
		CInputEventProcessor* m_InputProcessor = nullptr;
		std::vector<std::unique_ptr<CGraphLayer>> m_Layers;
		uint8_t m_ZoomLevel;
		QPoint m_ScreenTranslation;
		QPointF m_ModelTranslation;
		float m_ScreenToModelScale = 1;
		float m_ModelToScreenScale = 1;
		EState m_State = EState::Idle;
		QPoint m_MouseRef;

		struct SToolTip
		{
			QTimer* Timer = nullptr;
			QToolTip* ToolTip = nullptr;
			QPoint MousePos;
			size_t HoverLayer = (size_t)-1;
			element_t HoverElement;
		} m_ToolTip;
	};

	inline const QPoint& CGraphWidget::ScreenTranslation() const
	{
		return m_ScreenTranslation;
	}

	inline const QPointF& CGraphWidget::ModelTranslation() const
	{
		return m_ModelTranslation;
	}

	inline float CGraphWidget::ScreenToModelScale() const
	{
		return m_ScreenToModelScale;
	}

	inline float CGraphWidget::ModelToScreenScale() const
	{
		return m_ModelToScreenScale;
	}

	inline QPointF CGraphWidget::ScreenFromModel(const QPointF& pt_model) const
	{
		return (pt_model + m_ModelTranslation) * ModelToScreenScale();
	}

	inline QPointF CGraphWidget::ModelFromScreen(const QPoint& pt_screen) const
	{
		return QPointF(pt_screen - m_ScreenTranslation) * m_ScreenToModelScale;
	}

	inline QPointF CGraphWidget::ModelFromScreen(const QPointF& pt_screen) const
	{
		return (pt_screen * m_ScreenToModelScale) - m_ModelTranslation;
	}

	inline QRectF CGraphWidget::ModelFromScreen(const QRect& rc_screen) const
	{
		return QRectF(ModelFromScreen(rc_screen.topLeft()), QSizeF(ScreenToModelScale() * rc_screen.width(), ScreenToModelScale() * rc_screen.height()));
	}

	inline QRectF CGraphWidget::ModelFromScreen(const QRectF& rc_screen) const
	{
		return QRectF(ModelFromScreen(rc_screen.topLeft()), QSizeF(ScreenToModelScale() * rc_screen.width(), ScreenToModelScale() * rc_screen.height()));
	}

	// CGraphLayer Implementation

	inline void CGraphLayer::Update() { m_GraphWidget.update(); }
	inline void CGraphLayer::Update(const QRect& rc) { m_GraphWidget.update(rc); }
}