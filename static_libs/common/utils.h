#pragma once

#include <QDebug>
#include <QMetaEnum>
#include <QString>
#include <chrono>
#include <concepts>
#include <map>
#include <variant>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4117) // warning C4117: имя макроопределения "__cpp_consteval" зарезервировано, пропуск "#define"
#define __cpp_consteval 1       // NOTE костыли для clang tidy
#endif

#include <source_location>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <string_view>

using nS = std::nano;
using uS = std::micro;
using mS = std::milli;
using Sec = std::ratio<1>;
using Mins = std::ratio<60>;
using Hours = std::ratio<3600>;

template <typename... Ts>
constexpr auto isSame = std::is_same_v<Ts...>;

template <class T = Sec>
    requires isSame<T, nS> || isSame<T, uS> || isSame<T, mS> || isSame<T, Sec> || isSame<T, Mins> || isSame<T, Hours>

struct Timer {
#if defined(__gnu_linux__) || defined(__GNUC__)
    const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t1;
#else
    const std::chrono::time_point<std::chrono::steady_clock> t1;
#endif
    const std::string_view stringView;
    static inline std::map<std::string_view, std::pair<size_t, double>> avgMap;

    constexpr Timer(std::string_view name, T = {})
        : t1{std::chrono::high_resolution_clock::now()}
        , stringView{name} {
    }

    constexpr Timer(T = {}, std::source_location sl = std::source_location::current())
        : Timer{sl.function_name()} { }

    ~Timer() {
        using std::chrono::duration;
        using std::chrono::high_resolution_clock;

        duration<double, T> timeout{high_resolution_clock::now() - t1};

        auto& [ctr, avg] = avgMap[stringView];
        avg += timeout.count();
        ++ctr;
        qDebug(format(), timeout.count(), avg / ctr, stringView.data());
    }

    constexpr auto format() const noexcept {
        if constexpr(std::is_same_v<T, nS>) return ">>> %1.3f (avg %1.3f) nS\t -> %s";
        else if constexpr(std::is_same_v<T, uS>) return ">>> %1.3f (avg %1.3f) uS\t -> %s";
        else if constexpr(std::is_same_v<T, mS>) return ">>> %1.3f (avg %1.3f) mS\t -> %s";
        else if constexpr(std::is_same_v<T, Sec>) return ">>> %1.3f (avg %1.3f) S\t -> %s";
        else if constexpr(std::is_same_v<T, Mins>) return ">>> %1.3f (avg %1.3f) M\t -> %s";
        else if constexpr(std::is_same_v<T, Hours>) return ">>> %1.3f (avg %1.3f) H\t -> %s";
    }
};
template <class T>
Timer(std::string_view, T) -> Timer<T>;

template <class T>
Timer(T) -> Timer<T>;

inline auto toU16StrView(const QString& str) {
    return std::u16string_view(reinterpret_cast<const char16_t*>(str.utf16()), str.size());
}

template <class T>
struct CtreCapTo {
    T& cap;
    constexpr CtreCapTo(T& cap) /*requires class ctre::captured_content<0,void>::storage<class std::_String_view_iterator<struct std::char_traits<char16_t>>>*/
        : cap{cap} {
    }

    auto toDouble() const { return toString().toDouble(); }
    auto toInt() const { return toString().toInt(); }
    auto toString() const {
        // qDebug("QString  D%d S%d", cap.data(), cap.size());
        return QString(reinterpret_cast<const QChar*>(cap.data()), static_cast<size_t>(cap.size()));
    }

    operator QString() const { return toString(); }
    operator double() const { return toDouble(); }
    operator int() const { return toInt(); }
    //    template <typename Ty>
    //    operator Ty() const {

    //        if constexpr (std::is_enuv_<Ty>)
    //            return static_cast<Ty>(toInt());
    //        if constexpr (std::is_integral_v<Ty>)
    //            return toInt();
    //        if constexpr (std::is_floating_point_v<Ty>)
    //            return toDouble();
    //        return toString();
    //    }
};
template <class T>
CtreCapTo(T) -> CtreCapTo<T>;

