#pragma once

#include "geo/polygon.h"

#include <QColor>
#include <QDebug>
#include <QIcon>
#include <QMetaEnum>
#include <QString>
#include <chrono>
#include <concepts>
#include <qglobal.h>
#include <source_location>
#include <string_view>
#include <utility>
#include <variant>

using namespace std::literals;
using namespace std::placeholders;

using namespace Qt::Literals;

namespace r = std ::ranges;
namespace v = std ::views;

template <typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

template <typename Func>
struct Finaly final {
    Func const func;
    ~Finaly() { func(); }
};

using nS = std::nano;
using uS = std::micro;
using mS = std::milli;
using Sec = std::ratio<1>;
using Min = std::ratio<60>;
using Hour = std::ratio<3600>;

template <typename T, typename... Ts>
constexpr bool contains_v = std::disjunction_v<std::is_same<T, Ts>...>; // Or
template <typename T, typename... Ts> concept Contains = contains_v<T, Ts...>;

namespace chr = std::chrono;

template <Contains<nS, uS, mS, Sec, Min, Hour> T = Sec>
struct Timer {
    decltype(chr::high_resolution_clock::now()) t1;
    const std::string_view stringView;
    static inline std::unordered_map<std::string_view, std::pair<size_t, double>> avgMap;

    constexpr Timer(std::string_view name)
        : t1{chr::high_resolution_clock::now()}
        , stringView{name} {
    }

    constexpr Timer(std::source_location sl = std::source_location::current())
        : Timer{sl.function_name()} { }

    void now() {
        chr::duration<double, T> timeout{chr::high_resolution_clock::now() - t1};
        auto& [ctr, avg] = avgMap[stringView];
        avg += timeout.count();
        ++ctr;
        static constexpr Overload suffix{
            [](nS) { return "nS"; },
            [](uS) { return "uS"; },
            [](mS) { return "mS"; },
            [](Sec) { return "Sec"; },
            [](Min) { return "Min"; },
            [](Hour) { return "Hour"; },
        };
        qInfo(">>> %1.3f (avg %1.3f) %s\t -> %s\n",
            timeout.count(),
            avg / ctr,
            suffix(T{}),
            stringView.data());
        t1 = chr::high_resolution_clock::now();
    }
    ~Timer() { now(); }
};

using Timer_nS = Timer<nS>;
using Timer_uS = Timer<uS>;
using Timer_mS = Timer<mS>;
using TimerSec = Timer<Sec>;
using TimerMin = Timer<Min>;
using TimerHour = Timer<Hour>;

//------------------------------------------------------------------------------

template <class T>
struct CtreCapTo {
    T& cap;
    constexpr CtreCapTo(T& cap)
        : cap{cap} { }

    auto toDouble() const { return toString().toDouble(); }
    auto toInt() const { return toString().toInt(); }
    auto toString() const {
        // qDebug(u"QString  D%d S%d"_s, cap.data(), cap.size());
        return QString{
            reinterpret_cast<const QChar*>(cap.data()),
            static_cast<qsizetype>(cap.size()),
        };
    }

    operator QString() const { return toString(); }
    operator double() const { return toDouble(); }
    operator int() const { return toInt(); }
};
template <class T>
CtreCapTo(T) -> CtreCapTo<T>;

template <typename Cap> concept CapContent = requires(Cap a) {
    std::is_pointer_v<decltype(a.data())>;
    { a.size() } -> std::convertible_to<size_t>;
    { a.operator bool() } -> std::convertible_to<bool>;
};

template <CapContent Cap>
QDebug operator<<(QDebug debug, Cap& cap) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "captured_content(" << cap.view() << ')';
    return debug;
}

//------------------------------------------------------------------------------

inline constexpr double normalizeAngleDegrees(double angle) noexcept {
    return 360.0 - (angle > 180.0 ? angle - 180.0 : angle + 180.0);
}

inline constexpr bool contains(auto var, auto... vars) noexcept {
    return ((vars == var) || ...);
}

template <auto... vars>
inline constexpr bool contains(auto var) noexcept {
    return ((vars == var) || ...);
}

//------------------------------------------------------------------------------

struct ScopedTrue {
    bool& fl;
    ScopedTrue(bool& fl)
        : fl{fl} { fl = true; }
    ~ScopedTrue() { fl = false; }
};

