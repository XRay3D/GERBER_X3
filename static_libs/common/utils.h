#pragma once

#include <QDebug>
#include <QMetaEnum>
#include <QString>
#include <chrono>
#include <concepts>
#include <map>
#include <string_view>

using ns_ = std::nano;
using us_ = std::micro;
using ms_ = std::milli;
using seconds__ = std::ratio<1>;
using minutes__ = std::ratio<60>;
using hours__ = std::ratio<3600>;
template <class T = seconds__>
    requires                        //
    std::is_same_v<T, ns_> ||       //
    std::is_same_v<T, us_> ||       //
    std::is_same_v<T, ms_> ||       //
    std::is_same_v<T, seconds__> || //
    std::is_same_v<T, minutes__> || //
    std::is_same_v<T, hours__>      //

struct Timer {
#if defined(__gnu_linux__) || defined(__GNUC__)
    const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t1;
#else
    const std::chrono::time_point<std::chrono::steady_clock> t1;
#endif
    const std::string_view stringView;
    static inline std::map<std::string_view, std::pair<size_t, double>> avgMap;

    constexpr Timer(std::string_view name, T = {})
        : t1 {std::chrono::high_resolution_clock::now()}
        , stringView {name} {
    }

    ~Timer() {
        using std::chrono::duration;
        using std::chrono::high_resolution_clock;

        duration<double, T> timeout {high_resolution_clock::now() - t1};

        auto& [ctr, avg] = avgMap[stringView];
        avg += timeout.count();
        ++ctr;

        /**/ if constexpr (std::is_same_v<T, ns_>)
            qDebug("\t%10s -> %1.3f (avg %1.3f) nS", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, us_>)
            qDebug("\t%10s -> %1.3f (avg %1.3f) uS", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, ms_>)
            qDebug("\t%10s -> %1.3f (avg %1.3f) mS", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, seconds__>)
            qDebug("\t%10s -> %1.3f (avg %1.3f) S", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, minutes__>)
            qDebug("\t%10s -> %1.3f (avg %1.3f) M", stringView.data(), timeout.count(), avg / ctr);
        else if constexpr (std::is_same_v<T, hours__>)
            qDebug("\t%10s -> %1.3f (avg %1.3f) H", stringView.data(), timeout.count(), avg / ctr);
    }
};
template <class T>
Timer(std::string_view, T) -> Timer<T>;

inline auto toU16StrView(const QString& str) {
    return std::u16string_view(reinterpret_cast<const char16_t*>(str.utf16()), str.size());
}

template <class T>
struct CtreCapTo {
    T& cap;
    constexpr CtreCapTo(T& cap) /*requires class ctre::captured_content<0,void>::storage<class std::_String_view_iterator<struct std::char_traits<char16_t>>>*/
        : cap {cap} {
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
    //        qDebug() << __FUNCSIG__;
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
        : fl {fl} { fl = true; }
    ~ScopedTrue() { fl = false; }
};

template <typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

template <typename Cap>
concept CapContent = requires(Cap a) {
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
    if (!ok) {
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
        if (ok) [[likely]]
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
