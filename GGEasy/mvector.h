#pragma once

#include <QDebug>
#include <algorithm>
#include <vector>

template <class T>
struct mvector : std::vector<T> {
    using V = std::vector<T>;
    mvector(int size = 0)
        : std::vector<T>(size)
    {
    }
    mvector(int size, const T& t)
        : std::vector<T>(size, t)
    {
    }
    mvector(const mvector<T>& v)
        : std::vector<T>(v)
    {
    }

    mvector(const std::initializer_list<T>& v)
        : std::vector<T>(v)
    {
    }

    inline bool isEmpty() const noexcept { return V::empty(); }

    inline size_t count() const noexcept { return V::size(); }

    inline T& first() noexcept { return V::front(); }
    inline T& last() noexcept { return V::back(); }

    inline const T& first() const noexcept { return V::front(); }
    inline const T& last() const noexcept { return V::back(); }

    inline void push_back(const V& Val) { V::insert(V::end(), Val.begin(), Val.end()); }
    inline void push_back(const T& Val) { V::emplace_back(Val); }
    inline void push_back(T&& Val) { V::emplace_back(std::move(Val)); }

    inline void append(const T& Val) { V::emplace_back(Val); }
    inline void append(T&& Val) { V::emplace_back(std::move(Val)); }
    inline void append(const V& Val) { V::insert(V::end(), Val.begin(), Val.end()); }

    inline void remove(size_t idx) { V::erase(V::begin() + idx); }

    V mid(size_t idx, size_t len = 0)
    {
        V v;
        if (idx >= V::size())
            return v;
        typename V::iterator end;
        typename V::iterator begin = V::begin() + idx;
        if (len == 0)
            end = V::end();
        else if (idx + len > V::size())
            end = begin + (V::size() - idx);
        else
            end = begin + len;
        v.reserve(std::distance(begin, end));
        v.insert(v.end(), begin, end);
        return v;
    }

    inline void prepend(const T& Val) { V::insert(V::begin(), 1, Val); }
    inline void prepend(T&& Val) { V::insert(V::begin(), 1, std::move(Val)); }

    T takeLast()
    {
        T v(std::move(V::back()));
        V::erase(V::end() - 1);
        return v;
    }

    T takeFirst()
    {
        T v(std::move(V::front()));
        V::erase(V::begin());
        return v;
    }

    friend inline mvector& operator<<(mvector& v, const T& t)
    {
        v.push_back(t);
        return v;
    }

    friend inline mvector& operator<<(mvector& v, T&& t)
    {
        v.push_back(std::move(t));
        return v;
    }

    inline auto indexOf(const T& p) const noexcept
    {
        if (auto it = std::find(V::begin(), V::end(), p); it == V::end())
            return std::distance(V::begin() + 1, V::begin());
        else
            return std::distance(V::begin(), it);
    }

    inline bool contains(const T& t) const noexcept
    {
        return std::find(V::begin(), V::end(), t) != V::end();
    }
};