//------------------------------------------------------------------------------

template <typename... Ts>
struct Variant : std::variant<Ts...> {
    using std::variant<Ts...>::variant;
    using variant = std::variant<Ts...>;
    template <typename Func>
    auto visit(Func&& func) {
        return std::visit(std::forward<Func>(func), *this);
    }
    template <typename Func>
    auto visit(Func&& func) const {
        return std::visit(std::forward<Func>(func), *this);
    }
    template <typename... Funcs>
    auto visit(Funcs&&... funcs) {
        return std::visit(Overload{std::forward<Funcs>(funcs)...}, *this);
    }
    template <typename... Funcs>
    auto visit(Funcs&&... funcs) const {
        return std::visit(Overload{std::forward<Funcs>(funcs)...}, *this);
    }
    bool has_value() const { return Variant::index() != std::variant_npos; }
    operator bool() const { return has_value(); }
};

//------------------------------------------------------------------------------

namespace EnumHelper {
// Tool to convert enum values to/from QString
template <typename E>
E fromString(const QString& text) {
    bool ok;
    auto result = static_cast<E>(QMetaEnum::fromType<E>().keyToValue(text.toUtf8(), &ok));
    if(!ok) {
        qDebug() << "Failed to convert enum" << text;
        return {};
    }
    return result;
}

struct fromString {
    const QString& text;
    template <typename E>
    operator E() {
        bool ok;
        auto result = static_cast<E>(QMetaEnum::fromType<E>().keyToValue(text.toUtf8(), &ok));
        if(ok) [[likely]]
            return result;
        qDebug() << "Failed to convert enum" << text;
        return {};
    }
};

template <typename E>
QString toString(E value) {
    const int intValue = static_cast<int>(value);
    return QString::fromUtf8(QMetaEnum::fromType<E>().valueToKey(intValue));
}
} // namespace EnumHelper

struct Deleter {
    enum Polycy {
        DontDelete,
        Delete
    } del{Delete};
    void operator()(auto* ptr) const {
        if(del == Delete) delete ptr;
    }
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #include <QString>

template <size_t Len>
inline constexpr auto utf8toUtf16(char const (&utf8)[Len]) {
    std::vector<uint32_t> unicode;
    size_t i{};

    auto error{u"not a UTF-8 string"_s};
    while(i < Len) {
        unsigned long uni;
        size_t todo;
        unsigned char ch = utf8[i++];

        if(ch <= 0x7F) { // 0b01111111
            uni = ch;
            todo = 0;
        } else if(ch <= 0xBF) // 0b10111111
            throw error;
        else if(ch <= 0xDF) { // 0b11011111
            uni = ch & 0x1F;  // 0b00011111
            todo = 1;
        } else if(ch <= 0xEF) { // 0b11101111
            uni = ch & 0x0F;    // 0b00001111
            todo = 2;
        } else if(ch <= 0xF7) { // 0b11110111
            uni = ch & 0x07;    // 0b00000111
            todo = 3;
        } else
            throw error;

        for(size_t j{}; j < todo; ++j) {
            if(i == Len) throw error;
            unsigned char ch = utf8[i++];
            if(ch < 0x80 || ch > 0xBF) throw error; // 0b10000000  0b10111111
            uni <<= 6;
            uni += ch & 0x3F; // 0b00111111
        }
        if(uni >= 0xD800 && uni <= 0xDFFF) throw error; // 0b11011000'00000000 0b11011111'11111111
        if(uni > 0x10FFFF) throw error;                 // 0b10000'11111111'11111111

        unicode.push_back(uni);
    }
    std::u16string utf16;
    for(size_t i{}; i < unicode.size(); ++i) {
        unsigned long uni = unicode[i];
        if(uni <= 0xFFFF) { // 0b11111111'11111111
            utf16 += (char16_t)uni;
        } else {
            uni -= 0x10000;                              // 0b1'00000000'00000000
            utf16 += (char16_t)((uni >> 10) + 0xD800);   // 0b11011000'00000000
            utf16 += (char16_t)((uni & 0x3FF) + 0xDC00); // 0b1111111111 0b11011100'00000000
        }
    }
    return utf16;
}

template <size_t Size>
struct String {
    char16_t data[Size]{};
    size_t N;

