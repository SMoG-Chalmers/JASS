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

#include <cstdint>
#include <jass/Debug.h>
#include "JavaObjectSerializationStream.h"

namespace jass
{
    static const uint16_t STREAM_MAGIC = 0xaced;
    static const uint16_t STREAM_VERSION = 5;

    static const uint8_t TC_NULL = 0x70;
    static const uint8_t TC_REFERENCE = 0x71;
    static const uint8_t TC_CLASSDESC = 0x72;
    static const uint8_t TC_OBJECT = 0x73;
    static const uint8_t TC_STRING = 0x74;
    static const uint8_t TC_ARRAY = 0x75;
    static const uint8_t TC_CLASS = 0x76;
    static const uint8_t TC_BLOCKDATA = 0x77;
    static const uint8_t TC_ENDBLOCKDATA = 0x78;
    static const uint8_t TC_RESET = 0x79;
    static const uint8_t TC_BLOCKDATALONG = 0x7A;
    static const uint8_t TC_EXCEPTION = 0x7B;
    static const uint8_t TC_LONGSTRING = 0x7C;
    static const uint8_t TC_PROXYCLASSDESC = 0x7D;
    static const uint8_t TC_ENUM = 0x7E;

    static const int32_t BASE_WIRE_HANDLE = 0x7E0000;

    enum EClassDescFlags : uint8_t
    {
        WRITE_METHOD = 0x01, //if SC_SERIALIZABLE
        BLOCK_DATA = 0x08,    //if SC_EXTERNALIZABLE
        SERIALIZABLE = 0x02,
        EXTERNALIZABLE = 0x04,
        ENUM = 0x10,
    };

    const char* SClassField::TypeStr(EType type)
    {
        switch (type)
        {
        case Byte: return "byte";
        case Char: return "char";
        case Double: return "double";
        case Float: return "float";
        case Integer: return "int";
        case Long: return "long";
        case Short: return "short";
        case Boolean: return "boolean";
        case Array: return "Array";
        case Object: return "Object";
        }
        return "?";
    }

    void CJavaClassDesc::dump()
    {
        LOG_INFO("Class: %s", this->Name.c_str());
        LOG_INFO("Fields:");
        for (const auto& field : this->Fields)
        {
            if (field.Type == SClassField::Object)
            {
                LOG_INFO("\t%s %s", field.ClassName.c_str(), field.Name.c_str());
            }
            else if (field.Type == SClassField::Array)
            {
                LOG_INFO("\t%s %s[]", field.ClassName.c_str(), field.Name.c_str());
            }
            else
            {
                LOG_INFO("\t%s %s", SClassField::TypeStr(field.Type), field.Name.c_str());
            }
        }
    }

    CJavaObjectSerializationStream::CJavaObjectSerializationStream(QIODevice& in)
        : m_In(in)
    {
        const auto magic = be(readValue<uint16_t>());
        if (magic != STREAM_MAGIC)
        {
            throw std::runtime_error("Not a valid java object serialization stream");
        }

        const auto version = be(readValue<uint16_t>());
        if (version != STREAM_VERSION)
        {
            throw std::runtime_error("stream version not supported");
        }
    }

    void CJavaObjectSerializationStream::AddObjectFactory(const char* className, std::unique_ptr<CJavaObjectFactory>&& factory)
    {
        m_UninitializedObjectFactories.push_back(uninitialized_object_factory_t(std::string(className), std::move(factory)));
    }

    const std::string& CJavaObjectSerializationStream::readString()
    {
        static const std::string null_string;

        switch (readNextType())
        {
        case TC_NULL:
        {
            return null_string;
        }
        case TC_STRING:
        {
            auto* s = newObject<CJavaStringObject>();
            s->String = readUtf();
            return s->String;
        }
        case TC_LONGSTRING:
        {
            auto* s = newObject<CJavaStringObject>();
            s->String = readUtfLong();
            return s->String;
        }
        case TC_REFERENCE:
            return readReferenceT<CJavaStringObject>()->String;
        }
        throw std::runtime_error("String expected");
    }

    CJavaObject* CJavaObjectSerializationStream::readObject()
    {
        switch (readNextType())
        {
        case TC_NULL:
            return nullptr;
        case TC_REFERENCE:
            return readReference();
        case TC_OBJECT:
            break;
        default:
            throw std::runtime_error("Object expected");
        }
        
        auto* classDesc = readClassDesc();
        
        auto* obj = classDesc->ObjectFactory->New();
        m_Objects.push_back(std::unique_ptr<CJavaObject>(obj));

        classDesc->ObjectFactory->Load(*obj, *this);

        if (classDesc->Flags & EClassDescFlags::WRITE_METHOD || classDesc->Flags & EClassDescFlags::BLOCK_DATA)
        {
            if (readValue<uint8_t>() != TC_ENDBLOCKDATA)
            {
                throw std::runtime_error("Object expected");
            }
        }

        return obj;
    }

