/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  August 09, 2026                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

// Сериализация проекта в JSON через simdjson.
//
// Гибрид (проверено пробой на GCC 16):
//  - ЧТЕНИЕ  — родной рефлексивный simdjson: val.get<T>() сам обходит агрегаты
//    и сваливается в tag_invoke(deserialize_tag, ...) на членах;
//  - ЗАПИСЬ  — свой диспетчер Json::append: родной to_json не собирается для
//    агрегатов с tag_invoke-членами (в быстром пути atom(writer&,...) нет
//    ветки для них), поэтому агрегаты обходим сами через for_each_field, а
//    от simdjson берём примитивы string_builder (экранирование, числа).
//
// Свои типы подключаются hidden-friend'ами tag_invoke (доступ к приватным
// членам — как раньше friend-операторы QDataStream), Qt-типы — свободными
// tag_invoke в namespace simdjson ниже.

#include "reflection.h"

#include <simdjson.h>

#include <QColor>
#include <QDateTime>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <cmath>
#include <map>
#include <ranges>
#include <string_view>

namespace Json {

using Writer = simdjson::builder::string_builder;
using Reader = simdjson::ondemand::object;

template <typename T>
concept Custom = simdjson::require_custom_serialization<T>;

template <typename T>
void append(Writer& sb, const T& v);

template <typename T>
void writeFields(Writer& sb, const T& v);

} // namespace Json

