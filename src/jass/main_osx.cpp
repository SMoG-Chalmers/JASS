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

#ifdef __APPLE__

#include <stdexcept>
#include <QtWidgets/qmessagebox.h>
#include <jass/Debug.h>
#include <jass/jass.h>
#include <jass_version.h>

int main(int argc, char** argv)
{
	jass::CJass app;
	try
	{
		return app.Run(argc, argv);
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Exception: %s", e.what());
		QMessageBox::critical(nullptr, VERC_PROJECT_NAME " (" VERC_VERSION ")", e.what());
	}

	return 1;
}

#endif