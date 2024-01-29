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

#include "Settings.hpp"

namespace jass
{
	const QString CSettings::UI_SCALE = "ui/scale";

	CSettings::CSettings(QSettings& qsettings)
		: m_QSettings(qsettings)
	{
	}

	void CSettings::setValue(const QString& key, const QVariant& value)
	{
		if (value == this->value(key))
		{
			return;
		}
		m_QSettings.setValue(key, value);
		emit Changed(key, value);
	}

	QVariant CSettings::value(const QString& key, const QVariant& defaultValue) const
	{
		return m_QSettings.value(key, defaultValue);
	}
}

#include <moc_Settings.cpp>
