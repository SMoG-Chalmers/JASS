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

#include <QtGui/qevent.h>

namespace jass
{
	class CInputEventProcessor
	{
	public:
		inline static bool IsInputEvent(const QEvent& event);
		
		void ProcessInputEvent(QEvent& event);

		virtual void mouseMoveEvent(QMouseEvent& event) {}
		virtual void leaveEvent(QEvent& event) {}
		virtual void mousePressEvent(QMouseEvent& event) {}
		virtual void mouseReleaseEvent(QMouseEvent& event) {}
		virtual void wheelEvent(QWheelEvent& event) {}
		virtual void keyPressEvent(QKeyEvent& event) {}
		virtual void keyReleaseEvent(QKeyEvent& event) {}

	private:
		static const uint32_t TOOL_EVENT_ID_MASK =
			(1 << QEvent::Leave) |
			(1 << QEvent::KeyPress) |
			(1 << QEvent::KeyRelease) |
			(1 << QEvent::MouseButtonPress) |
			(1 << QEvent::MouseButtonRelease) |
			(1 << QEvent::MouseMove) |
			(1 << QEvent::Wheel);
	};

	inline bool CInputEventProcessor::IsInputEvent(const QEvent& event)
	{
		return 0 != ((uint32_t)(1 << event.type()) & TOOL_EVENT_ID_MASK);
	}
}