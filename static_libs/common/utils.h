#pragma once

#include <QDebug>
#include <QMetaEnum>
#include <QString>
#include <chrono>
#include <concepts>
#include <map>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4117) // warning C4117: имя макроопределения "__cpp_consteval" зарезервировано, пропуск "#define"
#define __cpp_consteval 1 // NOTE костыли для clang tidy
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
        /**/ if constexpr(std::is_same_v<T, nS>)
            return ">>> %1.3f (avg %1.3f) nS\t -> %s";
        else if constexpr(std::is_same_v<T, uS>)
            return ">>> %1.3f (avg %1.3f) uS\t -> %s";
        else if constexpr(std::is_same_v<T, mS>)
            return ">>> %1.3f (avg %1.3f) mS\t -> %s";
        else if constexpr(std::is_same_v<T, Sec>)
            return ">>> %1.3f (avg %1.3f) S\t -> %s";
        else if constexpr(std::is_same_v<T, Mins>)
            return ">>> %1.3f (avg %1.3f) M\t -> %s";
        else if constexpr(std::is_same_v<T, Hours>)
            return ">>> %1.3f (avg %1.3f) H\t -> %s";
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
