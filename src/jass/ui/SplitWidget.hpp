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

#include <QtWidgets/qwidget.h>

namespace jass
{
	class CSplitWidget : public QWidget
	{
		Q_OBJECT
	public:
		CSplitWidget(QWidget* parent);

		void AddWidget(QWidget* widget, int weight = 1);

		// Events
		bool eventFilter(QObject* watched, QEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		void paintEvent(QPaintEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;

	protected:
		virtual void DrawSplitter(QPainter& painter, const QRect& rc);

	private:
		void UpdateLayout();
		int FindWidget(QWidget* widget) const;
		inline size_t SplitterCount() const;
		inline QRect SplitterRect(size_t index) const;
		inline int SpaceBeforeSplitter(size_t splitter_index) const;
		inline int SpaceAfterSplitter(size_t splitter_index) const;
		inline bool IsResizing() const { return m_CurrentSplitterIndex != -1; }

		static const int MIN_WIDGET_SIZE;

		struct SWidget
		{
			QWidget* Widget = nullptr;
			int Weight = 1;
		};

		struct SSplitter
		{
			int Position;
			int Widget0;
			int Widget1;
		};

		int m_CurrentSplitterIndex = -1;
		int m_MouseOffset = 0;
		int m_SeparatorSize = 15;  // Should be odd
		std::vector<SWidget> m_Widgets;
		std::vector<SSplitter> m_Splitters;
	};
}
