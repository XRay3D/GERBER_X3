/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QDataStream>
#include <type_traits>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
inline QDataStream& operator>>(QDataStream& s, E& e)
{
    qint32 i;
    s >> i;
    e = static_cast<E>(i);
    return s;
}

template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
inline QDataStream& operator<<(QDataStream& s, E e)
{
    s << static_cast<qint32>(e);
    return s;
}

template <typename T>
inline QDataStream& operator>>(QDataStream& s, std::vector<T>& c)
{
    using Container = std::vector<T>;
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
inline QDataStream& operator<<(QDataStream& s, const std::vector<T>& c)
{
    using Container = std::vector<T>;
    s << quint32(c.size());
    for (const typename Container::value_type& t : c)
        s << t;
    return s;
}

template <class Key, class T>
inline QDataStream& operator>>(QDataStream& s, std::map<Key, T>& map)
{
    //StreamStateSaver stateSaver(&s);
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

template <class Key, class T>
inline QDataStream& operator<<(QDataStream& s, const std::map<Key, T>& map)
{
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

#endif
