#pragma once

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

    bool isEmpty() { return V::empty(); }

    size_t count() { return V::size(); }

    T& first() { return V::front(); }
    T& last() { return V::back(); }

    const T& first() const { return V::front(); }
    const T& last() const { return V::back(); }

    void push_back(const V& Val)
    {
        V::insert(V::end(), Val.begin(), Val.end());
    }

    void push_back(const T& Val) // insert element at end, provide strong guarantee
    {
        V::emplace_back(Val);
    }

    void push_back(T&& Val) // insert by moving into element at end, provide strong guarantee
    {
        V::emplace_back(_STD move(Val));
    }

    void remove(size_t idx)
    {
        V::erase(V::begin() + idx);
    }

    V mid(size_t idx, int len = -1)
    {
        V v;
        v.insert(V::end(), V::begin() + idx, (len > 0 //
                                                     ? std::clamp(V::begin() + idx + len, V::begin() + idx, V::end()) //
                                                     : V::end()));
        return v;
    }

    void prepend(const T& Val) // insert element at end, provide strong guarantee
    {
        V::insert(V::begin(), 1, Val);
    }

    void prepend(T&& Val) // insert by moving into element at end, provide strong guarantee
    {
        V::insert(V::begin(), 1, _STD move(Val));
    }

    T takeLast()
    {
        T v(std::move(V::back()));
        V::erase(V::end() - 1);
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

    inline auto indexOf(const T& p) const
    {
        if (auto it = std::find(V::begin(), V::end(), p); it == V::end())
            return std::distance(V::begin() + 1, V::begin());
        else
            return std::distance(V::begin(), it);
    }

    inline bool contains(const T& t) const
    {
        return std::find(V::begin(), V::end(), t) != V::end();
    }
};
