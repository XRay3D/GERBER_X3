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

// Сериализация в JSON: ОДИН рефлексивный движок поверх simdjson, классы
// размечаются аннотациями — никаких функций сериализации на каждый класс.
//
//   [[=Serial::skip]]            — член не сериализуется (кэши, указатели);
//   [[=Serial::rename("key")]]   — переопределить ключ (по умолчанию ключ =
//                                  имя поля с обрезанным хвостовым '_');
//   [[=Serial::name("Gerber")]]  — на классе: стабильное имя типа для
//                                  полиморфной диспетчеризации.
//
// Диспетчеризация типов — ТОЛЬКО if constexpr по точному типу: ни ADL, ни
// tag_invoke, поэтому неявные конверсии (Tool(double), Variant(double),
// Polyline(QPolygonF)) в выбор ветки не вмешиваются в принципе — этим v2
// отличается от снесённого tag_invoke-слоя v1.
//
// Точечная кастомизация — специализация Serial::Adapter<T> (Qt-типы здесь,
// Geo-типы с PIMPL — в geo/geo_json.h, полиморфные фабрики — у владельцев).
//
// Классы обходятся через subobjects_of(^^T, unchecked): приватные члены
// видны без friend'ов, базовые подобъекты — сплайсом obj.[:base:] (P3293):
//   - базу с Adapter'ом или базу-контейнер пишем значением под ключом "base"
//     (Handle : QPointF, ApBlock : QVector<GrObject>);
//   - базу, производную от QObject/QPaintDevice/QGraphicsItem, пропускаем
//     (GUI-кишки; их сериализуемая часть — забота preSave/postLoad);
//   - остальные базы «флаттим»: их поля пишутся в тот же JSON-объект
//     (GCode::File : AbstractFile, GrObject : GraphicObject).
//
// Хуки жизненного цикла (по одному на ИЕРАРХИЮ, не на класс):
//   v.preSave()   — перед записью (актуализировать кэш-поля);
//   v.postLoad()  — после чтения (восстановить связи, создать Gi и т.п.).
//
// Чтение полностью своё (родной simdjson get<T> не знает обрезку ключей,
// skip и флатт баз): один проход по объекту, незнакомый ключ пропускается,
// отсутствующий — оставляет дефолт (толерантность, как раньше Block::read).

#include <simdjson.h>

#include <QColor>
#include <QDateTime>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <cmath>
#include <map>
#include <meta>
#include <ranges>
#include <string_view>

class QObject;
class QPaintDevice;
class QGraphicsItem;

