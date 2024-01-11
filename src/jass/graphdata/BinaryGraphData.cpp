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

#include <stdexcept>
#include <vector>

#include <qapplib/utils/StreamUtils.h>
//#include <qapplib/utils/PagePool.h>

#include <jass/Debug.h>
#include "BinaryGraphData.h"
#include "GraphView.h"
#include "GraphBuilder.h"


namespace jass
{
	const static uint32_t SERIALIZATION_VERSION = 1;

	void ToBinary(const QString& s, std::ostream& out)
	{
		const auto nameUtf8 = s.toUtf8();
		ASSERT(nameUtf8.size() < 256);
		qapp::twrite(out, (uint8_t)nameUtf8.size());
		out.write((char*)nameUtf8.constData(), nameUtf8.size());
	}

	template <typename T>
	T FromBinary(std::istream& in);

	template <>
	QString FromBinary(std::istream& in)
	{	
		const auto length = qapp::tread<uint8_t>(in);
		char* data = (char*)alloca(length);
		qapp::read_exact(in, data, length);
		return QString::fromUtf8(data, length);
	}

	void ToBinary(const IGraphView& gview, std::ostream& out)
	{
		//auto& page_pool = qapp::CPagePool::DefaultPagePool();

		out.write("JASG", 4);

		qapp::twrite(out, (uint32_t)SERIALIZATION_VERSION);

		const size_t node_count = gview.NodeCount();
		qapp::twrite(out, (uint32_t)node_count);

		const size_t node_attribute_count = gview.NodeAttributeCount();
		qapp::twrite(out, (uint32_t)node_attribute_count);
		for (size_t node_attribute_index = 0; node_attribute_index < node_attribute_count; ++node_attribute_index)
		{
			const auto desc = gview.NodeAttributeDesc(node_attribute_index);
			ToBinary(desc.Name, out);
			qapp::twrite(out, (uint8_t)desc.Type);
			const size_t size = qapp::QVariantTypeSize(desc.Type) * node_count;
			QByteArray data((int)size, 0);
			gview.GetNodeAttributeData(node_attribute_index, data.data(), data.size());
			out.write((char*)data.data(), data.size());
		}

		const size_t edge_count = gview.EdgeCount();
		qapp::twrite(out, (uint32_t)edge_count);
		{
			std::vector<IGraphView::edge_t> edges(edge_count);
			gview.GetEdges(edges);
			out.write((char*)edges.data(), edges.size() * sizeof(IGraphView::edge_t));
		}
	}

	void FromBinary(IGraphBuilder& gbuilder, std::istream& in)
	{
		const uint32_t magic = qapp::tread<uint32_t>(in);
		if (memcmp(&magic, "JASG", 4) != 0)
		{
			throw std::runtime_error("Unrecognized format");
		}

		if (qapp::tread<uint32_t>(in) != SERIALIZATION_VERSION)
		{
			throw std::runtime_error("Unsupported version");
		}

		const size_t node_count = qapp::tread<uint32_t>(in);
		gbuilder.SetNodeCount(node_count);

		const size_t node_attribute_count = qapp::tread<uint32_t>(in);
		for (size_t node_attribute_index = 0; node_attribute_index < node_attribute_count; ++node_attribute_index)
		{
			SNodeAttributeDesc desc;
			desc.Name = FromBinary<QString>(in);
			desc.Type = (QVariant::Type)qapp::tread<uint8_t>(in);
			const size_t size = qapp::QVariantTypeSize(desc.Type) * node_count;
			QByteArray data((int)size, 0);
			qapp::read_exact(in, data.data(), data.size());
			gbuilder.AddNodeAttribute(desc, data.data(), data.size());
		}

		const size_t edge_count = qapp::tread<uint32_t>(in);
		{
			std::vector<IGraphBuilder::edge_t> edges(edge_count);
			qapp::read_exact(in, edges.data(), edges.size() * sizeof(IGraphBuilder::edge_t));
			gbuilder.SetEdges(edges);
		}
	}
}