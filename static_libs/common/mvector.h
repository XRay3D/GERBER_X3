#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

template <class T>
struct mvector : std::vector<T> {
    using V = std::vector<T>;
    mvector(size_t size = 0)
        : std::vector<T>(size)
    {
    }
    mvector(size_t size, const T& t)
        : std::vector<T>(size, t)
    {
    }
    mvector(const mvector<T>& v)
        : std::vector<T>(v)
    {
    }
    mvector(mvector<T>&& v)
        : std::vector<T>(v)
    {
    }

    mvector(const std::initializer_list<T>& v)
        : std::vector<T>(v)
    {
    }

    mvector& operator=(const mvector<T>& v)
    {
        V::operator=(v);
        return *this;
    }
    mvector& operator=(mvector<T>&& v)
    {
        V::operator=(std::move(v));
        return *this;
    }

    inline bool isEmpty() const noexcept { return V::empty(); }

    inline size_t count() const noexcept { return V::size(); }

    inline T& first() noexcept { return V::front(); }
    inline T& last() noexcept { return V::back(); }

    inline const T& first() const noexcept { return V::front(); }
    inline const T& last() const noexcept { return V::back(); }

    inline void push_back(T&& t) { V::emplace_back(std::move(t)); }
    inline void push_back(const T& t) { V::emplace_back(t); }
    inline void push_back(const V& vec) { V::insert(V::end(), vec.begin(), vec.end()); }

    inline void append(T&& t) { V::emplace_back(std::move(t)); }
    inline void append(const T& t) { V::emplace_back(t); }
    inline void append(const V& vec) { V::insert(V::end(), vec.begin(), vec.end()); }

    //    template <class... _Valty>
    //    decltype(auto) emplace_back(_Valty&&... _Val)
    //    {
    //        return V::template emplace_back<_Valty...>(_Val...);
    //    }
    //    template <class... _Valty>
    //    decltype(auto) emplace_back(_Valty&... _Val)
    //    {
    //        return V::template emplace_back<_Valty...>(_Val...);
    //    }

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

    template <class P, std::enable_if_t<std::is_base_of_v<T, std::unique_ptr<P>>, int> = 0>
    inline auto indexOf(P* t) const noexcept
    {
        if (auto it = std::find(V::begin(), V::end(), std::unique_ptr<P, std::function<void(P*)>>(t, [](P*) {})); it == V::end())
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
