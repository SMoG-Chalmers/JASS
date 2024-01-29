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

#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include "SplitWidget.hpp"

namespace jass
{
    const int CSplitWidget::MIN_WIDGET_SIZE = 10;

	inline size_t CSplitWidget::SplitterCount() const
	{
		return m_Splitters.size();
	}
	
	inline QRect CSplitWidget::SplitterRect(size_t index) const
	{
		const auto& splitter = m_Splitters[index];
		return QRect(splitter.Position - m_SeparatorSize / 2, 0, m_SeparatorSize, height());
	}

	inline int CSplitWidget::SpaceBeforeSplitter(size_t splitter_index) const
	{
		const auto& splitter = m_Splitters[splitter_index];
		return (splitter_index == 0) ? splitter.Position - m_SeparatorSize / 2 : splitter.Position - m_Splitters[splitter_index - 1].Position - m_SeparatorSize;
	}

	inline int CSplitWidget::SpaceAfterSplitter(size_t splitter_index) const
	{
		const auto& splitter = m_Splitters[splitter_index];
		return (splitter_index < m_Splitters.size() - 1) ? m_Splitters[splitter_index + 1].Position - splitter.Position - m_SeparatorSize : width() - splitter.Position - m_SeparatorSize / 2;
	}

	CSplitWidget::CSplitWidget(QWidget* parent)
		: QWidget(parent)
	{
		setAutoFillBackground(false);
		setMouseTracking(true);
		setCursor(Qt::SizeHorCursor);
	}

	void CSplitWidget::AddWidget(QWidget* widget, int weight)
	{
		SWidget w;
		w.Widget = widget;
		w.Weight = weight;
		m_Widgets.push_back(w);
		widget->setParent(this);
		widget->installEventFilter(this);
		UpdateLayout();
	}

	bool CSplitWidget::eventFilter(QObject* watched, QEvent* event)
	{
		switch (event->type())
		{
		case QEvent::Hide:
		case QEvent::Show:
			if (auto* widget = dynamic_cast<QWidget*>(watched))
			{
				if (FindWidget(widget) >= 0)
				{
					UpdateLayout();
				}
			}
			break;
		}

		// Call the base class implementation
		return QWidget::eventFilter(watched, event);
	}

	void CSplitWidget::resizeEvent(QResizeEvent* event)
	{
		UpdateLayout();
	}

	void CSplitWidget::paintEvent(QPaintEvent* event)
	{
		QPainter painter(this);

		const auto& rcClip = event->rect();

		painter.setRenderHint(QPainter::Antialiasing, true);

		for (size_t splitter_index = 0; splitter_index < SplitterCount(); ++splitter_index)
		{
			const auto rcSplitter = SplitterRect(splitter_index);
			if (!rcClip.intersects(rcSplitter))
			{
				continue;
			}
			DrawSplitter(painter, rcSplitter);
		}
	}

	void CSplitWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (IsResizing())
		{
			auto& splitter = m_Splitters[m_CurrentSplitterIndex];
			const int lastPosition = splitter.Position;
			// Try out the desired position
			splitter.Position = event->pos().x() - m_MouseOffset;
			const auto spaceAfter = SpaceAfterSplitter(m_CurrentSplitterIndex);
			if (spaceAfter < MIN_WIDGET_SIZE)
			{
				splitter.Position += spaceAfter - MIN_WIDGET_SIZE;
			}
			const auto spaceBefore = SpaceBeforeSplitter(m_CurrentSplitterIndex);
			if (spaceBefore < MIN_WIDGET_SIZE)
			{
				splitter.Position += MIN_WIDGET_SIZE - spaceBefore;
			}
			if (lastPosition != splitter.Position)
			{
				auto& w0 = m_Widgets[splitter.Widget0];
				w0.Weight = SpaceBeforeSplitter(m_CurrentSplitterIndex);
				w0.Widget->setGeometry(splitter.Position - m_SeparatorSize / 2 - w0.Weight, 0, w0.Weight, height());
				auto& w1 = m_Widgets[splitter.Widget1];
				w1.Weight = SpaceAfterSplitter(m_CurrentSplitterIndex);
				w1.Widget->setGeometry(splitter.Position + m_SeparatorSize / 2, 0, w1.Weight, height());
				update(SplitterRect(m_CurrentSplitterIndex));
			}
		}

