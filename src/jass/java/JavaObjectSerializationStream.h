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

#include <vector>

#include <QtCore/QIODevice>

#include <jass/endian.h>

namespace jass
{
    class CJavaClassDesc;
    class CJavaObjectFactory;
    class CJavaObjectSerializationStream;

    class CJavaObject
    {
    public:
        virtual ~CJavaObject() {}
    };

    class CJavaStringObject : public CJavaObject
    {
    public:
        std::string String;
    };

    struct SClassField
    {
        enum EType
        {
            Byte,
            Char,
            Double,
            Float,
            Integer,
            Long,
            Short,
            Boolean,
            Array,
            Object,
        };

        std::string Name;
        std::string ClassName;
        EType       Type;

        static const char* TypeStr(EType type);
    };

    class CJavaObjectFactory
    {
    public:
        virtual void Init(CJavaClassDesc& classDesc) { m_ClassDesc = &classDesc; }
        virtual CJavaObject* New() = 0;
        virtual void Load(CJavaObject& obj, CJavaObjectSerializationStream& in) = 0;

    protected:
        CJavaClassDesc* m_ClassDesc = nullptr;
    };

    template <class T>
    class CJavaObjectFactoryT: public CJavaObjectFactory
    {
    public:
        CJavaObject* New() { return new T(); }
    };

    class CJavaClassDesc: public CJavaObject
    {
    public:
        std::string              Name;
        uint64_t                 SerialVersionUID;
        uint8_t                  Flags;
        std::vector<SClassField> Fields;
        const CJavaClassDesc*    SuperClass = nullptr;
        std::unique_ptr<CJavaObjectFactory> ObjectFactory;

        void dump();
    };

    class CJavaObjectSerializationStream
    {
    public:
        CJavaObjectSerializationStream(QIODevice& in);

        void AddObjectFactory(const char* className, std::unique_ptr<CJavaObjectFactory>&& factory);

        template <typename T>
        T read()
        {
            T value;
            readBlock(&value, sizeof(value));
            return be(value);
        }

        template <typename T>
        inline T readField() { return be(readValue<T>()); }

        const std::string& readString();

        CJavaObject* readObject();

        void skipArray();

        template <class T>
        inline T* readObjectT()
        {
            auto* ptr = readObject();
            auto* tptr = dynamic_cast<T*>(ptr);
            if (!tptr && ptr)
            {
                throw std::runtime_error("Class type mismatch");
            }
            return tptr;
        }

        void dump(size_t size);

    private:
        typedef uint32_t reference_handle_t;

        template <typename T>
        bool tryReadValue(T& out_value)
        {
            return m_In.read((char*)&out_value, sizeof(T)) == sizeof(T);
        }

        template <typename T>
        T readValue()
        {
            T value;
            if (!tryReadValue(value))
            {
                throw std::runtime_error("End of stream");
            }
            return value;
        }

        void readExact(void* buffer, size_t size)
        {
            if (m_In.read((char*)buffer, size) != size)
            {
                throw std::runtime_error("End of stream");
            }
        }

        uint8_t readNextType();

        void readBlock(void* buffer, size_t size);

        std::string readUtf();

        std::string readUtfLong();

        inline bool inBlock() const { return m_RemainingBlockSize > 0; }

        const CJavaClassDesc* readClassDesc();

        CJavaObject* readReference();

        template <class T>
        inline T* readReferenceT()
        {
            auto* ptr = readReference();
            auto* tptr = dynamic_cast<T*>(ptr);
            if (!tptr && ptr)
            {
                throw std::runtime_error("Class type mismatch");
            }
            return tptr;
        }

        template <class T>
        inline T* newObject()
        {
            m_Objects.push_back(std::make_unique<T>());
            return static_cast<T*>(m_Objects.back().get());
        }

        QIODevice& m_In;
        size_t m_RemainingBlockSize = 0;
        uint8_t m_NextType;

        std::vector<std::unique_ptr<CJavaObject>> m_Objects;

        typedef std::pair<std::string, std::unique_ptr<CJavaObjectFactory>> uninitialized_object_factory_t;
        std::vector<uninitialized_object_factory_t> m_UninitializedObjectFactories;
    };
}