namespace Serial {

using Writer = simdjson::builder::string_builder;

////////////////////////////////////////////////////////////////
/// Аннотации
///
namespace ann {

struct Skip { };

// Структурная consteval-строка (char[] вместо указателя — иначе
// reflect_constant для NTTP не проходит; тот же приём, что у simdjson).
template <unsigned N>
struct Rename {
    char s[N]{};
    consteval Rename(const char (&str)[N]) {
        for(unsigned i{}; i < N; ++i) s[i] = str[i];
    }
};
template <unsigned N>
Rename(const char (&)[N]) -> Rename<N>;

template <unsigned N>
struct Name {
    char s[N]{};
    consteval Name(const char (&str)[N]) {
        for(unsigned i{}; i < N; ++i) s[i] = str[i];
    }
};
template <unsigned N>
Name(const char (&)[N]) -> Name<N>;

} // namespace ann

inline constexpr ann::Skip skip{};

template <unsigned N>
consteval auto rename(const char (&str)[N]) { return ann::Rename<N>{str}; }

template <unsigned N>
consteval auto name(const char (&str)[N]) { return ann::Name<N>{str}; }

////////////////////////////////////////////////////////////////
/// Consteval-запросы к разметке
///
namespace detail {

inline constexpr auto UCTX = std::meta::access_context::unchecked();

// Член вне сериализации: помечен skip либо непригоден в принципе —
// указатель/ссылка (рантайм-связь, восстанавливают хуки) или const (прочитать
// в него нельзя). Автоматика здесь вместо разметки каждого такого поля.
consteval bool isSkipped(std::meta::info member) {
    if(!std::meta::annotations_of_with_type(member, ^^ann::Skip).empty()) return true;
    const auto type = std::meta::type_of(member);
    return std::meta::is_pointer_type(type)
        || std::meta::is_reference_type(type)
        || std::meta::is_const_type(std::meta::remove_reference(type));
}

/// Класс, целиком выведенный из сериализации: [[=Serial::skip]] на самом
/// классе (служебные базы вроде FileTree::Node).
template <typename T>
consteval bool isSkippedType() {
    return !std::meta::annotations_of_with_type(^^T, ^^ann::Skip).empty();
}

// Ключ члена: rename-аннотация либо имя поля с обрезанным хвостовым '_'.
template <std::meta::info Member>
consteval std::string_view keyOf() {
    template for(constexpr auto A:
        std::define_static_array(std::meta::annotations_of(Member))) {
        constexpr auto AT = std::meta::type_of(A);
        if constexpr(std::meta::has_template_arguments(AT)
            && std::meta::template_of(AT) == ^^ann::Rename) {
            using Ann = [:AT:];
            constexpr auto value = std::meta::extract<Ann>(A);
            return std::define_static_string(
                std::string_view{value.s, std::extent_v<decltype(value.s)> - 1});
        }
    }
    constexpr auto id = std::meta::identifier_of(Member);
    return id.ends_with('_') ? id.substr(0, id.size() - 1) : id;
}

} // namespace detail

/// Имя типа из [[=Serial::name("...")]] на классе; пустая строка, если нет.
template <typename T>
consteval std::string_view typeNameOf() {
    template for(constexpr auto A:
        std::define_static_array(std::meta::annotations_of(^^T))) {
        constexpr auto AT = std::meta::type_of(A);
        if constexpr(std::meta::has_template_arguments(AT)
            && std::meta::template_of(AT) == ^^ann::Name) {
            using Ann = [:AT:];
            constexpr auto value = std::meta::extract<Ann>(A);
            return std::define_static_string(
                std::string_view{value.s, std::extent_v<decltype(value.s)> - 1});
        }
    }
    return {};
}

////////////////////////////////////////////////////////////////
/// Точечная кастомизация: специализировать Serial::Adapter<T> с парой
///   static void write(Writer& sb, const T& v);
///   static simdjson::error_code read(simdjson::ondemand::value& val, T& v);
template <typename T>
struct Adapter;

template <typename T> concept HasAdapter = requires(Writer& sb, simdjson::ondemand::value& val, const T& cv, T& v) {
    Adapter<T>::write(sb, cv);
    { Adapter<T>::read(val, v) } -> std::same_as<simdjson::error_code>;
};

namespace detail {

template <typename T> concept Map = requires {
    typename T::key_type;
    typename T::mapped_type;
};

/// База, которую движок не обходит вовсе: GUI-кишки (их сериализуемая часть —
/// забота preSave/postLoad) или класс, помеченный skip.
template <typename T> concept SkippedBase = std::is_base_of_v<QObject, T>
    || std::is_base_of_v<QPaintDevice, T>
    || std::is_base_of_v<QGraphicsItem, T>
    || isSkippedType<T>();

// База, которую пишем значением под ключом "base", а не флаттим:
// свой Adapter или контейнер (ApBlock : QVector<GrObject>).
template <typename B> concept ValueBase = HasAdapter<B> || (std::ranges::input_range<B> && !SkippedBase<B>);

// Тип-range, который всё же класс со структурой, а не просто контейнер:
// у него КОНТЕЙНЕРНАЯ база и вдобавок свои поля либо другие базы
// (ApBlock : AbstractAperture + QVector<GrObject>). Чистые контейнеры
// остаются массивами: у QList база не-range (QListSpecialMethods) и член d,
// у QPolygonF база-range, но ни своих полей, ни других баз.
template <typename T>
consteval bool classLike() {
    if constexpr(!std::is_class_v<T>) // C-массивы и прочие range-не-классы
        return false;
    else {
        size_t rangeBases = 0, otherBases = 0;
        template for(constexpr auto B:
            std::define_static_array(std::meta::bases_of(^^T, UCTX))) {
            using BT = [:std::meta::type_of(B):];
            if constexpr(std::ranges::input_range<BT>) ++rangeBases;
            else ++otherBases;
        }
        const auto own = std::meta::nonstatic_data_members_of(^^T, UCTX).size();
        return rangeBases > 0 && (own > 0 || otherBases > 0);
    }
}

} // namespace detail

template <typename T>
void write(Writer& sb, const T& v);

template <typename T>
simdjson::error_code read(simdjson::ondemand::value& val, T& out);

////////////////////////////////////////////////////////////////
/// Запись
///
namespace detail {

// Поля v (включая флатт баз) в УЖЕ открытый JSON-объект.
template <typename T>
void writeMembers(Writer& sb, const T& v, bool& first) {
    template for(constexpr auto SUB:
        std::define_static_array(std::meta::subobjects_of(^^T, UCTX))) {
        if constexpr(std::meta::is_base(SUB)) {
            using B = [:std::meta::type_of(SUB):];
            if constexpr(ValueBase<B>) {
                if(!first) sb.append_comma();
                first = false;
                sb.append_raw("\"base\":");
                Serial::write(sb, static_cast<const B&>(v.[:SUB:]));
            } else if constexpr(!SkippedBase<B>)
                writeMembers<B>(sb, v.[:SUB:], first);
            // пропущенные базы (GUI, skip) не обходим
        } else if constexpr(!isSkipped(SUB)) {
            if(!first) sb.append_comma();
            first = false;
            sb.escape_and_append_with_quotes(keyOf<SUB>());
            sb.append_colon();
            Serial::write(sb, v.[:SUB:]);
        }
    }
}

} // namespace detail

/// Поля v в открытый объект, с ведущей запятой (для элементов с "type").
template <typename T>
void writeInto(Writer& sb, const T& v) {
    if constexpr(requires { v.preSave(); }) v.preSave();
    bool first = false; // перед первым полем будет запятая — "type" уже записан
    detail::writeMembers(sb, v, first);
}

template <typename T>
void write(Writer& sb, const T& v) {
    if constexpr(HasAdapter<T>)
        Adapter<T>::write(sb, v);
    else if constexpr(std::is_enum_v<T>)
        sb.append(std::to_underlying(v));
    else if constexpr(std::is_floating_point_v<T>) {
        if(std::isnan(v)) sb.append_null(); // NaN в JSON нелегален
        else sb.append(v);
    } else if constexpr(std::is_arithmetic_v<T>)
        sb.append(v);
    else if constexpr(std::is_convertible_v<T, std::string_view>)
        sb.escape_and_append_with_quotes(std::string_view{v});
    else if constexpr(detail::Map<T>) {
        if constexpr(std::same_as<typename T::key_type, QString>) {
            // строковый ключ → JSON-объект
            sb.start_object();
            bool first = true;
            for(auto& [key, val]: v) {
                if(!first) sb.append_comma();
                first = false;
                const QByteArray utf8 = key.toUtf8();
                sb.escape_and_append_with_quotes(
                    std::string_view{utf8.constData(), size_t(utf8.size())});
                sb.append_colon();
                write(sb, val);
            }
            sb.end_object();
        } else {
            // ключ-не-строка → массив пар [k, v] (JSON-объект такого не умеет)
            sb.start_array();
            bool first = true;
            for(auto& [key, val]: v) {
                if(!first) sb.append_comma();
                first = false;
                sb.start_array();
                write(sb, key);
                sb.append_comma();
                write(sb, val);
                sb.end_array();
            }
            sb.end_array();
        }
    } else if constexpr(std::ranges::input_range<T> && !detail::classLike<T>()) {
        sb.start_array();
        bool first = true;
        for(auto& item: v) {
            if(!first) sb.append_comma();
            first = false;
            write(sb, item);
        }
        sb.end_array();
    } else if constexpr(std::is_class_v<T>) {
        if constexpr(requires { v.preSave(); }) v.preSave();
        sb.start_object();
        bool first = true;
        detail::writeMembers(sb, v, first);
        sb.end_object();
    } else
        static_assert(false, "Serial::write: тип не сериализуем — Adapter или [[=Serial::skip]]?");
}

////////////////////////////////////////////////////////////////
/// Чтение
///
namespace detail {

// Подставить значение в член по ключу; true — ключ наш (включая флатт баз).
// Ошибка чтения конкретного поля толерантно оставляет дефолт (как Block::read
// при недоборе полей).
template <typename T>
bool tryReadMember(simdjson::ondemand::value& v, T& out, std::string_view key) {
    template for(constexpr auto SUB:
        std::define_static_array(std::meta::subobjects_of(^^T, UCTX))) {
        if constexpr(std::meta::is_base(SUB)) {
            using B = [:std::meta::type_of(SUB):];
            if constexpr(ValueBase<B>) {
                if(key == "base")
                    return (void)Serial::read(v, static_cast<B&>(out.[:SUB:])), true;
            } else if constexpr(!SkippedBase<B>) {
                if(tryReadMember<B>(v, out.[:SUB:], key)) return true;
            }
        } else if constexpr(!isSkipped(SUB)) {
            if(key == keyOf<SUB>())
                return (void)Serial::read(v, out.[:SUB:]), true;
        }
    }
    return false;
}

} // namespace detail

////////////////////////////////////////////////////////////////
/// Разбор текста: владеет копией и парсером (ondemand требует padded_string,
/// живущую дольше документа).
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

/// Поля из объекта (незнакомые ключи, включая "type", мимо), в обрамлении
/// хуков preLoad (подготовить контекст ДО чтения полей — Gerber::File
/// выставляет crutch для апертур) и postLoad.
template <typename T>
void readFields(simdjson::ondemand::object& obj, T& out) {
    if constexpr(requires { out.preLoad(); }) out.preLoad();
    for(auto field: obj) {
        std::string_view key;
        if(field.unescaped_key().get(key)) break;
        simdjson::ondemand::value v;
        if(field.value().get(v)) break;
        detail::tryReadMember(v, out, key); // непотреблённое значение ondemand скипает сам
    }
    if constexpr(requires { out.postLoad(); }) out.postLoad();
}

/// Поля из текста элемента {"type":..., <поля>} в существующий объект.
template <typename T>
bool loadInto(std::string_view json, T& out) {
    Parsed parsed{json};
    simdjson::ondemand::object obj;
    if(parsed.error || parsed.doc.get_object().get(obj)) return false;
    readFields(obj, out);
    return true;
}

template <typename T>
simdjson::error_code read(simdjson::ondemand::value& val, T& out) {
    if constexpr(HasAdapter<T>)
        return Adapter<T>::read(val, out);
    else if constexpr(std::is_enum_v<T>) {
        int64_t i{};
        if(auto err = val.get_int64().get(i); err) return err;
        out = static_cast<T>(i);
        return simdjson::SUCCESS;
    } else if constexpr(std::same_as<T, bool>)
        return val.get_bool().get(out);
    else if constexpr(std::is_floating_point_v<T>) {
        bool null{};
        if(auto err = val.is_null().get(null); err) return err;
        if(null) return out = std::numeric_limits<T>::quiet_NaN(), simdjson::SUCCESS;
        double d{};
        if(auto err = val.get_double().get(d); err) return err;
        out = static_cast<T>(d);
        return simdjson::SUCCESS;
    } else if constexpr(std::is_integral_v<T>) {
        if constexpr(std::is_signed_v<T>) {
            int64_t i{};
            if(auto err = val.get_int64().get(i); err) return err;
            out = static_cast<T>(i);
        } else {
            uint64_t u{};
            if(auto err = val.get_uint64().get(u); err) return err;
            out = static_cast<T>(u);
        }
        return simdjson::SUCCESS;
    } else if constexpr(std::is_convertible_v<T, std::string_view>) {
        std::string_view sv;
        if(auto err = val.get_string().get(sv); err) return err;
        out = T{sv};
        return simdjson::SUCCESS;
    } else if constexpr(std::is_bounded_array_v<T>) {
        simdjson::ondemand::array arr;
        if(auto err = val.get_array().get(arr); err) return err;
        size_t i{};
        for(auto elem: arr) {
            if(i == std::extent_v<T>) break;
            simdjson::ondemand::value v;
            if(auto err = elem.get(v); err) return err;
            if(auto err = read(v, out[i]); err) return err;
            ++i;
        }
        return simdjson::SUCCESS;
    } else if constexpr(detail::Map<T>) {
        out.clear();
        if constexpr(std::same_as<typename T::key_type, QString>) {
            simdjson::ondemand::object obj;
            if(auto err = val.get_object().get(obj); err) return err;
            for(auto field: obj) {
                std::string_view key;
                if(auto err = field.unescaped_key().get(key); err) return err;
                simdjson::ondemand::value v;
                if(auto err = field.value().get(v); err) return err;
                typename T::mapped_type value{};
                if(auto err = read(v, value); err) return err;
                out.emplace(QString::fromUtf8(key.data(), qsizetype(key.size())), std::move(value));
            }
        } else {
            simdjson::ondemand::array arr;
            if(auto err = val.get_array().get(arr); err) return err;
            for(auto elem: arr) {
                simdjson::ondemand::array pair;
                if(auto err = elem.get_array().get(pair); err) return err;
                typename T::key_type key{};
                typename T::mapped_type value{};
                size_t i{};
                for(auto item: pair) {
                    if(i == 2) return simdjson::CAPACITY;
                    simdjson::ondemand::value v;
                    if(auto err = item.get(v); err) return err;
                    if(i == 0) {
                        if(auto err = read(v, key); err) return err;
                    } else if(auto err = read(v, value); err)
                        return err;
                    ++i;
                }
                if(i != 2) return simdjson::INCORRECT_TYPE;
                out.emplace(std::move(key), std::move(value));
            }
        }
        return simdjson::SUCCESS;
    } else if constexpr(std::ranges::input_range<T> && !detail::classLike<T>()) {
        static_assert(std::default_initializable<std::ranges::range_value_t<T>>,
            "Serial::read: элемент контейнера не дефолтный — нужен Adapter");
        simdjson::ondemand::array arr;
        if(auto err = val.get_array().get(arr); err) return err;
        out.clear();
        for(auto elem: arr) {
            simdjson::ondemand::value v;
            if(auto err = elem.get(v); err) return err;
            std::ranges::range_value_t<T> item{};
            if(auto err = read(v, item); err) return err;
            out.push_back(std::move(item));
        }
        return simdjson::SUCCESS;
    } else if constexpr(std::is_class_v<T>) {
        simdjson::ondemand::object obj;
        if(auto err = val.get_object().get(obj); err) return err;
        readFields(obj, out);
        return simdjson::SUCCESS;
    } else
        static_assert(false, "Serial::read: тип не десериализуем — Adapter или [[=Serial::skip]]?");
}

/// Загрузка полиморфного элемента {"type":..., <поля>}: new T, поля, хуки.
template <typename T>
T* load(std::string_view json) {
    auto* out = new T;
    if(!loadInto(json, *out)) return delete out, nullptr;
    return out;
}

////////////////////////////////////////////////////////////////
/// Adapter'ы Qt-типов (точечные, по точному типу)
///
template <>
struct Adapter<QString> {
    static void write(Writer& sb, const QString& s) {
        const QByteArray utf8 = s.toUtf8();
        sb.escape_and_append_with_quotes(std::string_view{utf8.constData(), size_t(utf8.size())});
    }
    static simdjson::error_code read(simdjson::ondemand::value& val, QString& s) {
        std::string_view sv;
        if(auto err = val.get_string().get(sv); err) return err;
        s = QString::fromUtf8(sv.data(), qsizetype(sv.size()));
        return simdjson::SUCCESS;
    }
};

template <>
struct Adapter<QColor> {
    static void write(Writer& sb, const QColor& c) {
        Adapter<QString>::write(sb, c.name(QColor::HexArgb));
    }
    static simdjson::error_code read(simdjson::ondemand::value& val, QColor& c) {
        QString name;
        if(auto err = Adapter<QString>::read(val, name); err) return err;
        c = QColor{name};
        return simdjson::SUCCESS;
    }
};

template <>
struct Adapter<QDateTime> {
    static void write(Writer& sb, const QDateTime& dt) {
        Adapter<QString>::write(sb, dt.toString(Qt::ISODateWithMs));
    }
    static simdjson::error_code read(simdjson::ondemand::value& val, QDateTime& dt) {
        QString iso;
        if(auto err = Adapter<QString>::read(val, iso); err) return err;
        dt = QDateTime::fromString(iso, Qt::ISODateWithMs);
        return simdjson::SUCCESS;
    }
};

// [x, y]; NaN ↔ null — у координат дефолт NaN, а NaN в JSON нелегален.
template <>
struct Adapter<QPointF> {
    static void write(Writer& sb, const QPointF& p) {
        sb.start_array();
        if(std::isnan(p.x())) sb.append_null();
        else sb.append(p.x());
        sb.append_comma();
        if(std::isnan(p.y())) sb.append_null();
        else sb.append(p.y());
        sb.end_array();
    }
    static simdjson::error_code read(simdjson::ondemand::value& val, QPointF& p) {
        simdjson::ondemand::array arr;
        if(auto err = val.get_array().get(arr); err) return err;
        double xy[2]{};
        size_t i{};
        for(auto elem: arr) {
            if(i == 2) return simdjson::CAPACITY;
            simdjson::ondemand::value v;
            if(auto err = elem.get(v); err) return err;
            bool null{};
            if(auto err = v.is_null().get(null); err) return err;
            if(null) xy[i] = std::numeric_limits<double>::quiet_NaN();
            else if(auto err = v.get_double().get(xy[i]); err) return err;
            ++i;
        }
        if(i != 2) return simdjson::INCORRECT_TYPE;
        p = {xy[0], xy[1]};
        return simdjson::SUCCESS;
    }
};

// [x, y, w, h]
template <>
struct Adapter<QRectF> {
    static void write(Writer& sb, const QRectF& r) {
        sb.start_array();
        sb.append(r.x()), sb.append_comma();
        sb.append(r.y()), sb.append_comma();
        sb.append(r.width()), sb.append_comma();
        sb.append(r.height());
        sb.end_array();
    }
    static simdjson::error_code read(simdjson::ondemand::value& val, QRectF& r) {
        simdjson::ondemand::array arr;
        if(auto err = val.get_array().get(arr); err) return err;
        double v[4]{};
        size_t i{};
        for(auto elem: arr) {
            if(i == 4) return simdjson::CAPACITY;
            if(auto err = elem.get_double().get(v[i]); err) return err;
            ++i;
        }
        if(i != 4) return simdjson::INCORRECT_TYPE;
        r = {v[0], v[1], v[2], v[3]};
        return simdjson::SUCCESS;
    }
};

} // namespace Serial