    constexpr String(char16_t const (&str)[Size]) {
        N = Size;
        r::copy(str, data);
    };

    constexpr String(char const (&str)[Size]) {
        auto utf8{utf8toUtf16(str)};
        N = utf8.size();
        r::copy(utf8, data);
    };

    constexpr auto staticData() const { return staticData(std::make_index_sequence<Size>{}); };

private:
    template <std::size_t... Is>
    constexpr auto staticData(std::index_sequence<Is...>) const {
        return QStaticStringData<Size>{
            QArrayData{{-1}, static_cast<int>(N - 1), 0, 0, sizeof(QStringData)},
            {data[Is]...},
        };
    };
};

// template <size_t N>
// String(char16_t const (&)[N]) -> String<N>;

// template <size_t N>
// String(char8_t const (&)[N]) -> String<N>;

template <String Str>
/*constexpr*/ auto operator""_s() noexcept {
    static const auto qstring_literal{Str.staticData()};
    return QString{{qstring_literal.data_ptr()}};
}
#else
using namespace QtLiterals;
using namespace Qt::Literals;
#endif

#if USE_ENUM == 1
    #include <array>
    #include <ranges>

using namespace std::literals;
template <class Ty>
inline constexpr bool isEnum{};
template <class Ty>
inline constexpr bool isBitField{};
namespace Impl {
template <class Ty>
inline constexpr Ty Max = Ty{};
template <class Ty>
inline constexpr Ty Tokens = Ty{};
using sv = std::string_view;
consteval auto trim(sv str) {
    auto isSpaceOrSep = [](auto ch) {
        return ch == ' ' || ch == ','; // || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v';
    };
    while(isSpaceOrSep(str.front()))
        str = str.substr(1);
    while(isSpaceOrSep(str.back()))
        str = str.substr(0, str.size() - 1);
    return str;
};
template <class E>
consteval auto toNum(sv str) {
    std::underlying_type_t<E> val{};
    for(auto var: str) {
        if(var == '-')
            continue;
        template <typename T>
        Cast(T && arg) -> Cast<decltype(arg)>;

        template <typename T>
        template <typename To>
        constexpr Cast<T>::operator To() const { return static_cast<To>(val); }

        template <>
        template <typename To>
        constexpr Cast<QVariant>::operator To() const { return val.value<To>(); }
        val *= 10, val += var - '0';
    }
    return str.starts_with('-') ? -val : val;
};
consteval size_t enumSize(sv enums) {
    return r::count(enums, ',') + !enums.ends_with(',');
}
template <size_t N, class E>
consteval auto tokenize(sv base) {
    size_t count{};
    std::array<std::pair<sv, E>, N> tokens;
    std::underlying_type_t<E> val{};
    sv name;
    for(auto&& word: r::views::split(base, u", "_ssv)) {
        for(int i{}; auto&& tok: r::views::split(word, uu "="_ssv)) {
            sv token{tok.begin(), tok.end()};
            if(i++ == 0)
                name = trim(token);
            else if(token.size())
                val = toNum<E>(trim(token));
        }
        tokens[count++]{name, static_cast<E>(val++)};
    }
    return tokens;
}
template <size_t N, class E>
consteval bool isBitField(std::array<std::pair<sv, E>, N> tokens) {
    std::underlying_type_t<E> checker{};
    for(auto [name, val]: tokens)
        checker ^= std::underlying_type_t<E>(val);
    return checker == (1 << N) - 1;
}
} // namespace Impl
    #define ENUM(E, ...)                                                                       \
        enum class E : int {                                                                   \
            __VA_ARGS__                                                                        \
        };                                                                                     \
        template <>                                                                            \
        inline constexpr bool isEnum<E> = true;                                                \
        template <>                                                                            \
        inline constexpr auto Impl::Max<E> = Impl::enumSize(#__VA_ARGS__);                     \
        template <>                                                                            \
        inline constexpr auto Impl::Tokens<E> = Impl::tokenize<Impl::Max<E>, E>(#__VA_ARGS__); \
        template <>                                                                            \
        inline constexpr auto isBitField<E> = false; // RImpl::isBitField(Impl::Tokens<E>);
inline std::string arr;                              //[100] {};
template <class E>
    requires isEnum<E>
constexpr Impl::sv enumToString(E e) {
    auto it = r::find(Impl::Tokens<E>, e, &std::pair<Impl::sv, E>::second);
    if(it != Impl::Tokens<E>.end())
        return it->first;
    if constexpr(isBitField<E>) {
        std::string arr;
        std::back_insert_iterator bi(arr);
        using U = std::underlying_type_t<E>;
        for(auto&& [name, val]: Impl::Tokens<E>) {
            if(U(val) & U(e)) {
                if(arr.size())
                    arr += u", "_s;
                arr += name;
            }
        }
        return ::arr = u"{ "_s + arr + u" }"_s;
    }
    return {};
}
template <class E>
    requires isEnum<E>
constexpr E stringToEnum(Impl::sv str) {
    auto it = r::find(Impl::Tokens<E>, str, &std::pair<Impl::sv, E>::first);
    return it == Impl::Tokens<E>.end() ? static_cast<E>(
                                             std::numeric_limits<std::underlying_type_t<E>>::min())
                                       : it->second;
}
#endif

QString toQString(std::string_view cp1251Str);

std::string toCp1251(const QString& utf16Str);

// QByteArray toUtf8(std::string_view cp1251Str);

void detectEncoding(std::string_view data);

/* ---------- 1. Проверка BOM  ------------------------------------------ */
bool hasBom(std::string_view data);

/* ---------- 2. Проверка валидности UTF‑8 ------------------------------ */
bool isValidUtf8(std::string_view data) noexcept;

//------------------------------------------------------------------------------

#if 0
template <typename T>
struct Cast final {
    T val;
    template <typename To>
    constexpr operator To() const {
        if constexpr(std::is_same_v<std::remove_cvref_t<T>, QVariant>)
            return val.template value<To>();
        else
            return static_cast<To>(val);
    }
};
template <typename T>
Cast(T&& arg) -> Cast<decltype(arg)>;
#else
template <typename T>
struct Cast final {
    const T& val;
    template <typename To>
    constexpr operator To() const;
};

template <typename T>
template <typename To>
constexpr Cast<T>::operator To() const { return static_cast<To>(val); }

template <>
template <typename To>
constexpr Cast<QVariant>::operator To() const { return val.value<To>(); }

#endif
//------------------------------------------------------------------------------
template <typename E> concept Enum = std::is_enum_v<E>;

// template <typename E> concept Integral = std::is_integral_v<E>;

inline namespace EnumOps {

#if __cpp_lib_to_underlying >= 202102L
using std::to_underlying;
#else
template <Enum E>
[[nodiscard]] constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
#endif

#define DECLARE_VIEWS_IOTA(ENUM)                                        \
    template <>                                                         \
    struct std::incrementable_traits<ENUM> {                            \
        using difference_type = make_signed_t<underlying_type_t<ENUM>>; \
    };

template <Enum E>
[[nodiscard]] constexpr auto operator+(E left) noexcept { return to_underlying(left); }

template <Enum E>
[[nodiscard]] constexpr E& operator++(E& e) noexcept { return e = static_cast<E>(+e + 1); }

template <Enum E>
[[nodiscard]] constexpr E operator++(E& e, int) noexcept { return std::exchange(e, static_cast<E>(+e + 1)); }

template <Enum E>
[[nodiscard]] constexpr E& operator--(E& e) noexcept { return e = static_cast<E>(+e - 1); }

template <Enum E>
[[nodiscard]] constexpr E operator--(E& e, int) noexcept { return std::exchange(e, static_cast<E>(+e - 1)); }

// Enum OP Enum
template <Enum L, Enum R>
[[nodiscard]] constexpr L operator&(L left, R right) noexcept { return static_cast<L>(+left & +right); }

template <Enum L, Enum R>
[[nodiscard]] constexpr L operator^(L left, R right) noexcept { return static_cast<L>(+left ^ +right); }

template <Enum L, Enum R>
[[nodiscard]] constexpr L operator|(L left, R right) noexcept { return static_cast<L>(+left | +right); }

template <Enum L, Enum R>
[[nodiscard]] constexpr L operator+(L left, R right) noexcept { return static_cast<L>(+left | +right); }

template <Enum E>
[[nodiscard]] constexpr E operator~(E left) noexcept { return static_cast<E>(~+left); }

// Enum OP= Enum
template <Enum L, Enum R>
/*[[nodiscard]]*/ constexpr const L& operator&=(L& left, R right) noexcept { return left = left & right; }

template <Enum L, Enum R>
/*[[nodiscard]]*/ constexpr const L& operator^=(L& left, R right) noexcept { return left = left ^ right; }

template <Enum L, Enum R>
/*[[nodiscard]]*/ constexpr const L& operator|=(L& left, R right) noexcept { return left = left | right; }

#if 0 //  Enum and Integral
// Enum OP Integral
template <Enum L, Integral R>
[[nodiscard]] constexpr L operator&(L left, R right) noexcept { return static_cast<L>(+left & right); }

template <Enum L, Integral R>
[[nodiscard]] constexpr L operator^(L left, R right) noexcept { return static_cast<L>(+left ^ right); }

template <Enum L, Integral R>
[[nodiscard]] constexpr L operator|(L left, R right) noexcept { return static_cast<L>(+left | right); }

// Integral OP Enum
template <Integral L, Enum R>
[[nodiscard]] constexpr R operator&(L left, R right) noexcept { return static_cast<R>(left & +right); }

template <Integral L, Enum R>
[[nodiscard]] constexpr R operator^(L left, R right) noexcept { return static_cast<R>(left ^ +right); }

template <Integral L, Enum R>
[[nodiscard]] constexpr R operator|(L left, R right) noexcept { return static_cast<R>(left | +right); }


// Enum OP= Integral
template <Enum L, Integral R>
[[nodiscard]] constexpr const L& operator&=(L& left, R right) noexcept { return left = left & right; }

template <Enum L, Integral R>
[[nodiscard]] constexpr const L& operator^=(L& left, R right) noexcept { return left = left ^ right; }

template <Enum L, Integral R>
[[nodiscard]] constexpr const L& operator|=(L& left, R right) noexcept { return left = left | right; }

#endif

} // namespace EnumOps

template <Enum E>
constexpr bool HasAllFlags(E value, E flags) noexcept { return (+value & +flags) == +flags; }

template <Enum E>
constexpr bool HasAnyFlag(E value, E flags) noexcept { return (+value & +flags) != 0; }

#if TEST
namespace Test {

using namespace Zhele::EnumOps;

enum class E : signed char {
    A,
    B,
    C,
    D
};

static_assert((E::B & E::C) == E::A);
static_assert((E::B ^ E::C) == E::D);
static_assert((E::B | E::C) == E::D);

static_assert((E::B & 2) == E::A);
static_assert((E::B ^ 2) == E::D);
static_assert((E::B | 2) == E::D);

static_assert((1 & E::C) == E::A);
static_assert((1 ^ E::C) == E::D);
static_assert((1 | E::C) == E::D);

static_assert(~E::A == static_cast<E>(-1));
static_assert(Zhele::HasAllFlags(E::D, E::B | E::C));
static_assert(Zhele::HasAnyFlag(E::D, E::C));

static_assert([](E e) consteval { return e &= 2; }(E::B) == E::A);
static_assert([](E e) consteval { return e ^= 2; }(E::B) == E::D);
static_assert([](E e) consteval { return e |= 2; }(E::B) == E::D);

static_assert([](E e) consteval { return e++; }(E::B) == E::B);
static_assert([](E e) consteval { return e--; }(E::B) == E::B);
static_assert([](E e) consteval { return ++e; }(E::B) == E::C);
static_assert([](E e) consteval { return --e; }(E::B) == E::A);
static_assert([](E e) consteval { auto _ = e++; return e; }(E::B) == E::C);
static_assert([](E e) consteval { auto _ = e--; return e; }(E::B) == E::A);

} // namespace Test
#endif
//------------------------------------------------------------------------------
// reflect_print.hpp — универсальный форматтер структур и классов
// на рефлексии C++26 (P2996 + P3491 define_static_array).
//
// Компиляторы:
//   * GCC trunk (16.x):    g++ -std=c++26
//   * clang-p2996:         clang++ -std=c++26 -freflection-latest -stdlib=libc++
//
// Возможности:
//   * произвольные структуры/классы, включая приватные поля и базовые классы;
//   * enum / enum class — печать по имени енумератора;
//   * контейнеры (ranges), map-подобные, tuple/pair, optional, variant,
//     умные и сырые указатели;
//   * плоский и многострочный (pretty) режимы;
//   * интеграция со std::format / std::print: "{}" — плоско, "{:#}" — pretty.
//
// Ограничения (осознанные):
//   * по умолчанию указатели (сырые и умные) печатаются адресом;
//     options{.deref = true} (или спецификатор "{:*}") разыменовывает их
//     и печатает поля как "0xADDR -> Type{...}"; от циклов защищает
//     options::max_depth — глубже него снова печатается только адрес;
//   * анонимные члены печатаются как <anon>, union — как <union>.
// ============================================================================

#include <cstdint>
#include <format>
#include <memory>
#include <meta>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace rfl {

struct options {
    bool pretty = false; // многострочный вывод с отступами
    int indent = 2;      // ширина одного уровня отступа
    bool deref = false;  // разыменовывать ненулевые указатели и
                         // печатать поля: "0x... -> Type{...}"
    int max_depth = 32;  // предохранитель: глубже этого указатели
                         // печатаются адресом (защита от циклов)
};

// ---------------------------------------------------------------------------
// Классификация типов
// ---------------------------------------------------------------------------
namespace detail {

template <class T, template <class...> class P>
inline constexpr bool is_specialization_v = false;
template <template <class...> class P, class... A>
inline constexpr bool is_specialization_v<P<A...>, P> = true;

} // namespace detail

template <class T> concept CharString = std::same_as<T, std::string> || std::same_as<T, std::string_view> || std::same_as<std::decay_t<T>, char*> || std::same_as<std::decay_t<T>, const char*>;

template <class T> concept Range = std::ranges::input_range<T> && !CharString<T>;

template <class T> concept TupleLike = requires { std::tuple_size<T>::value; } && !Range<T>;

template <class T> concept OptionalLike = detail::is_specialization_v<T, std::optional>;

template <class T> concept VariantLike = detail::is_specialization_v<T, std::variant>;

template <class T> concept SmartPtr = detail::is_specialization_v<T, std::unique_ptr> || detail::is_specialization_v<T, std::shared_ptr>;

// Класс, который печатаем через рефлексию (всё остальное отсеяно выше)
template <class T> concept ReflectClass = std::is_class_v<T> && !CharString<T> && !Range<T> && !TupleLike<T> && !OptionalLike<T> && !VariantLike<T> && !SmartPtr<T>;

// ---------------------------------------------------------------------------
// Обход членов через рефлексию.
// Вместо expansion statements (template for) — раскрытие пачки индексов,
// а std::meta::info передаётся как NTTP (он structural). Работает и в GCC
// trunk, и в clang-p2996.
// ---------------------------------------------------------------------------
namespace detail {

// unchecked() — видим в т.ч. приватные члены; замените на
// access_context::current(), если нужны только доступные из этого контекста.
consteval auto ctx() { return std::meta::access_context::unchecked(); }

template <class T, class F>
constexpr void for_each_member(F&& f) {
    constexpr auto members = std::define_static_array(
        std::meta::nonstatic_data_members_of(^^T, ctx()));
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (f.template operator()<members[I]>(), ...);
    }(std::make_index_sequence<members.size()>{});
}

template <class T, class F>
constexpr void for_each_base(F&& f) {
    constexpr auto bases = std::define_static_array(std::meta::bases_of(^^T, ctx()));
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (f.template operator()<bases[I]>(), ...);
    }(std::make_index_sequence<bases.size()>{});
}

} // namespace detail

// ---------------------------------------------------------------------------
// Имя енумератора по значению
// ---------------------------------------------------------------------------
template <class E>
    requires std::is_enum_v<E>
std::string enum_to_string(E value) {
    constexpr auto enums = std::define_static_array(std::meta::enumerators_of(^^E));
    std::string result;
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        (void)((value == [:enums[I]:]
                   ? (result = std::meta::identifier_of(enums[I]), true)
                   : false)
            || ...);
    }(std::make_index_sequence<enums.size()>{});
    if(result.empty()) // значение вне списка енумераторов (флаги и т.п.)
        return std::format("{}({})", std::meta::display_string_of(^^E),
            std::to_underlying(value));
    return std::format("{}::{}", std::meta::display_string_of(^^E), result);
}

