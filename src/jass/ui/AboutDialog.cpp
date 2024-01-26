/*
Copyright XMN Software AB 2023

JASS is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version. The GNU Lesser General Public License is intended to
guarantee your freedom to share and change all versions of a program --
to make sure it remains free software for all its users.

JASS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with JASS. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qpushbutton.h>
#include <jass_version.h>

#include "AboutDialog.h"

static const char* s_AuthorsText = R"(
<center>
	<b>Authors</b><br/>
	Spatial Morphology Group (SMoG)-Chalmers University of Technology,<br/>
	KTH School of Architecture,<br/>
	(Ioanna Stavroulaki, Martin Fitger,	Meta Berghauser Pont, <br/>
	Daniel Koch, Lena Bergsten, Tommy F&auml;rnqvist, <br/>
	Patrik Georgii-Hemming, Per Grandien, Christer Olofsson,<br/>
	Mikael Silfver, Erik Sj&ouml;stedt, Fredrik Stavfors, Marko Tokic)
</center>
)";

namespace jass
{
	CAboutDialog::CAboutDialog(QWidget* parent)
	{
		setWindowTitle("About");

		auto* vlayout = new QVBoxLayout();
		vlayout->setMargin(30);
		vlayout->setSpacing(15);

		{
			auto* iconLabel = new QLabel(this);
			iconLabel->setPixmap(QPixmap(":/logo/logo_64x64.png"));

			auto* hlayout = new QHBoxLayout();
			hlayout->setMargin(0);
			hlayout->addStretch();
			hlayout->addWidget(iconLabel);
			hlayout->addStretch();

			vlayout->addLayout(hlayout);
		}
		vlayout->addWidget(new QLabel("<center><h1>" VERC_PROJECT_NAME "</h1></center>", this));
		vlayout->addWidget(new QLabel("<center>Version " VERC_VERSION "</center>", this));
		vlayout->addWidget(new QLabel("<center>GNU General Public Licence</center>", this));
		vlayout->addWidget(new QLabel(s_AuthorsText, this));

		vlayout->addSpacing(20);

		{
			auto* ok_button = new QPushButton("OK", this);
			ok_button->setDefault(true);
			connect(ok_button, &QPushButton::clicked, this, &CAboutDialog::accept);

			auto* hlayout = new QHBoxLayout();
			hlayout->setMargin(0);
			hlayout->addStretch();
			hlayout->addWidget(ok_button);
			hlayout->addStretch();

			vlayout->addLayout(hlayout);
		}

		setLayout(vlayout);

		// Center in parent
		const auto center = parent->geometry().center();
		const auto size = sizeHint();
		setGeometry(QRect(center.x() - size.width() / 2, center.y() - size.height() / 2, size.width(), size.height()));
	}
}