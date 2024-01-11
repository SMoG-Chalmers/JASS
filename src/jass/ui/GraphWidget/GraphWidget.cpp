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

#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>

#include <jass/utils/bitvec.h>
#include <jass/Debug.h>

#include "GraphWidget.hpp"

namespace jass
{
	CGraphWidget::CGraphWidget(QWidget* parent)
	{
		setMouseTracking(true);

		// Set the focus policy to accept keyboard focus
		setFocusPolicy(Qt::StrongFocus);

		//SetScreenToModelScale(2);
		//SetScreenTranslation(QPoint(300, 300));
	}

	CGraphWidget::~CGraphWidget()
	{
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
		ASSERT(!layer->m_Widget);
		layer->m_Widget = this;
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
		for (size_t layer_index = 0; layer_index < m_Layers.size(); ++layer_index)
		{
			m_Layers[layer_index]->Paint(painter, event->rect());
		}
	}

	bool CGraphWidget::event(QEvent* event)
	{
		if (m_InputProcessor && CInputEventProcessor::IsInputEvent(*event))
		{
			m_InputProcessor->ProcessInputEvent(*event);
		}
		
		return QWidget::event(event);
	}
}

#include <moc_GraphWidget.cpp>