// ---------------------------------------------------------------------------
// Главный диспетчер
// ---------------------------------------------------------------------------
template <class T>
std::string to_string(const T& value, options opt = {}, int depth = 0);

namespace detail {

inline std::string ind(const options& o, int depth) {
    return o.pretty ? "\n" + std::string(std::size_t(depth) * o.indent, ' ')
                    : std::string{};
}

template <class T>
std::string class_to_string(const T& value, options opt, int depth) {
    std::string out{std::meta::display_string_of(^^T)};
    out += '{';
    bool first = true;
    auto sep = [&]() -> std::string {
        std::string s = first ? "" : (opt.pretty ? "," : ", ");
        first = false;
        return s + ind(opt, depth + 1);
    };

    // Базовые классы — печатаем их поля как вложенный объект
    for_each_base<T>([&]<std::meta::info B>() {
        using Base = [:std::meta::type_of(B):];
        out += sep();
        out += class_to_string(static_cast<const Base&>(value), opt, depth + 1);
    });

    // Собственные нестатические члены
    for_each_member<T>([&]<std::meta::info M>() {
        out += sep();
        if constexpr(std::meta::has_identifier(M))
            out += std::meta::identifier_of(M);
        else
            out += "<anon>";
        out += ": ";
        out += rfl::to_string(value.[:M:], opt, depth + 1);
    });

    if(!first) out += ind(opt, depth);
    out += '}';
    return out;
}

} // namespace detail

