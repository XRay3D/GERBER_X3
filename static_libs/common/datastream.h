/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#pragma once

#include "mvector.h"

#include <QDataStream>
#include <map>
#include <type_traits>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
inline QDataStream& operator>>(QDataStream& s, E& e) {
    qint32 i;
    s >> i;
    e = static_cast<E>(i);
    return s;
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
inline QDataStream& operator<<(QDataStream& s, E e) {
    s << static_cast<qint32>(e);
    return s;
}
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 99, 99)
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

////////////////////////////////////////////////////////////////
/// std::map<Key, T, std::greater<int>>
///
// template <class Key, class T>
// inline QDataStream& operator>>(QDataStream& s, std::map<Key, T, std::greater<int>>& map)
//{
//     //StreamStateSaver stateSaver(&s);
//     using Container = std::map<Key, T>;
//     map.clear();
//     quint32 n;
//     s >> n;
//     for (quint32 i = 0; i < n; ++i) {
//         typename Container::key_type k;
//         typename Container::mapped_type t;
//         s >> k >> t;
//         if (s.status() != QDataStream::Ok) {
//             map.clear();
//             break;
//         }
//         map.emplace(k, t);
//     }
//     return s;
// }

// template <class Key, class T>
// inline QDataStream& operator<<(QDataStream& s, const std::map<Key, T, std::greater<int>>& map)
//{
//     s << quint32(map.size());
//     // Deserialization should occur in the reverse order.
//     // Otherwise, value() will return the least recently inserted
//     // value instead of the most recently inserted one.
//     auto it = map.cend();
//     auto begin = map.cbegin();
//     while (it != begin) {
//         --it;
//         s << it->first << it->second;
//     }
//     return s;
// }
#endif

template <typename T>
inline QDataStream& operator>>(QDataStream& s, mvector<T>& c) {
    using Container = mvector<T>;
    c.clear();
    quint32 n;
    s >> n;
    c.reserve(n);
    for (quint32 i = 0; i < n; ++i) {
        typename Container::value_type t;
        s >> t;
        if (s.status() != QDataStream::Ok) {
            c.clear();
            break;
        }
        c.push_back(t);
    }
    return s;
}

template <typename T>
inline QDataStream& operator<<(QDataStream& s, const mvector<T>& c) {
    using Container = mvector<T>;
    s << quint32(c.size());
    for (const typename Container::value_type& t : c)
        s << t;
    return s;
}

////////////////////////////////////////////////////////////////
/// std::map<Key, T>
///
template <class Key, class T, class C>
inline QDataStream& operator>>(QDataStream& s, std::map<Key, T, C>& map) {
    // StreamStateSaver stateSaver(&s);
    using Container = std::map<Key, T>;
    map.clear();
    quint32 n;
    s >> n;
    for (quint32 i = 0; i < n; ++i) {
        typename Container::key_type k;
        typename Container::mapped_type t;
        s >> k >> t;
        if (s.status() != QDataStream::Ok) {
            map.clear();
            break;
        }
        map.emplace(k, t);
    }
    return s;
}

template <class Key, class T, class C>
inline QDataStream& operator<<(QDataStream& s, const std::map<Key, T, C>& map) {
    s << quint32(map.size());
    // Deserialization should occur in the reverse order.
    // Otherwise, value() will return the least recently inserted
    // value instead of the most recently inserted one.
    auto it = map.cend();
    auto begin = map.cbegin();
    while (it != begin) {
        --it;
        s << it->first << it->second;
    }
    return s;
}
////////////////////////////////////////////////////////////////
/// std::vector<T>
///
template <typename T>
inline QDataStream& operator>>(QDataStream& s, std::vector<T>& c) {
    using Container = mvector<T>;
    c.clear();
    quint32 n;
    s >> n;
    c.reserve(n);
    for (quint32 i = 0; i < n; ++i) {
        typename Container::value_type t;
        s >> t;
        if (s.status() != QDataStream::Ok) {
            c.clear();
            break;
        }
        c.push_back(t);
    }
    return s;
}

template <typename T>
inline QDataStream& operator<<(QDataStream& s, const std::vector<T>& c) {
    using Container = mvector<T>;
    s << quint32(c.size());
    for (const typename Container::value_type& t : c)
        s << t;
    return s;
}
