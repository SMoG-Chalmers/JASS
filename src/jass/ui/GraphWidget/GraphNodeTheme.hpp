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

#include "GraphWidget.hpp"

namespace jass
{
	class CGraphNodeTheme: public QObject
	{
		Q_OBJECT
	public:
		using element_t = CGraphLayer::element_t;
		enum class EStyle
		{
			Normal,
			Selected,
			Hilighted,
			_COUNT
		};

		virtual QRect ElementLocalRect(element_t element, EStyle style) const = 0;
		virtual void  DrawElement(element_t element, EStyle style, const QPoint& pos, QPainter& painter) const = 0;
		virtual QRgb  ElementColor(element_t element) const = 0;  // Not sure about this one. Currently only needed for SVG export.

	Q_SIGNALS:
		void Updated();
	};
}