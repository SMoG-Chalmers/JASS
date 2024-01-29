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

#include <QtGui/qpainter.h>
#include "ImageGraphLayer.hpp"

namespace jass
{
	void CImageGraphLayer::SetImage(QPixmap&& image)
	{
		m_Image = std::move(image);
		Update();
	}

	void CImageGraphLayer::Paint(QPainter& painter, const QRect& rc)
	{
		painter.setRenderHint(QPainter::SmoothPixmapTransform);
		painter.drawPixmap(QRectF(rc), m_Image, GraphWidget().ModelFromScreen(rc));
	}
}