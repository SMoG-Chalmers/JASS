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

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>

#include <qapplib/widgets/ColorWidget.hpp>

#include "CategoryDialog.hpp"

namespace jass
{
	CCategoryDialog::CCategoryDialog(QWidget* parent)
	{
		auto* vlayout = new QVBoxLayout(this);
		//vlayout->setMargin(10);

		{
			auto* gridLayout = new QGridLayout();
			gridLayout->setMargin(0);
			int y = 0;
			m_NameLineEdit = new QLineEdit(this);
			gridLayout->addWidget(new QLabel("Name", this), y, 0);
			gridLayout->addWidget(m_NameLineEdit, y, 2);
			++y;
			m_ColorWidget = new qapp::CColorWidget(this);
			gridLayout->addWidget(new QLabel("Color", this), y, 0);
			gridLayout->addWidget(m_ColorWidget, y, 2);
			++y;
			m_ShapeComboBox = new QComboBox(this);
			for (int i = 0; i < (int)EShape::_COUNT; ++i)
			{
				m_ShapeComboBox->addItem(
					QIcon(QPixmap::fromImage(CreateShapeImage((EShape)i, qRgb(0,0,0), 2, QPoint(1, 1), 1, 16))),
					ShapeToString((EShape)i));
			}
			gridLayout->addWidget(new QLabel("Shape", this), y, 0);
			gridLayout->addWidget(m_ShapeComboBox, y, 2);
			++y;
			vlayout->addLayout(gridLayout);
		}

		vlayout->addSpacing(10);

		{
			auto* hlayout = new QHBoxLayout(this);
			hlayout->setMargin(0);
			//hlayout->setSpacing();
			auto* ok_button = new QPushButton("OK", this);
			ok_button->setDefault(true);
			connect(ok_button, &QPushButton::clicked, this, &CCategoryDialog::accept);
			hlayout->addWidget(ok_button);
			auto* cancel_button = new QPushButton("Cancel", this);
			connect(cancel_button, &QPushButton::clicked, this, &CCategoryDialog::reject);
			hlayout->addWidget(cancel_button);
			vlayout->addLayout(hlayout);
		}

		setLayout(vlayout);
	}

	void CCategoryDialog::SetName(const QString& name)
	{
		m_NameLineEdit->setText(name);
	}

	QString CCategoryDialog::Name() const
	{
		return m_NameLineEdit->text();
	}

	void CCategoryDialog::SetColor(QRgb color)
	{
		m_ColorWidget->setValue(QColor::fromRgba(color));
	}

	QRgb CCategoryDialog::Color() const
	{
		return m_ColorWidget->value().rgba();
	}

	void CCategoryDialog::SetShape(EShape shape)
	{
		m_ShapeComboBox->setCurrentIndex((int)shape);
	}

	EShape CCategoryDialog::Shape() const
	{
		return (EShape)m_ShapeComboBox->currentIndex();
	}
}

#include <moc_CategoryDialog.cpp>