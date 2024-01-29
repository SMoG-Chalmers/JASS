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

#include <QtCore/qsettings.h>

namespace jass
{
	class CSettings: public QObject
	{
		Q_OBJECT
	public:
		static const QString UI_SCALE;

		CSettings(QSettings& qsettings);

		void setValue(const QString& key, const QVariant& value);
		QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

	Q_SIGNALS:
		void Changed(const QString& key, const QVariant& newValue);

	private:
		QSettings& m_QSettings;
	};
}