template <class T>
std::string to_string(const T& value, options opt, int depth) {
    if constexpr(std::same_as<T, bool>) {
        return value ? "true" : "false";
    } else if constexpr(std::same_as<T, char>) {
        return std::format("'{}'", value);
    } else if constexpr(std::is_arithmetic_v<T>) {
        return std::format("{}", value);
    } else if constexpr(CharString<T>) {
        if constexpr(std::is_pointer_v<std::decay_t<T>>)
            if(value == nullptr) return "null";
        return std::format("\"{}\"", std::string_view(value));
    } else if constexpr(std::is_enum_v<T>) {
        return enum_to_string(value);
    } else if constexpr(std::is_pointer_v<T>) {
        using P = std::remove_cv_t<std::remove_pointer_t<T>>;
        if(!value) return "null";
        if constexpr(std::is_function_v<P>) {
            // static_cast функционального указателя в void* нелегален
            return std::format("{}", reinterpret_cast<const void*>(value));
        } else {
            std::string addr = std::format("{}", static_cast<const void*>(value));
            if constexpr(!std::is_void_v<P>) {
                if(opt.deref && depth < opt.max_depth)
                    return addr + " -> " + to_string(*value, opt, depth + 1);
            }
            return addr;
        }
    } else if constexpr(SmartPtr<T>) {
        if(!value) return "null";
        std::string addr = std::format("{}", static_cast<const void*>(value.get()));
        if(opt.deref && depth < opt.max_depth)
            return addr + " -> " + to_string(*value, opt, depth + 1);
        return addr;
    } else if constexpr(OptionalLike<T>) {
        return value ? to_string(*value, opt, depth) : "nullopt";
    } else if constexpr(VariantLike<T>) {
        return std::visit(
            [&](const auto& v) { return to_string(v, opt, depth); }, value);
    } else if constexpr(TupleLike<T>) {
        std::string out = "(";
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ((out += (I ? ", " : "") + to_string(std::get<I>(value), opt, depth)),
                ...);
        }(std::make_index_sequence<std::tuple_size_v<T>>{});
        return out + ")";
    } else if constexpr(Range<T>) {
        constexpr bool is_map = requires {
            typename T::key_type;
            typename T::mapped_type;
        };
        std::string out = is_map ? "{" : "[";
        bool first = true;
        for(const auto& e: value) {
            if(!first) out += ", ";
            first = false;
            if constexpr(is_map) {
                out += to_string(e.first, opt, depth) + ": " + to_string(e.second, opt, depth);
            } else {
                out += to_string(e, opt, depth);
            }
        }
        return out + (is_map ? "}" : "]");
    } else if constexpr(std::is_union_v<T>) {
        return "<union>";
    } else if constexpr(ReflectClass<T>) {
        return detail::class_to_string(value, opt, depth);
    } else {
        return "<?>";
    }
}