    void CJavaObjectSerializationStream::skipArray()
    {
        if (TC_NULL != readNextType())
        {
            throw std::runtime_error("Arrays are note supported yet");
        }
    }

    void CJavaObjectSerializationStream::dump(size_t size)
    {
        uint8_t* buf = (uint8_t*)alloca(size);
        const auto n = (size_t)m_In.read((char*)buf, size);

        char* text_buf = (char*)alloca(size * 3 + 1);

        for (size_t i = 0; i < n; ++i)
        {
            snprintf(text_buf + i*3, 4, "%.2X ", buf[i]);
        }
        LOG_INFO(text_buf);
    }

    uint8_t CJavaObjectSerializationStream::readNextType()
    {
        if (inBlock())
        {
            throw std::runtime_error("Unfinished data block");
        }
        return readValue<uint8_t>();
    }

    void CJavaObjectSerializationStream::readBlock(void* buffer, size_t size)
    {
        if (!inBlock())
        {
            const uint8_t typeCode = readValue<uint8_t>();
            switch (typeCode)
            {
            case TC_BLOCKDATA:
                m_RemainingBlockSize = readValue<uint8_t>();
                break;
            case TC_BLOCKDATALONG:
                m_RemainingBlockSize = be(readValue<uint32_t>());
                break;
            default:
                throw std::runtime_error("Block data expected");
            }
        }

        if (size > m_RemainingBlockSize)
        {
            throw std::runtime_error("More block data expected");
        }

        readExact(buffer, size);

        m_RemainingBlockSize -= size;
    }

    std::string CJavaObjectSerializationStream::readUtf()
    {
        const auto length = be(readValue<uint16_t>());
        std::string s;
        s.resize(length);
        readExact(s.data(), length);
        return std::move(s);
    }

    std::string CJavaObjectSerializationStream::readUtfLong()
    {
        const auto length = be(readValue<uint32_t>());
        std::string s;
        s.resize(length);
        readExact(s.data(), length);
        return std::move(s);
    }

    const CJavaClassDesc* CJavaObjectSerializationStream::readClassDesc()
    {
        const auto typeCode = readNextType();
        if (TC_NULL == typeCode)
        {
            return nullptr;
        }
        if (TC_REFERENCE == typeCode)
        {
            return readReferenceT<CJavaClassDesc>();
        }
        if (TC_CLASSDESC != typeCode)
        {
            throw std::runtime_error("Class description expected");
        }

        auto* classDesc = newObject<CJavaClassDesc>();
        classDesc->Name = readUtf();
        classDesc->SerialVersionUID = be(readValue<uint64_t>());
        classDesc->Flags = readValue<uint8_t>();

        const auto fieldCount = be(readValue<uint16_t>());
        classDesc->Fields.resize(fieldCount);
        for (auto& field : classDesc->Fields)
        {
            const uint8_t type_code = readValue<uint8_t>();
            switch (type_code)
            {
            case 'B': field.Type = SClassField::Byte; break;
            case 'C': field.Type = SClassField::Char; break;
            case 'D': field.Type = SClassField::Double; break;
            case 'F': field.Type = SClassField::Float; break;
            case 'I': field.Type = SClassField::Integer; break;
            case 'J': field.Type = SClassField::Long; break;
            case 'S': field.Type = SClassField::Short; break;
            case 'Z': field.Type = SClassField::Boolean; break;
            case '[': field.Type = SClassField::Array; break;
            case 'L': field.Type = SClassField::Object; break;
            default:
                throw std::runtime_error("Unsupported class field type");
            }
            field.Name = readUtf();
            if (SClassField::Array == field.Type || SClassField::Object == field.Type)
            {
                field.ClassName = readString();
            }
        }

        // Annotations
        if (TC_ENDBLOCKDATA != readNextType())
        {
            throw std::runtime_error("Class annotations is not yet supported");
        }

        classDesc->SuperClass = readClassDesc();

        //classDesc->dump();

        for (auto it = m_UninitializedObjectFactories.begin(); m_UninitializedObjectFactories.end() != it; ++it)
        {
            if (it->first == classDesc->Name)
            {
                classDesc->ObjectFactory = std::move(it->second);
                m_UninitializedObjectFactories.erase(it);
                break;
            }
        }

        if (!classDesc->ObjectFactory)
        {
            JASS_FORMAT_EXCEPTION("No factory found for class '%s'", classDesc->Name.c_str());
        }

        classDesc->ObjectFactory->Init(*classDesc);

        return classDesc;
    }

    CJavaObject* CJavaObjectSerializationStream::readReference()
    {
        auto reference = be(readValue<reference_handle_t>());
        const size_t object_index = (size_t)reference - BASE_WIRE_HANDLE;
        if (object_index >= m_Objects.size())
        {
            throw std::runtime_error("Invalid object reference");
        }
        return m_Objects[object_index].get();
    }
}