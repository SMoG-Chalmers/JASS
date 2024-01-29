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

#include <jass/Shape.h>

class QPixmap;

namespace jass
{
	struct SNodeSpriteDesc
	{
		EShape  Shape;
		float   Radius;
		float   OutlineWidth;
		float   OutlineWidth2 = 0;
		QPointF ShadowOffset;
		float   ShadowBlurRadius;
		QRgb    FillColor;
		QRgb    OutlineColor;
		QRgb    OutlineColor2;
		QRgb    ShadowColor;
		QPoint  Offset = { 0,0 };
	};

	void CreateNodeSprite(const SNodeSpriteDesc& desc, QPixmap& out_pixmap, QPoint& out_origin);
}