// ---------------------------------------------------------------------------
// Интеграция со std::format / std::print
//
// По умолчанию включаем только для агрегатов, не являющихся диапазонами
// и tuple-like — чтобы не конфликтовать со стандартными формattерами
// диапазонов (C++23) и pair/tuple. Для неагрегатных классов можно явно
// включить: template <> constexpr bool rfl::format_enabled<MyClass> = true;
// ---------------------------------------------------------------------------
template <class T>
constexpr bool format_enabled = ReflectClass<T> && std::is_aggregate_v<T>;

} // namespace rfl

template <class T>
    requires rfl::format_enabled<T>
struct std::formatter<T, char> {
    rfl::options opt{};

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        for(; it != ctx.end() && *it != '}'; ++it) {
            if(*it == '#') opt.pretty = true;     // "{:#}" — pretty
            else if(*it == '*') opt.deref = true; // "{:*}" — deref
            else
                throw std::format_error(
                    "rfl: поддерживаются только '#' и '*'");
        }
        return it;
    }

    template <class FmtCtx>
    auto format(const T& value, FmtCtx& ctx) const {
        return std::ranges::copy(rfl::to_string(value, opt), ctx.out()).out;
    }
};

// ---------------------------------------------------------------------------
#if 0
// Специализация std::formatter для ЛЮБОГО указателя на объект.
// Делает легальным std::format("{}", ptr) для App* и т.п.
//
//   "{}"    -> адрес (или "null")
//   "{:*}"  -> адрес -> Type{поля...} (рефлексия, рекурсивно)
//   "{:#*}" -> то же, многострочно
//
// void*, const void*, char*, const char*, nullptr_t не задеваются: для них
// в стандартной библиотеке есть ПОЛНЫЕ специализации, а полная
// специализация всегда предпочтительнее частичной. Указатели на функции
// исключены (их печатает ветка is_function_v в rfl::to_string при
// необходимости, но formatter для них стандарт не разрешает подменять
// осмысленно).
// ---------------------------------------------------------------------------
template <class T>
    requires(!std::is_function_v<T>)
