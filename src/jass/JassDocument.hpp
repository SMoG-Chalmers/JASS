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

#pragma once

#include <qapplib/Document.hpp>
#include <qapplib/DocumentManager.hpp>
#include "GraphEditor/CategorySet.hpp"
#include "GraphModel.hpp"

namespace jass
{
	class CJassDocumentTypeHandler : public qapp::IDocumentTypeHandler
	{
	public:
		CJassDocumentTypeHandler();

		std::unique_ptr<qapp::CDocument> NewDocument() override;
		std::unique_ptr<qapp::CDocument> LoadDocument(QIODevice& in) override;
		std::span<const qapp::SDocumentTypeDesc> SupportedSaveFormats() override;
		void SaveDocument(qapp::CDocument& document, QIODevice& out, const qapp::SDocumentTypeDesc& format) override;
		std::unique_ptr<qapp::CEditor>   CreateEditor(qapp::CDocument& document) override;
		bool                             TryIdBlock(const QByteArray& id_block) override;

		const qapp::SDocumentTypeDesc& Description() const 
		{ 
			return m_Description;
		}

		static CJassDocumentTypeHandler g_Instance;

	private:
		qapp::SDocumentTypeDesc m_Description;
		int m_NewDocumentNamingCounter = 0;
	};

	class CJassDocument: public qapp::CDocument
	{
	public:
		CJassDocument();

		CGraphModel& GraphModel() { return m_GraphModel; }
		const CGraphModel& GraphModel() const { return m_GraphModel; }

		inline CCategorySet& Categories() { return m_Categories; }
		inline const CCategorySet& Categories() const { return m_Categories; }

		void SetImage(const QByteArray& image_data, QString extension_no_dot);

		inline void RemoveImage() { SetImage(QByteArray(), QString()); }

		inline const QByteArray& ImageData() const { return m_ImageData; }

		inline const QString& ImageExtensionNoDot() const { return m_ImageExtensionNoDot; }

	private:
		CGraphModel m_GraphModel;
		QByteArray m_ImageData;
		QString m_ImageExtensionNoDot;
		CCategorySet m_Categories;
	};
}