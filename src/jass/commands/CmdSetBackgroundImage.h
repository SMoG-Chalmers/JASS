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

#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>

#include <qapplib/commands/Command.h>

namespace jass
{
	class CJassEditor;

	class CCmdSetBackgroundImage: public qapp::ICommand
	{
	public:
		CCmdSetBackgroundImage(qapp::SCommandCreationContext& ctx, CJassEditor& editor, QByteArray&& image_data, QString extension_no_dot);
	
		void Do(qapp::SCommandExecutionContext&) override;
		void Undo(qapp::SCommandExecutionContext&) override;

	private:
		QByteArray m_OldImageData;
		QByteArray m_NewImageData;
		QString    m_OldExtensionNoDot;
		QString    m_NewExtensionNoDot;
	};
}