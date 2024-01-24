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

#include <QtCore/qtimer.h>
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qtooltip.h>

#include <jass/math/Geometry.h>
#include <jass/utils/bitvec.h>
#include <jass/Debug.h>

#include "GraphWidget.hpp"

namespace jass
{
	const int MOUSE_WHEEL_NOTCH_SIZE = 120;  // Is there no better way of doing this?
	const int TOOLTIP_DELAY_MSEC = 500;

	static const uint8_t DEFAULT_ZOOM_LEVEL = 9;
	static const float s_ZoomLevels[] =
	{
		500,
		400,
		300,
		250,
		200,
		175,
		150,
		125,
		110,
		100,
		90,
		80,
		200.0f / 3,
		50,
		100.f / 3,
		25,
	};

	CGraphWidget::CGraphWidget(QWidget* parent)
		: m_ZoomLevel(DEFAULT_ZOOM_LEVEL)
	{
		setMouseTracking(true);

		// Set the focus policy to accept keyboard focus
		setFocusPolicy(Qt::StrongFocus);

		SetScreenToModelScale(s_ZoomLevels[m_ZoomLevel] * .01f);

		setAutoFillBackground(false);

		setCursor(Qt::ArrowCursor);
	}

	CGraphWidget::~CGraphWidget()
	{
	}

	void CGraphWidget::SetDelegate(IGraphWidgetDelegate* dlgt)
	{
		m_Delegate = dlgt;
	}

	void CGraphWidget::EnableTooltips(bool enable)
	{
		if (enable)
		{
			if (!m_ToolTip.Timer)
			{
				m_ToolTip.Timer = new QTimer(this);
				m_ToolTip.Timer->setSingleShot(true);
				connect(m_ToolTip.Timer, &QTimer::timeout, this, &CGraphWidget::OnTooltipTimer);
			}
		}
		else
		{
			if (m_ToolTip.Timer)
			{
				delete m_ToolTip.Timer;
				m_ToolTip.Timer = nullptr;
			}
		}
	}

	size_t CGraphWidget::LayerCount() const
	{
		return m_Layers.size();
	}

	bool CGraphWidget::HitTest(const QPoint& pt, size_t& out_layer_index, element_t& out_element) const
	{
		for (size_t layer_index = m_Layers.size() - 1; layer_index < m_Layers.size(); --layer_index)
		{
			auto& layer = *m_Layers[layer_index].get();
			const auto element = layer.HitTest(pt);
			if (element != CGraphLayer::NO_ELEMENT)
			{
				out_layer_index = layer_index;
				out_element = element;
				return true;
			}
		}
		return false;
	}

	bool CGraphWidget::RangedHitTest(const QRect& rc, size_t& out_layer_index, bitvec& out_hit_elements) const
	{
		for (size_t layer_index = m_Layers.size() - 1; layer_index < m_Layers.size(); --layer_index)
		{
			auto& layer = *m_Layers[layer_index].get();
			if (layer.RangedHitTest(rc, out_hit_elements))
			{
				out_layer_index = layer_index;
				return true;
			}
		}
		return false;
	}

	void CGraphWidget::DeselectAll()
	{
		for (auto& layer : m_Layers)
		{
			layer->SetSelection(bitvec());
		}
	}

	CGraphLayer& CGraphWidget::Layer(size_t index)
	{
		return *m_Layers[index];
	}

	const CGraphLayer& CGraphWidget::Layer(size_t index) const
	{
		return *m_Layers[index];
	}

	void CGraphWidget::InsertLayer(size_t index, std::unique_ptr<CGraphLayer>&& layer)
	{
		m_Layers.insert(m_Layers.begin() + index, std::move(layer));
	}

	void CGraphWidget::AppendLayer(std::unique_ptr<CGraphLayer>&& layer)
	{
		InsertLayer(m_Layers.size(), std::move(layer));
	}

	std::unique_ptr<CGraphLayer> CGraphWidget::RemoveLayer(size_t index)
	{
		auto it = m_Layers.begin() + index;
		auto layer = std::move(*it);
		m_Layers.erase(it);
		return std::move(layer);
	}

	void CGraphWidget::SetScreenTranslation(const QPoint& translation)
	{
		m_ScreenTranslation = translation;
		m_ModelTranslation = QPointF(translation) * ScreenToModelScale();
	}

	void CGraphWidget::SetScreenToModelScale(float scale)
	{
		m_ScreenToModelScale = scale;
		m_ModelToScreenScale = 1.0f / scale;
	}

	void CGraphWidget::SetInputProcessor(CInputEventProcessor* input_processor)
	{
		m_InputProcessor = input_processor;
	}

	void CGraphWidget::paintEvent(QPaintEvent* event)
	{
		QPainter painter(this);

		// Paint background
		painter.setPen(Qt::NoPen);
		painter.setBrush(Qt::white);
		painter.drawRect(rect());

		for (size_t layer_index = 0; layer_index < m_Layers.size(); ++layer_index)
		{
			m_Layers[layer_index]->Paint(painter, event->rect());
		}
	}

