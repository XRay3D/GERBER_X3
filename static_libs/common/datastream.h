/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include <pfr.hpp>

#include <QDataStream>
#include <map>
#include <type_traits>

// #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
//  template <typename E, typename = std::enable_if_t<std::is_enuv_<E>>>
//  inline QDataStream& operator>>(QDataStream& s, E& e) {
//     qint32 i;
//     s >> i;
//     e = static_cast<E>(i);
//     return s;
// }

// template <typename E, typename = std::enable_if_t<std::is_enuv_<E>>>
// inline QDataStream& operator<<(QDataStream& s, E e) {
//     s << static_cast<qint32>(e);
//     return s;
// }
// #endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

template <class T1, class T2>
inline QDataStream& operator>>(QDataStream& s, std::pair<T1, T2>& p) {
    s >> p.first >> p.second;
    return s;
}

template <class T1, class T2>
inline QDataStream& operator<<(QDataStream& s, const std::pair<T1, T2>& p) {
    s << p.first << p.second;
    return s;
}

#endif

template <class T, size_t N>
inline QDataStream& operator>>(QDataStream& s, T (&p)[N]) {
    uint32_t n;
    s >> n;
    for (int i = 0; i < std::min<uint32_t>(n, N); ++i)
        s >> p[i];
    return s;
}

template <class T, size_t N>
inline QDataStream& operator<<(QDataStream& s, const T (&p)[N]) {
    s << uint32_t(N);
    for (auto& var : p)
        s << var;
    return s;
}

////////////////////////////////////////////////////////////////
/// std::vector<T>
///
template <typename T, class Alloc>
inline QDataStream& operator>>(QDataStream& stream, std::vector<T, Alloc>& container) {
    uint32_t n;
    stream >> n;
    container.reserve(n);
    while (n--) {
        T var;
        stream >> var;
        container.emplace_back(std::move(var));
        if (stream.status() != QDataStream::Ok)
            return container.clear(), stream;
    }
    return stream;
}

template <typename T, class Alloc>
inline QDataStream& operator<<(QDataStream& stream, const std::vector<T, Alloc>& container) {
    stream << uint32_t(container.size());
    for (const auto& var : container)
        stream << var;
    return stream;
}

////////////////////////////////////////////////////////////////
/// std::map<Key, T>
///
template <class Key, class Val, class Comp, class Alloc>
inline QDataStream& operator>>(QDataStream& stream, std::map<Key, Val, Comp, Alloc>& map) {
    // StreamStateSaver stateSaver(&s);
    map.clear();
    uint32_t n;
    stream >> n;
    while (n--) {
        Key key;
        Val val;
        stream >> key >> val;
        if (stream.status() != QDataStream::Ok)
            return map.clear(), stream;
        map.emplace(key, val);
    }
    return stream;
}

template <class Key, class Val, class Comp, class Alloc>
inline QDataStream& operator<<(QDataStream& stream, const std::map<Key, Val, Comp, Alloc>& map) {
    stream << uint32_t(map.size());
    // Deserialization should occur in the reverse order.
    // Otherwise, value() will return the least recently inserted
    // value instead of the most recently inserted one.

    auto it = map.cend();
    auto begin = map.cbegin();
    while (it != begin) {
        --it;
        stream << it->first << it->second;
    }
    return stream;
}

// порционное сохранение для гибкости.
class Block {
    QDataStream& stream;
    QByteArray data;

public:
    explicit Block(QDataStream& stream)
        : stream {stream} { }
#if 0
    template <typename T>
    QDataStream& read(T& val) {
        stream >> data;
        QDataStream in(&data, QIODevice::ReadOnly);
        pfr::for_each_field(val, [&in](auto& field) { !in.atEnd() ? in >> field : in; });
        return stream;
    }

    template <typename... Args>
    QDataStream& read(Args&... args) {
        stream >> data;
        QDataStream in(&data, QIODevice::ReadOnly);
        if constexpr (sizeof...(Args) == 1)
            pfr::for_each_field(args..., [&in](auto& field) { !in.atEnd() ? in >> field : in; });
        else
            ((!in.atEnd() ? in >> args : in), ...);
        return stream;
    }

    template <typename T>
    QDataStream& write(const T& val) {
        QDataStream out(&data, QIODevice::WriteOnly);
        pfr::for_each_field(val, [&out](const auto& field) {  out << field ; });
        return stream << data;
    }

    template <typename... Args>
    QDataStream& write(const Args&... args) {
        QDataStream out(&data, QIODevice::WriteOnly);
        ((out << args ), ...);
        return stream << data;
    }
#else
    template <typename... Args>
    QDataStream& read(Args&... args) {
        stream >> data;
        qDebug(__FUNCTION__ "%d", data.size());
        QDataStream in(&data, QIODevice::ReadOnly);
        if constexpr (sizeof...(Args) == 1)
            pfr::for_each_field(args..., [&in](auto& field) { !in.atEnd() ? in >> field : in; });
        else
            ((!in.atEnd() ? in >> args : in), ...);
        return stream;
    }

    template <typename... Args>
    QDataStream& write(const Args&... args) {
        QDataStream out(&data, QIODevice::WriteOnly);
        if constexpr (sizeof...(Args) == 1)
            pfr::for_each_field(args..., [&out](const auto& field) { out << field; });
        else
            ((out << args), ...);
        qDebug(__FUNCTION__ "%d", data.size());
        return stream << data;
    }
#endif

    template <typename... Args>
    QDataStream& rw(Args&... args) {
        if constexpr ((std::is_const_v<Args> && ... && true))
            return write(args...);
        else
            return read(args...);
    }
};

#define SERIALIZE_POD(TYPE)                                              \
    friend QDataStream& operator<<(QDataStream& stream, const TYPE& p) { \
        return ::Block(stream).write(p);                                 \
    }                                                                    \
    friend QDataStream& operator>>(QDataStream& stream, TYPE& p) {       \
        return ::Block(stream).read(p);                                  \
    }
