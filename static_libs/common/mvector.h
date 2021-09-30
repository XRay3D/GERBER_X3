#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

template <class T>
struct mvector : std::vector<T> {
    using V = std::vector<T>;
    using M = mvector<T>;
    // clang-format off
    mvector() {}
    mvector(size_t size) : V(size) {}
    mvector(size_t size, const T& t) : V(size, t) {}
    mvector(const V& v) : V(v) {}
    mvector(V&& v) : V(std::forward<V>(v)) {}
    mvector(const M& v) : V(v) {}
    mvector(M&& v) : V(std::forward<M>(v)) {}
    mvector(const std::initializer_list<T>& v) : V(v) {}
    // clang-format on

    mvector& operator=(const M& v)
    {
        V::operator=(v);
        return *this;
    }
    mvector& operator=(M&& v)
    {
        V::operator=(std::move(v));
        return *this;
    }

    inline void append(const V& vec) { V::insert(V::end(), vec.begin(), vec.end()); }

    inline void remove(size_t idx) { V::erase(V::begin() + idx); }

    bool removeOne(const T& t)
    {
        auto it = std::find(V::begin(), V::end(), t);
        if (it == V::end())
            return false;
        V::erase(it);
        return true;
    }

    mvector mid(size_t idx, size_t len = 0) const
    {
        mvector v;
        if (idx >= V::size())
            return v;
        typename V::const_iterator end;
        typename V::const_iterator begin = V::cbegin() + idx;
        if (len == 0)
            end = V::cend();
        else if (idx + len > V::size())
            end = begin + (V::size() - idx);
        else
            end = begin + len;
        v.reserve(std::distance(begin, end));
        v.insert(v.end(), begin, end);
        return v;
    }

    //    mvector mid(size_t idx, size_t len = 0)
    //    {
    //        mvector v;
    //        if (idx >= V::size())
    //            return v;
    //        typename V::iterator end;
    //        const typename V::iterator begin = V::begin() + idx;
    //        if (len == 0)
    //            end = V::end();
    //        else if (idx + len > V::size())
    //            end = begin + (V::size() - idx);
    //        else
    //            end = begin + len;
    //        v.reserve(std::distance(begin, end));
    //        v.insert(v.end(), begin, end);
    //        return v;
    //    }

    inline void prepend(T&& t) { V::insert(V::begin(), 1, std::move(t)); }
    inline void prepend(const T& t) { V::insert(V::begin(), 1, t); }

    T takeLast()
    {
        T rt(std::move(V::back()));
        V::erase(V::end() - 1);
        return rt;
    }

    T takeFirst()
    {
        T rt(std::move(V::front()));
        V::erase(V::begin());
        return rt;
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

    inline auto indexOf(const T& t) const noexcept
    {
        if (auto it = std::find(V::begin(), V::end(), t); it == V::end())
            return std::distance(V::begin() + 1, V::begin());
        else
            return std::distance(V::begin(), it);
    }

    template <class P>
    inline auto indexOf(const P* t) const noexcept requires std::is_base_of_v<T, std::unique_ptr<P>>
    {
        using CP = const P;
        auto it = std::find(V::begin(), V::end(), std::unique_ptr<CP, std::function<void(CP*)>>(t, [](CP*) {}));
        if (it == V::end())
            return std::distance(V::begin() + 1, V::begin());
        else
            return std::distance(V::begin(), it);
    }

    template <class P>
    inline auto indexOf(P* t) const noexcept requires std::is_base_of_v<T, std::shared_ptr<P>>
    {
        auto it = std::find(V::begin(), V::end(), t /*std::shared_ptr<P, std::function<void(P*)>>(t, [](P*) {})*/);
        if (it == V::end())
            return std::distance(V::begin() + 1, V::begin());
        else
            return std::distance(V::begin(), it);
    }

    inline T takeAt(const T& t) noexcept
    {
        auto it = std::find(V::begin(), V::end(), t);
        if (it == V::end())
            return {};
        T r(std::move(*it));
        V::erase(it);
        return r;
    }

    inline T takeAt(size_t idx) noexcept
    {
        if (V::begin() + idx >= V::end())
            return {};
        T r(std::move(*(V::begin() + idx)));
        V::erase(V::begin() + idx);
        return r;
    }

    //    template <class P, std::enable_if_t<std::is_base_of_v<T, std::shared_ptr<P>>, int> = 0>
    //    inline auto indexOf(P* t) const noexcept
    //    {
    //        if (auto it = std::find(V::begin(), V::end(), std::shared_ptr<P, std::function<void(P*)>>(t, [](P*) {})); it == V::end())
    //            return std::distance(V::begin() + 1, V::begin());
    //        else
    //            return std::distance(V::begin(), it);
    //    }

    inline bool
    contains(const T& t) const noexcept
    {
        return std::find(V::begin(), V::end(), t) != V::end();
    }
};