	bool CGraphWidget::event(QEvent* event)
	{
		if (CInputEventProcessor::IsInputEvent(*event))
		{
			switch (event->type())
			{
			case QEvent::MouseButtonPress:
				CancelTooltip();
				break;
			case QEvent::Wheel:
				{
					auto& mouseEvent = (QWheelEvent&)*event;
					if (mouseEvent.modifiers() == Qt::ControlModifier && mouseEvent.buttons() == Qt::NoButton)
					{
						const int delta_steps = mouseEvent.angleDelta().y() / MOUSE_WHEEL_NOTCH_SIZE;
						const auto new_zoom_level = (uint8_t)std::min(std::max(0, (int)m_ZoomLevel + delta_steps), (int)std::size(s_ZoomLevels) - 1);
						if (new_zoom_level != m_ZoomLevel)
						{
							m_ZoomLevel = new_zoom_level;
							const auto mouse_pos_model = ModelFromScreen(mouseEvent.position());
							SetScreenToModelScale(s_ZoomLevels[m_ZoomLevel] * .01f);

							const auto model_space_delta = ModelFromScreen(mouseEvent.position()) - mouse_pos_model;
							const auto screen_space_translation_f = (ModelTranslation() + model_space_delta) * ModelToScreenScale();
							SetScreenTranslation(QPointFromRoundedQPointF(screen_space_translation_f));

							NotifyViewChanged();

							update();
						}
					}
				}
				break;
			}

			switch (m_State)
			{
			case EState::Idle:
				if (event->type() == QEvent::MouseButtonPress)
				{
					auto& mouseEvent = (QMouseEvent&)*event;
					if (mouseEvent.buttons() == Qt::MiddleButton && mouseEvent.modifiers() == Qt::NoModifier)  // Only middle mouse button, no modifier
					{
						m_MouseRef = mouseEvent.pos();
						m_State = EState::Panning;
					}
				}
				break;
			case EState::Panning:
				if (event->type() == QEvent::MouseButtonRelease)
				{
					m_State = EState::Idle;
				}
				else if (event->type() == QEvent::MouseMove)
				{
					auto& mouseEvent = (QMouseEvent&)*event;
					const auto delta = mouseEvent.pos() - m_MouseRef;
					m_MouseRef = mouseEvent.pos();
					SetScreenTranslation(m_ScreenTranslation + delta);
					update();
				}
				break;
			}

			if (EState::Idle == m_State && m_InputProcessor)
			{
				m_InputProcessor->ProcessInputEvent(*event);
			}
		}
		
		return QWidget::event(event);
	}

	void CGraphWidget::mouseMoveEvent(QMouseEvent* event)
	{
		UpdateTooltip(*event);
	}

	void CGraphWidget::OnTooltipTimer()
	{
		TryShowTooltip(m_ToolTip.MousePos);

	}

	void CGraphWidget::NotifyViewChanged()
	{
		for (auto& layer : m_Layers)
		{
			layer->OnViewChanged(rect(), ScreenToModelScale());
		}
	}

	void CGraphWidget::UpdateTooltip(const QMouseEvent& event)
	{
		if (!m_ToolTip.Timer)
		{
			return;
		}

		if (event.buttons() != Qt::NoButton)
		{
			CancelTooltip();
			return;
		}

		m_ToolTip.MousePos = event.pos();

		size_t layer_index = (size_t)-1;
		element_t layer_element = CGraphLayer::NO_ELEMENT;
		HitTest(event.pos(), layer_index, layer_element);
		if (m_ToolTip.HoverLayer == layer_index && m_ToolTip.HoverElement == layer_element)
		{
			return;
		}

		if (QToolTip::isVisible())
		{
			QToolTip::hideText();
		}

		m_ToolTip.HoverLayer = layer_index;
		m_ToolTip.HoverElement = layer_element;

		if (CGraphLayer::NO_ELEMENT == m_ToolTip.HoverElement)
		{
			m_ToolTip.Timer->stop();
			return;
		}

		if (!QToolTip::isVisible())
		{
			m_ToolTip.Timer->start(TOOLTIP_DELAY_MSEC);
			return;
		}

		TryShowTooltip(event.pos());
	}

	void CGraphWidget::CancelTooltip()
	{
		if (QToolTip::isVisible())
		{
			QToolTip::hideText();
		}
		if (m_ToolTip.Timer)
		{
			m_ToolTip.Timer->stop();
		}
		m_ToolTip.HoverLayer = (size_t)-1;
		m_ToolTip.HoverElement = CGraphLayer::NO_ELEMENT;
	}

	bool CGraphWidget::TryShowTooltip(const QPoint& pos)
	{
		if (!m_Delegate)
		{
			return false;
		}

		size_t layer_index;
		element_t layer_element;
		if (!HitTest(pos, layer_index, layer_element))
		{
			return false;
		}

		m_ToolTip.HoverLayer = layer_index;
		m_ToolTip.HoverElement = layer_element;

		QString text = m_Delegate->ToolTipText(*this, layer_index, layer_element);
		if (text.isEmpty())
		{
			return false;
		}

		// HACK: Without this hack, the tooltip position won't update if text is the same as last time
		static bool every_other = false;
		every_other = !every_other;
		if (every_other)
		{
			text += " ";
		}

		QToolTip::showText(mapToGlobal(pos), text, this);
		
		return true;
	}
}

#include <moc_GraphWidget.cpp>