struct std::formatter<T*, char> {
    rfl::options opt{};

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        for(; it != ctx.end() && *it != '}'; ++it) {
            if(*it == '#') opt.pretty = true;
            else if(*it == '*') opt.deref = true;
            else
                throw std::format_error(
                    "rfl: поддерживаются только '#' и '*'");
        }
        return it;
    }

    template <class FmtCtx>
    auto format(T* value, FmtCtx& ctx) const {
        return std::ranges::copy(rfl::to_string(value, opt), ctx.out()).out;
    }
};
// template <typename T>
// inline QDebug operator<<(QDebug debug, const T& data) {
//     return debug.noquote() << std::format("{}", data);
// }
#endif
// ---------------------------------------------------------------------------

// Ищет val в range. Работает как по значениям, так и по контейнерам умных
// указателей (сравнение идёт с elem.get()). Возвращает -1, если не найдено.
constexpr std::ptrdiff_t indexOf(const std::ranges::range auto& range, const auto& val) {
    auto it = r::find_if(range, [&val](const auto& elem) -> bool {
        if constexpr(requires { { elem.get() == val } -> std::convertible_to<bool>; })
            return elem.get() == val;
        else
            return elem == val;
    });
    return (it != r::end(range)) ? std::distance(r::begin(range), it) : -1;
}

constexpr auto takeAt(std::ranges::range auto& range, size_t idx)
    -> std::remove_cvref_t<decltype(range[0])> {
    if(r::begin(range) + idx >= r::end(range))
        return {};
    auto r(std::move(*(r::begin(range) + idx)));
    range.erase(r::begin(range) + idx);
    return r;
}

constexpr int IconSize = 24;

QIcon drawIcon(const QPainterPath& pPath, QColor color = Qt::black, bool stroke = false);

// Иконки по геометрии. Живут здесь, а не в Geo: та про геометрию и вычисления,
// про QPixmap с QPainter ей знать незачем.
QIcon drawIcon(const Geo::Polylines& polylines, QColor color = Qt::black, bool stroke = false);
QIcon drawIcon(const Geo::Polygons& polygons, QColor color = Qt::black);

QIcon drawDrillIcon(QColor color);
