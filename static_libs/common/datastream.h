/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include <boost/pfr.hpp>

#include <QDataStream>
#include <map>
#include <ranges>
#include <type_traits>

namespace pfr = boost::pfr;

template <class T, size_t N>
inline QDataStream& operator>>(QDataStream& s, T (&p)[N]) {
    uint32_t n;
    s >> n;
    for(auto&& val: p | std::views::take(std::min<uint32_t>(n, N)))
        s >> val;
    return s;
}

template <class T, size_t N>
inline QDataStream& operator<<(QDataStream& s, const T (&p)[N]) {
    s << uint32_t(N);
    for(auto& var: p) s << var;
    return s;
}

inline QDataStream& operator>>(QDataStream& s, size_t& val) {
    s.readRawData(reinterpret_cast<char*>(&val), sizeof(val));
    return s;
}

inline QDataStream& operator<<(QDataStream& s, size_t val) {
    s.writeRawData(reinterpret_cast<char*>(&val), sizeof(val));
    return s;
}

////////////////////////////////////////////////////////////////
/// std::vector<T>
///
template <typename T, class Alloc>
inline QDataStream& operator>>(QDataStream& stream, std::vector<T, Alloc>& container) {
    uint32_t n;
    stream >> n;
    container.resize(n);
    for(auto& var: container) {
        stream >> var;
        if(stream.status() != QDataStream::Ok)
            return container.clear(), stream;
    }
    return stream;
}

template <typename T, class Alloc>
inline QDataStream& operator<<(QDataStream& stream, const std::vector<T, Alloc>& container) {
    stream << uint32_t(container.size());
    for(const auto& var: container) stream << var;
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
    while(n--) {
        Key key;
        Val val;
        stream >> key >> val;
        if(stream.status() != QDataStream::Ok)
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

    for(auto& [key, val]: std::views::reverse(map))
        stream << key << val;

    return stream;
}

// порционное сохранение для гибкости.
class Block {
    QDataStream& stream;
    //    QByteArray data;
    uint32_t count{};

public:
    explicit Block(QDataStream& stream)
        : stream{stream} { }

    template <typename T>
        requires(pfr::detail::fields_count<T>() > 1)
    QDataStream& read(T& val) {
        stream >> count;
        count = std::min<uint32_t>(count, pfr::detail::fields_count<T>());
        pfr::for_each_field(val, [this](auto& field) { if(count) --count, stream >> field; });
        return stream;
    }

    template <typename T>
        requires(pfr::detail::fields_count<T>() > 1)
    QDataStream& write(const T& val) {
        stream << (count = pfr::detail::fields_count<T>());
        pfr::for_each_field(val, [this](const auto& field) { stream << field; });
        return stream;
    }

    template <typename... Args>
    QDataStream& read(Args&... args) {
        stream >> count;
        assert(count <= sizeof...(Args));
        count = std::min<uint32_t>(count, sizeof...(Args));
        return ((count ? --count, stream >> args : stream), ...);
    }

    template <typename... Args>
    QDataStream& write(const Args&... args) {
        stream << (count = sizeof...(Args));
        ((stream << args), ...);
        return stream;
    }

    template <typename... Args>
    QDataStream& rw(Args&... args) {
        if constexpr((std::is_const_v<Args> && ... && true))
            return write(args...);
        else
            return read(args...);
    }
};

#define SERIALIZE_POD(TYPE)                                              \
    friend QDataStream& operator<<(QDataStream& stream, const TYPE& p) { \
        return ::Block{stream}.write(p);                                 \
    }                                                                    \
    friend QDataStream& operator>>(QDataStream& stream, TYPE& p) {       \
        return ::Block{stream}.read(p);                                  \
    }
