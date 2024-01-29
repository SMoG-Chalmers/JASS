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

#include <QtCore/qmimedata.h>
#include <QtCore/qjsondocument.h>
#include <QtGui/qclipboard.h>
#include <QtGui/qguiapplication.h>
#include <qapplib/utils/StreamUtils.h>
#include <qapplib/utils/PageBufferStream.h>
#include <jass/graphdata/BinaryGraphData.h>
#include <jass/graphdata/JsonGraphData.h>
#include "GraphClipboardData.h"

namespace jass
{
	const QString JASS_GRAPH_MIME_FORMAT = "application/jass.graph";

	void SetGraphClipboardData(const IGraphView& graph_view)
	{
		auto mimeData = std::make_unique<QMimeData>();
		mimeData->setText(QJsonDocument(ToJson(graph_view)).toJson());
		
		{
			qapp::page_buffer buf;
			qapp::page_buffer_ostream out(buf);
			ToBinary(graph_view, out);
			QByteArray bytes((int)buf.size(), Qt::Uninitialized);
			buf.read(0, bytes.data(), bytes.size());
			mimeData->setData(JASS_GRAPH_MIME_FORMAT, bytes);
		}

		QGuiApplication::clipboard()->setMimeData(mimeData.release());
	}

	bool TryGetGraphClipboardData(IGraphBuilder& gbuilder)
	{
		auto* mimeData = QGuiApplication::clipboard()->mimeData();

		{
			auto binaryData = mimeData->data(JASS_GRAPH_MIME_FORMAT);
			if (!binaryData.isEmpty())
			{
				qapp::imemstream in(binaryData.data(), (char*)binaryData.data() + binaryData.size());
				FromBinary(gbuilder, in);
				return true;
			}
		}

		auto utf8TextData = mimeData->data("text/plain");
		if (utf8TextData.isEmpty())
		{
			return false;
		}
		
		auto jsonDocument = QJsonDocument::fromJson(utf8TextData);
		if (jsonDocument.isNull() || !jsonDocument.isObject()) 
		{
			return false;
		}

		GraphFromJson(gbuilder, jsonDocument.object());

		return true;
	}
}