		QWidget::mousePressEvent(event);
	}

	void CSplitWidget::mousePressEvent(QMouseEvent* event)
	{
		if (!IsResizing() && event->buttons() == Qt::LeftButton && event->modifiers() == Qt::NoModifier)
		{
			for (size_t splitter_index = 0; splitter_index < SplitterCount(); ++splitter_index)
			{
				const auto& splitter = m_Splitters[splitter_index];
				const int offset = event->pos().x() - splitter.Position;
				if (std::abs(offset) <= m_SeparatorSize / 2)
				{
					m_CurrentSplitterIndex = (int)splitter_index;
					m_MouseOffset = offset;
					m_Widgets[splitter.Widget0].Weight = SpaceBeforeSplitter(splitter_index);
					m_Widgets[splitter.Widget1].Weight = SpaceAfterSplitter(splitter_index);
					break;
				}
			}
		}

		QWidget::mousePressEvent(event);
	}

	void CSplitWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		if (IsResizing() && event->button() == Qt::LeftButton)
		{
			m_CurrentSplitterIndex = -1;
		}

		QWidget::mousePressEvent(event);
	}

	void CSplitWidget::DrawSplitter(QPainter& painter, const QRect& rc)
	{
		// Background
		painter.setPen(Qt::NoPen);
		painter.setBrush(Qt::white);
		painter.drawRect(rc);

		// Line
		const float half_width = .5f * rc.width();
		const float centerX = (float)rc.left() + half_width;
		QPen pen(Qt::lightGray);
		pen.setWidth(3);
		pen.setCapStyle(Qt::RoundCap);
		painter.setPen(pen);
		painter.drawLine(QPointF(centerX, rc.top() + half_width), QPointF(centerX, rc.bottom() - half_width));
	}

	void CSplitWidget::UpdateLayout()
	{
		int visibleWeightSum = 0;
		int visibleCount = 0;
		for (const auto& w : m_Widgets)
		{
			if (!w.Widget->testAttribute(Qt::WA_WState_Visible))
			{
				continue;
			}
			visibleWeightSum += w.Weight;
			++visibleCount;
		}

		const auto separatorCount = visibleCount - 1;
		const int stretchableSize = width() - separatorCount * m_SeparatorSize;

		m_Splitters.clear();
		{
			int n = 0;
			int at = 0;
			for (auto& w : m_Widgets)
			{
				if (!w.Widget->testAttribute(Qt::WA_WState_Visible))
				{
					continue;
				}

				if (!m_Splitters.empty())
				{
					m_Splitters.back().Widget1 = (int)(&w - m_Widgets.data());
				}

				int size;
				if (n < visibleCount - 1)
				{
					size = ((float)w.Weight / visibleWeightSum) * stretchableSize;
				}
				else
				{
					size = width() - at;
				}
				size = std::max(MIN_WIDGET_SIZE, size);

				w.Widget->setGeometry(at, 0, size, height());

				at += size;

				if (n < visibleCount - 1)
				{
					SSplitter splitter;
					splitter.Position = at + m_SeparatorSize / 2;
					splitter.Widget0 = (int)(&w - m_Widgets.data());
					splitter.Widget1 = -1;
					m_Splitters.push_back(splitter);
					update(SplitterRect(m_Splitters.size() - 1));
				}
				
				at += m_SeparatorSize;

				++n;
			}
		}
	}

	int CSplitWidget::FindWidget(QWidget* widget) const
	{
		for (auto& w : m_Widgets)
		{
			if (w.Widget == widget)
			{
				return (int)(&w - m_Widgets.data());
			}
		}
		return -1;
	}
}

#include <moc_SplitWidget.cpp>