struct ScopedTrue {
    bool& fl;
    ScopedTrue(bool& fl)
        : fl{fl} { fl = true; }
    ~ScopedTrue() { fl = false; }
};

template <typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

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

template <typename Cap> concept CapContent = requires(Cap a) {
    std::is_pointer_v<decltype(a.data())>;
    { a.size() } -> std::convertible_to<size_t>;
    { a.operator bool() } -> std::convertible_to<bool>;
};

template <CapContent Cap>
QDebug operator<<(QDebug debug, Cap& cap) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "captured_content(" << QStringView(cap.data(), cap.size()) << ')';
    return debug;
}

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
        if(del == Delete)
            delete ptr;
    }
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QString>

template <size_t Size>
struct String {
    char16_t data[Size]{};
    size_t N;

    constexpr String(char16_t const (&str)[Size]) {
        N = Size;
        std::ranges::copy(str, data);
    };

    constexpr String(char const (&str)[Size]) {
        auto utf8{utf8toUtf16(str)};
        N = utf8.size();
        std::ranges::copy(utf8, data);
    };

    constexpr auto staticData() const { return staticData(std::make_index_sequence<Size>{}); };

    template <size_t Len>
    constexpr std::u16string utf8toUtf16(char const (&utf8)[Len]) {
        std::vector<uint32_t> unicode;
        size_t i{};

        auto error{"not a UTF-8 string"};
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
                utf16 += (char16_t)((uni >> 10) + 0xD800);   //   0b11011000'00000000
                utf16 += (char16_t)((uni & 0x3FF) + 0xDC00); // 0b1111111111 0b11011100'00000000
            }
        }
        return utf16;
    }

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
/*constexpr*/ auto operator"" _qs() noexcept {
    static const auto qstring_literal{Str.staticData()};
    return QString{{qstring_literal.data_ptr()}};
}
#else
using namespace QtLitherals;
#endif

#if USE_ENUM == 1
#include <ranges>

using namespace std::literals;
template <class Ty>
inline constexpr bool isEnum = false;
template <class Ty>
inline constexpr bool isBitField = false;
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
        val *= 10, val += var - '0';
    }
    return str.starts_with('-') ? -val : val;
};
consteval size_t enumSize(sv enums) {
    return std::ranges::count(enums, ',') + !enums.ends_with(',');
}
template <size_t N, class E>
consteval auto tokenize(sv base) {
    size_t count{};
    std::array<std::pair<sv, E>, N> tokens;
    std::underlying_type_t<E> val{};
    sv name;
    for(auto&& word: std::ranges::views::split(base, ", "sv)) {
        for(int i{}; auto&& tok: std::ranges::views::split(word, "="sv)) {
            sv token{tok.begin(), tok.end()};
            if(i++ == 0)
                name = trim(token);
            else if(token.size())
                val = toNum<E>(trim(token));
        }
        tokens[count++] = {name, static_cast<E>(val++)};
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
inline std::string arr;                          //[100] {};
template <class E>
    requires isEnum<E>
constexpr Impl::sv enumToString(E e) {
    auto it = std::ranges::find(Impl::Tokens<E>, e, &std::pair<Impl::sv, E>::second);
    if(it != Impl::Tokens<E>.end())
        return it->first;
    if constexpr(isBitField<E>) {
        std::string arr;
        std::back_insert_iterator bi(arr);
        using U = std::underlying_type_t<E>;
        for(auto&& [name, val]: Impl::Tokens<E>) {
            if(U(val) & U(e)) {
                if(arr.size())
                    arr += ", ";
                arr += name;
            }
        }
        return ::arr = "{ " + arr + " }";
    }
    return {};
}
template <class E>
    requires isEnum<E>
constexpr E stringToEnum(Impl::sv str) {
    auto it = std::ranges::find(Impl::Tokens<E>, str, &std::pair<Impl::sv, E>::first);
    return it == Impl::Tokens<E>.end() ? static_cast<E>(
               std::numeric_limits<std::underlying_type_t<E>>::min())
                                       : it->second;
}
#endif