namespace simdjson {

////////////////////////////////////////////////////////////////
/// Qt-типы
///
inline void tag_invoke(serialize_tag, auto& sb, const std::same_as<QString> auto& s) {
    const QByteArray utf8 = s.toUtf8();
    sb.escape_and_append_with_quotes(std::string_view{utf8.constData(), size_t(utf8.size())});
}

inline error_code tag_invoke(deserialize_tag, auto& val, QString& s) {
    std::string_view sv;
    if(auto err = val.get_string().get(sv); err) return err;
    s = QString::fromUtf8(sv.data(), qsizetype(sv.size()));
    return SUCCESS;
}

inline void tag_invoke(serialize_tag, auto& sb, const std::same_as<QColor> auto& c) {
    tag_invoke(serialize_tag{}, sb, c.name(QColor::HexArgb));
}

inline error_code tag_invoke(deserialize_tag, auto& val, QColor& c) {
    QString name;
    if(auto err = tag_invoke(deserialize_tag{}, val, name); err) return err;
    c = QColor{name};
    return SUCCESS;
}

inline void tag_invoke(serialize_tag, auto& sb, const std::same_as<QDateTime> auto& dt) {
    tag_invoke(serialize_tag{}, sb, dt.toString(Qt::ISODateWithMs));
}

inline error_code tag_invoke(deserialize_tag, auto& val, QDateTime& dt) {
    QString iso;
    if(auto err = tag_invoke(deserialize_tag{}, val, iso); err) return err;
    dt = QDateTime::fromString(iso, Qt::ISODateWithMs);
    return SUCCESS;
}

// [x, y]; NaN ↔ null — у координат дефолт NaN, а NaN в JSON нелегален.
inline void tag_invoke(serialize_tag, auto& sb, const std::same_as<QPointF> auto& p) {
    sb.start_array();
    if(std::isnan(p.x())) sb.append_null();
    else sb.append(p.x());
    sb.append_comma();
    if(std::isnan(p.y())) sb.append_null();
    else sb.append(p.y());
    sb.end_array();
}

inline error_code tag_invoke(deserialize_tag, auto& val, QPointF& p) {
    ondemand::array arr;
    if(auto err = val.get_array().get(arr); err) return err;
    double xy[2]{};
    size_t i{};
    for(auto elem: arr) {
        if(i == 2) return CAPACITY;
        ondemand::value v;
        if(auto err = elem.get(v); err) return err;
        bool null{};
        if(auto err = v.is_null().get(null); err) return err;
        if(null) xy[i] = std::numeric_limits<double>::quiet_NaN();
        else if(auto err = v.get_double().get(xy[i]); err) return err;
        ++i;
    }
    if(i != 2) return INCORRECT_TYPE;
    p = {xy[0], xy[1]};
    return SUCCESS;
}

// [x, y, w, h]
inline void tag_invoke(serialize_tag, auto& sb, const std::same_as<QRectF> auto& r) {
    sb.start_array();
    sb.append(r.x()), sb.append_comma();
    sb.append(r.y()), sb.append_comma();
    sb.append(r.width()), sb.append_comma();
    sb.append(r.height());
    sb.end_array();
}

inline error_code tag_invoke(deserialize_tag, auto& val, QRectF& r) {
    ondemand::array arr;
    if(auto err = val.get_array().get(arr); err) return err;
    double v[4]{};
    size_t i{};
    for(auto elem: arr) {
        if(i == 4) return CAPACITY;
        if(auto err = elem.get_double().get(v[i]); err) return err;
        ++i;
    }
    if(i != 4) return INCORRECT_TYPE;
    r = {v[0], v[1], v[2], v[3]};
    return SUCCESS;
}

////////////////////////////////////////////////////////////////
/// enum ↔ число подлежащего типа. Запись делает Json::append (to_underlying).
/// Родной рефлексивный путь simdjson гоняет enum'ы ИМЕНАМИ энумераторов —
/// для битовых комбинаций без имени (GraphicObject::Type: Circle|FlStamp)
/// это не работает вовсе. Конкретный ValT (а не шаблонный, как у них)
/// делает эти перегрузки более специализированными — partial ordering
/// выбирает их без неоднозначности.
template <typename E>
    requires std::is_enum_v<E>
error_code tag_invoke(deserialize_tag, ondemand::value& val, E& e) {
    int64_t i{};
    if(auto err = val.get_int64().get(i); err) return err;
    e = static_cast<E>(i);
    return SUCCESS;
}

template <typename E>
    requires std::is_enum_v<E>
error_code tag_invoke(deserialize_tag, ondemand::document& val, E& e) {
    int64_t i{};
    if(auto err = val.get_int64().get(i); err) return err;
    e = static_cast<E>(i);
    return SUCCESS;
}

////////////////////////////////////////////////////////////////
/// std::map
///
/// map<QString, V> ↔ объект. Родной string_view_keyed_map не берёт QString
/// (не конвертится в string_view).
template <typename V, typename C, typename A>
void tag_invoke(serialize_tag, auto& sb, const std::map<QString, V, C, A>& map) {
    sb.start_object();
    bool first = true;
    for(auto& [key, val]: map) {
        if(!first) sb.append_comma();
        first = false;
        const QByteArray utf8 = key.toUtf8();
        sb.escape_and_append_with_quotes(std::string_view{utf8.constData(), size_t(utf8.size())});
        sb.append_colon();
        Json::append(sb, val);
    }
    sb.end_object();
}

template <typename V, typename C, typename A>
error_code tag_invoke(deserialize_tag, auto& val, std::map<QString, V, C, A>& map) {
    ondemand::object obj;
    if(auto err = val.get_object().get(obj); err) return err;
    map.clear();
    for(auto field: obj) {
        std::string_view key;
        if(auto err = field.unescaped_key().get(key); err) return err;
        V v{};
        if(auto err = field.value().template get<V>().get(v); err) return err;
        map.emplace(QString::fromUtf8(key.data(), qsizetype(key.size())), std::move(v));
    }
    return SUCCESS;
}

/// map с прочим нестроковым ключом ↔ массив пар [key, value]: JSON-объект не
/// умеет ключ-не-строку (UsedItems: map<vector<int32_t>, vector<int32_t>>;
/// visibility_: map<int32_t, bool>; апертуры гербера: map<int32_t, shared_ptr>).
template <typename K, typename V, typename C, typename A>
    requires(!std::is_convertible_v<K, std::string_view> && !std::is_same_v<K, QString>)
void tag_invoke(serialize_tag, auto& sb, const std::map<K, V, C, A>& map) {
    sb.start_array();
    bool first = true;
    for(auto& [key, val]: map) {
        if(!first) sb.append_comma();
        first = false;
        sb.start_array();
        Json::append(sb, key);
        sb.append_comma();
        Json::append(sb, val);
        sb.end_array();
    }
    sb.end_array();
}

template <typename K, typename V, typename C, typename A>
    requires(!std::is_convertible_v<K, std::string_view> && !std::is_same_v<K, QString>)
error_code tag_invoke(deserialize_tag, auto& val, std::map<K, V, C, A>& map) {
    ondemand::array arr;
    if(auto err = val.get_array().get(arr); err) return err;
    map.clear();
    for(auto elem: arr) {
        ondemand::array pair;
        if(auto err = elem.get_array().get(pair); err) return err;
        K key{};
        V value{};
        size_t i{};
        for(auto item: pair) {
            if(i == 2) return CAPACITY;
            ondemand::value v;
            if(auto err = item.get(v); err) return err;
            if(i == 0) {
                if(auto err = v.template get<K>().get(key); err) return err;
            } else if(auto err = v.template get<V>().get(value); err)
                return err;
            ++i;
        }
        if(i != 2) return INCORRECT_TYPE;
        map.emplace(std::move(key), std::move(value));
    }
    return SUCCESS;
}

} // namespace simdjson

