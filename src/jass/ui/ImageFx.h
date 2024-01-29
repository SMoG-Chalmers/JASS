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

#include <QtGui/qrgb.h>

class QImage;

namespace jass
{
	QRgb Blend(const QRgb& c0, const QRgb& c1, uint8_t alpha);
		
	QRgb AddOver(QRgb over, QRgb under);
	
	void DropShadow(QImage& img, QColor color, unsigned int radius, int offs_x, int offs_y, float sigma_range);
}