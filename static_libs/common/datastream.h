/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "mvector.h"

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

// template <typename T>
// inline QDataStream& operator>>(QDataStream& stream, mvector<T>& container) {
//     quint32 n;
//     stream >> n;
//     container.resize(n);
//     for (auto&& var : container) {
//         stream >> var;
//         if (stream.status() != QDataStream::Ok)
//             return container.clear(), stream;
//     }
//     return stream;
// }

// template <typename T>
// inline QDataStream& operator<<(QDataStream& stream, const mvector<T>& container) {
//     using Container = mvector<T>;
//     stream << quint32(container.size());
//     for (const auto& var : container)
//         stream << var;
//     return stream;
// }

////////////////////////////////////////////////////////////////
/// std::vector<T>
///
template <typename T>
inline QDataStream& operator>>(QDataStream& stream, std::vector<T>& container) {
    quint32 n;
    stream >> n;
    container.resize(n);
    for (auto&& var : container) {
        stream >> var;
        if (stream.status() != QDataStream::Ok)
            return container.clear(), stream;
    }
    return stream;
}

template <typename T>
inline QDataStream& operator<<(QDataStream& stream, const std::vector<T>& container) {
    stream << quint32(container.size());
    for (const auto& var : container)
        stream << var;
    return stream;
}

////////////////////////////////////////////////////////////////
/// std::map<Key, T>
///
template <class Key, class Val, class Comp>
inline QDataStream& operator>>(QDataStream& stream, std::map<Key, Val, Comp>& map) {
    // StreamStateSaver stateSaver(&s);
    map.clear();
    quint32 n;
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

template <class Key, class Val, class Comp>
inline QDataStream& operator<<(QDataStream& stream, const std::map<Key, Val, Comp>& map) {
    stream << quint32(map.size());
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
