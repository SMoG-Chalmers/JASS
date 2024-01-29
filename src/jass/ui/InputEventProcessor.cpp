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

#include "InputEventProcessor.h"

namespace jass
{
	void CInputEventProcessor::ProcessInputEvent(QEvent& event)
	{
		switch (event.type())
		{
		case QEvent::Leave:
			leaveEvent(event);
			break;
		case QEvent::MouseMove:
			mouseMoveEvent((QMouseEvent&)event);
			break;
		case QEvent::MouseButtonPress:
			mousePressEvent((QMouseEvent&)event);
			break;
		case QEvent::MouseButtonRelease:
			mouseReleaseEvent((QMouseEvent&)event);
			break;
		case QEvent::Wheel:
			wheelEvent((QWheelEvent&)event);
			break;
		case QEvent::KeyPress:
			keyPressEvent((QKeyEvent&)event);
			break;
		case QEvent::KeyRelease:
			keyReleaseEvent((QKeyEvent&)event);
			break;
		}
	}
}