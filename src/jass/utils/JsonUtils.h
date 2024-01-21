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
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>

namespace jass
{
	template <typename T> inline QJsonValue::Type QJsonValueType();
	template <>	inline QJsonValue::Type QJsonValueType<int>() { return QJsonValue::Double; }
	template <>	inline QJsonValue::Type QJsonValueType<QString>() { return QJsonValue::String; }
	template <>	inline QJsonValue::Type QJsonValueType<QJsonObject>() { return QJsonValue::Object; }
	template <>	inline QJsonValue::Type QJsonValueType<QJsonArray>() { return QJsonValue::Array; }

	template <typename T> inline T JsonValue(const QJsonValue& jsonValue);
	template <>	inline int         JsonValue<int>(const QJsonValue& jsonValue) { return jsonValue.toInt(); }
	template <>	inline QString     JsonValue<QString>(const QJsonValue& jsonValue) { return jsonValue.toString(); }
	template <>	inline QJsonObject JsonValue<QJsonObject>(const QJsonValue& jsonValue) { return jsonValue.toObject(); }
	template <>	inline QJsonArray  JsonValue<QJsonArray>(const QJsonValue& jsonValue) { return jsonValue.toArray(); }

	template <typename T>
	inline bool TryGetJsonValue(const QJsonObject& obj, const QString& key, T& out_value)
	{
		auto it = obj.find(key);
		if (it == obj.end() || QJsonValueType<T>() != it->type())
		{
			return false;
		}
		out_value = JsonValue<T>(*it);
		return true;
	}

	template <typename T>
	inline T GetRequiredJsonValue(const QJsonObject& obj, const QString& key)
	{
		T value;
		if (!TryGetJsonValue(obj, key, value))
		{
			throw std::runtime_error(QString("Missing field \"%1\"").arg(key).toStdString().c_str());
		}
		return value;
	}

	template <typename T>
	inline T GetOptionalJsonValue(const QJsonObject& obj, const QString& key, const T& default_value)
	{
		T value;
		return TryGetJsonValue(obj, key, value) ? value : default_value;
	}
}