namespace Json {

////////////////////////////////////////////////////////////////
/// Запись: свой рекурсивный диспетчер.
/// Порядок веток существенен: tag_invoke раньше всего (QString — это и range,
/// и класс), enum раньше арифметики, range раньше «просто класса».
template <typename T>
void append(Writer& sb, const T& v) {
    if constexpr(Custom<T>)
        sb.append(v); // serialize → tag_invoke
    else if constexpr(std::is_enum_v<T>)
        sb.append(std::to_underlying(v));
    else if constexpr(std::is_floating_point_v<T>) {
        if(std::isnan(v)) sb.append_null();
        else sb.append(v);
    } else if constexpr(std::is_arithmetic_v<T>)
        sb.append(v);
    else if constexpr(std::is_convertible_v<T, std::string_view>)
        sb.append(v);
    else if constexpr(std::ranges::input_range<T>) {
        sb.start_array();
        bool first = true;
        for(auto& item: v) {
            if(!first) sb.append_comma();
            first = false;
            Json::append(sb, item);
        }
        sb.end_array();
    } else if constexpr(std::is_class_v<T>)
        writeFields(sb, v); // агрегат — объект по всем полям через рефлексию
    else
        static_assert(false, "Json::append: неизвестный тип");
}

/// Весь агрегат как объект по именам полей (аналог Block{}.write(val)).
/// Пишет напрямую, минуя диспетчер append: тело tag_invoke из JSON_POD зовёт
/// именно её, иначе Custom-ветка append зациклилась бы на самой себе.
template <typename T>
void writeFields(Writer& sb, const T& v) {
    sb.start_object();
    bool first = true;
    for_each_field(v, [&sb, &first](auto& field, std::string_view name) {
        if(!first) sb.append_comma();
        first = false;
        sb.escape_and_append_with_quotes(name);
        sb.append_colon();
        Json::append(sb, field);
    });
    sb.end_object();
}

/// Объект из именованных полей: Json::write(sb, "id", id_, "date", date_, ...)
/// — замена Block{}.write(...) там, где сериализуемое ≠ все члены класса.
namespace detail {
template <typename K, typename V, typename... Rest>
void writePairs(Writer& sb, bool& first, const K& key, const V& val, const Rest&... rest) {
    if(!first) sb.append_comma();
    first = false;
    sb.escape_and_append_with_quotes(std::string_view{key});
    sb.append_colon();
    Json::append(sb, val);
    if constexpr(sizeof...(rest)) writePairs(sb, first, rest...);
}
} // namespace detail

template <typename... Args>
void write(Writer& sb, const Args&... args) {
    static_assert(sizeof...(args) % 2 == 0, "Json::write: пары «ключ, значение»");
    sb.start_object();
    [[maybe_unused]] bool first = true;
    if constexpr(sizeof...(args)) detail::writePairs(sb, first, args...);
    sb.end_object();
}

////////////////////////////////////////////////////////////////
/// Чтение: родной get<T>. Хелперы ниже — толерантные (нет ключа или не тот
/// тип → поле остаётся дефолтным), как раньше Block::read с недобором полей.
template <typename T>
simdjson::error_code get(auto&& src, T& out) {
    return src.template get<T>().get(out);
}

namespace detail {
template <typename K, typename V, typename... Rest>
void readPairs(Reader& obj, const K& key, V& val, Rest&... rest) {
    if constexpr(std::is_bounded_array_v<V>) {
        // C-массив: simdjson_result<T[N]> не существует — поэлементно.
        simdjson::ondemand::array arr;
        if(!obj[std::string_view{key}].get_array().get(arr)) {
            size_t i{};
            for(auto elem: arr) {
                if(i == std::extent_v<V>) break;
                simdjson::ondemand::value v;
                if(elem.get(v)) break;
                [[maybe_unused]] auto err = v.template get<std::remove_extent_t<V>>().get(val[i]);
                ++i;
            }
        }
    } else {
        [[maybe_unused]] auto err = obj[std::string_view{key}].template get<V>().get(val);
    }
    if constexpr(sizeof...(rest)) readPairs(obj, rest...);
}
} // namespace detail

template <typename... Args>
void read(Reader& obj, Args&&... args) {
    static_assert(sizeof...(args) % 2 == 0, "Json::read: пары «ключ, значение»");
    if constexpr(sizeof...(args)) detail::readPairs(obj, args...);
}

/// Весь агрегат из объекта по именам полей.
template <typename T>
void readFields(Reader& obj, T& v) {
    for_each_field(v, [&obj](auto& field, std::string_view name) {
        [[maybe_unused]] auto err = obj[name].template get<std::remove_cvref_t<decltype(field)>>().get(field);
    });
}

////////////////////////////////////////////////////////////////
/// Разбор строки: владеет копией текста и парсером, отдаёт документ.
/// (ondemand требует padded_string, живущую дольше документа.)
struct Parsed {
    simdjson::padded_string data;
    simdjson::ondemand::parser parser;
    simdjson::ondemand::document doc;
    simdjson::error_code error{};

    explicit Parsed(std::string_view json)
        : data{json} {
        error = parser.iterate(data).get(doc);
    }
};

} // namespace Json

////////////////////////////////////////////////////////////////
/// Пара hidden-friend tag_invoke по всем полям типа — замена SERIALIZE_POD.
#define JSON_POD(TYPE)                                                              \
    friend void tag_invoke(simdjson::serialize_tag, auto& sb, const TYPE& p) {      \
        Json::writeFields(sb, p);                                                   \
    }                                                                               \
    friend simdjson::error_code tag_invoke(                                         \
        simdjson::deserialize_tag, auto& val, TYPE& p) {                            \
        simdjson::ondemand::object obj;                                             \
        if(auto err = val.get_object().get(obj); err) return err;                   \
        Json::readFields(obj, p);                                                   \
        return simdjson::SUCCESS;                                                   \
    }
