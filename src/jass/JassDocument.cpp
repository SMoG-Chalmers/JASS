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


#include <QtGui/qicon.h>

#include <jass/GraphEditor/JassEditor.hpp>
#include "JassDocument.hpp"
#include "JassFileFormat.h"
#include "LegacyJassFileFormat.h"

namespace jass
{
	CJassDocumentTypeHandler CJassDocumentTypeHandler::g_Instance;

	CJassDocumentTypeHandler::CJassDocumentTypeHandler()
	{
		m_Description.Name = "Jass Project";
		m_Description.Icon = QIcon();
		m_Description.FileNamePrefix = "JassProject";
		m_Description.ExtensionNoDot = "jass";
		m_Description.Capabilities = qapp::SDocumentTypeDesc::Caps_New | qapp::SDocumentTypeDesc::Caps_Load;
		m_Description.MinIdBlockSize = std::max(JASS_FILE_ID_BYTE_COUNT, LEGACY_JASS_FILE_ID_BYTE_COUNT);
	}

	std::unique_ptr<qapp::CDocument> CJassDocumentTypeHandler::NewDocument()
	{
		return std::make_unique<CJassDocument>();
	}

	std::unique_ptr<qapp::CDocument> CJassDocumentTypeHandler::LoadDocument(QIODevice& in)
	{
		auto doc = std::make_unique<CJassDocument>();

		auto id_block = in.peek(m_Description.MinIdBlockSize);
		if (IsJassFile(std::span<const uint8_t>((const uint8_t*)id_block.constData(), id_block.size())))
		{
			LoadJassFile(in, *doc);
		}
		else
		{
			LoadLegacyJassFile(in, *doc);
		}

		return doc;
	}

	std::span<const qapp::SDocumentTypeDesc> CJassDocumentTypeHandler::SupportedSaveFormats()
	{
		return std::span<const qapp::SDocumentTypeDesc>(&m_Description, 1);
	}

	void CJassDocumentTypeHandler::SaveDocument(qapp::CDocument& document, QIODevice& out, const qapp::SDocumentTypeDesc& format)
	{
		auto* jass_doc = dynamic_cast<CJassDocument*>(&document);
		if (!jass_doc)
		{
			throw std::runtime_error("Unsupported document type");
		}
		SaveJassFile(out, *jass_doc);
	}

	std::unique_ptr<qapp::CEditor> CJassDocumentTypeHandler::CreateEditor(qapp::CDocument& document)
	{
		return std::make_unique<CJassEditor>(static_cast<CJassDocument&>(document));
	}
	
	bool CJassDocumentTypeHandler::TryIdBlock(const QByteArray& id_block)
	{
		return 
			IsJassFile(std::span<const uint8_t>((const uint8_t*)id_block.constData(), id_block.size())) ||
			IsLegacyJassFile(std::span<const uint8_t>((const uint8_t*)id_block.constData(), id_block.size()));
	}


	// CJassDocument

	void CJassDocument::SetImage(const QByteArray& image_data, QString extension_no_dot)
	{
		m_ImageData = image_data;
		m_ImageExtensionNoDot = extension_no_dot;
	}
}