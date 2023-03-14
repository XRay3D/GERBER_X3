/*******************************************************************************
 * Author    :  Angus Johnson                                                   *
 * Date      :  21 November 2022                                                *
 * Website   :  http://www.angusj.com                                           *
 * Copyright :  Angus Johnson 2010-2022                                         *
 * Purpose   :  Core Clipper Library structures and functions                   *
 * License   :  http://www.boost.org/LICENSE_1_0.txt                            *
 *******************************************************************************/

#pragma once

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <QDataStream>
#include <QDebug>
#include <QPolygonF>
#include <mvector.h>

static constexpr auto uScale {100000};
static constexpr auto dScale {1. / uScale};
extern void ifCancelThenThrow();

namespace Clipper2Lib {

using int_t = int32_t;

template <typename T>
struct Point {
    using Type = T;
    T x;
    T y;
#ifdef USINGZ
    IntType z;

    template <typename T2>
    inline void Init(const T2 x_ = 0, const T2 y_ = 0, const IntType z_ = 0) {
        if constexpr (std::numeric_limits<T>::is_integer && !std::numeric_limits<T2>::is_integer) {
            x = static_cast<T>(std::round(x_));
            y = static_cast<T>(std::round(y_));
            z = z_;
        } else {
            x = static_cast<T>(x_);
            y = static_cast<T>(y_);
            z = z_;
        }
    }

    explicit Point()
        : x(0)
        , y(0)
        , z(0) {};

    template <typename T2>
    Point(const T2 x_, const T2 y_, const IntType z_ = 0) {
        Init(x_, y_);
        z = z_;
    }

    template <typename T2>
    explicit Point<T>(const Point<T2>& p) {
        Init(p.x, p.y, p.z);
    }

    Point operator*(const double scale) const {
        return Point(x * scale, y * scale, z);
    }

    friend std::ostream& operator<<(std::ostream& os, const Point& point) {
        os << point.x << " " << point.y << " " << point.z;
        return os;
    }
#else
    template <typename T2>
    inline void Init(const T2 x_ = 0, const T2 y_ = 0) {
        if constexpr (std::numeric_limits<T>::is_integer && !std::numeric_limits<T2>::is_integer) {
            x = static_cast<T>(std::round(x_));
            y = static_cast<T>(std::round(y_));
        } else {
            x = static_cast<T>(x_);
            y = static_cast<T>(y_);
        }
    }

    /*explicit*/ Point()
        : x(0)
        , y(0) {};

    template <typename T2>
    Point(const T2 x_, const T2 y_) { Init(x_, y_); }

    template <typename T2>
    explicit Point<T>(const Point<T2>& p) { Init(p.x, p.y); }

    Point operator*(const double scale) const {
        return Point(x * scale, y * scale);
    }

    friend std::ostream& operator<<(std::ostream& os, const Point& point) {
        os << point.x << " " << point.y;
        return os;
    }
#endif

    friend bool operator==(const Point& a, const Point& b) {
        return a.x == b.x && a.y == b.y;
    }

    friend bool operator!=(const Point& a, const Point& b) {
        return !(a == b);
    }

    inline Point<T> operator-() const {
        return Point<T>(-x, -y);
    }

    inline Point operator+(const Point& b) const {
        return Point(x + b.x, y + b.y);
    }

    inline Point operator-(const Point& b) const {
        return Point(x - b.x, y - b.y);
    }

    inline void Negate() {
        x = -x;
        y = -y;
    }
    //    T X;
    //    T Y;
    //    #ifdef use_xyz
    //    T Z;
    //    Point(T x = 0, T y = 0, T z = 0) F
    //        : X(x),
    //          Y(y),
    //          Z(z) {};
    //    #else
    //    #endif
    //    constexpr Point(T x = {}, T y = {}) noexcept
    //        : X(x)
    //        , Y(y) {
    //    }

    //    constexpr Point(Point&& p) noexcept = default;
    //    constexpr Point(const Point& p) noexcept = default;
    //    constexpr Point& operator=(Point&& p) noexcept = default;
    //    constexpr Point& operator=(const Point& p) noexcept = default;

    constexpr Point(QPointF&& p) noexcept
        : x(p.x() * uScale)
        , y(p.y() * uScale) {
    }
    constexpr Point(const QPointF& p) noexcept
        : x(p.x() * uScale)
        , y(p.y() * uScale) {
    }
    constexpr Point& operator=(QPointF&& p) noexcept {
        x = p.x() * uScale;
        y = p.y() * uScale;
        return *this;
    }
    constexpr Point& operator=(const QPointF& p) noexcept {
        x = p.x() * uScale;
        y = p.y() * uScale;
        return *this;
    }

    constexpr bool isNull() const noexcept {
        return x == 0 && y == 0;
    }

    constexpr Point& operator*=(double s) noexcept {
        return x *= s, y *= s, *this;
    }

    constexpr Point& operator+=(const Point& pt) noexcept {
        return x += pt.x, y += pt.y, *this;
    }

    constexpr Point& operator-=(const Point& pt) noexcept {
        return x -= pt.x, y -= pt.y, *this;
    }

    constexpr operator QPointF() const noexcept {
        return {x * dScale, y * dScale};
    }

#ifdef __GNUC___
    bool operator==(const Point& L) const noexcept {
        return X == L.x && Y == L.y;
    }
    bool operator!=(const Point& L) const noexcept {
        return !(*this == L);
    }
    bool operator<(const Point& L) const noexcept {
        return std::tuple {X, Y} < std::tuple {L.x, L.y};
    }
#else
    constexpr auto operator<=>(const Point&) const noexcept = default;
#endif

    friend QDataStream& operator<<(QDataStream& stream, const Point& pt) {
        stream.writeRawData(reinterpret_cast<const char*>(&pt), sizeof(Point));
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Point& pt) {
        stream.readRawData(reinterpret_cast<char*>(&pt), sizeof(Point));
        return stream;
    }

    friend QDebug operator<<(QDebug d, const Point& p) {
        d << "IntPt(" << p.x << ", " << p.y << ")";
        return d;
    }

    /*constexpr*/ double angleTo(const Point& pt2) const noexcept {
        const double dx = pt2.x - x;
        const double dy = pt2.y - y;
        const double theta = atan2(-dy, dx) * 360.0 / (M_PI * 2);
        const double theta_normalized = theta < 0 ? theta + 360 : theta;
        if (qFuzzyCompare(theta_normalized, double(360)))
            return 0.0;
        else
            return theta_normalized;
    }

    /*constexpr*/ double angleRadTo(const Point& pt2) const noexcept {
        const double dx = pt2.x - x;
        const double dy = pt2.y - y;
        const double theta = atan2(-dy, dx);
        return theta;
        const double theta_normalized = theta < 0 ? theta + (M_PI * 2) : theta; // NOTE theta_normalized
        if (qFuzzyCompare(theta_normalized, (M_PI * 2)))
            return 0.0;
        else
            return theta_normalized;
    }

    /*constexpr*/ double distTo(const Point& pt2) const noexcept {
        double x_ = pt2.x - x;
        double y_ = pt2.y - y;
        return sqrt(x_ * x_ + y_ * y_);
    }

    constexpr double distToSq(const Point& pt2) const noexcept {
        double x_ = pt2.x - x;
        double y_ = pt2.y - y;
        return (x_ * x_ + y_ * y_);
    }

    QString toString() const { return QStringLiteral("{%1, %2}").arg(x * dScale).arg(y * dScale); }
};

//------------------------------------------------------------------------------
template <typename T>
struct Path : mvector<Point<T>> {
    using MV = mvector<Point<T>>;
    using MV::MV;

    Path(const QPolygonF& v) {
        MV::reserve(v.size());
        for (auto& pt : v)
            MV::emplace_back(pt);
    }

    //    template <typename U>
    //        requires(!std::is_same_v<T, U>)
    //    Path(const Path<U>& v) {
    //        MV::reserve(v.size());
    //        for (auto& pt : v)
    //            MV::emplace_back(pt);
    //    }

    //    template <typename U>
    //        requires(!std::is_same_v<T, U>)
    //    Path& operator=(const Path<U>& v) {
    //        MV::clear();
    //        MV::reserve(v.size());
    //        for (auto& pt : v)
    //            MV::emplace_back(pt);
    //        return *this;
    //    }

    operator QPolygonF() const {
        QPolygonF poly;
        poly.reserve(int(MV::size() + 1)); // +1 if need closed polygons
        for (const auto pt : *this)
            poly << pt;
        return poly;
    }
    //    int id {};

    auto friend operator+(const Path& path, QPointF pt) noexcept {
        Path ret(path);
        for (auto&& point : ret)
            point += pt;
        return ret;
    }

    auto& operator+=(QPointF pt) noexcept {
        for (auto&& point : *this)
            point += pt;
        return *this;
    }
};

template <typename T>
struct Paths : mvector<Path<T>> {
    using MV = mvector<Path<T>>;
    using MV::MV;

    Paths(const MV& v)
        : MV(v) { }

    Paths(MV&& v)
        : MV(std::move(v)) { }

    Paths(const QList<QPolygonF>& v) {
        MV::reserve(v.size());
        for (auto&& qpolygonf : v)
            MV::emplace_back(qpolygonf);
    }

    //    template <typename U>
    //        requires(!std::is_same_v<T, U>)
    //    Paths(const Paths<U>& v) {
    //        MV::reserve(v.size());
    //        for (auto& path : v)
    //            MV::emplace_back(path);
    //    }

    //    template <typename U>
    //        requires(!std::is_same_v<T, U>)
    //    auto& operator=(const Paths<U>& v) {
    //        MV::clear();
    //        MV::reserve(v.size());
    //        for (auto& path : v)
    //            MV::emplace_back(path);
    //        return *this;
    //    }

    operator mvector<QPolygonF>() const {
        mvector<QPolygonF> polys;
        polys.reserve(MV::size());
        for (const auto& poly : *this)
            polys.emplace_back(poly);
        return polys;
    }

    auto friend operator+(const Paths& paths, QPointF pt) noexcept {
        Paths ret(paths);
        for (auto&& point : ret)
            point += pt;
        return ret;
    }

    auto& operator+=(QPointF pt) noexcept {
        for (auto&& path : *this)
            path += pt;
        return *this;
    }

    //    int id {};
};

using Point64 = Point<int_t>;
using Path64 = Path<int_t>;
using Paths64 = Paths<int_t>;

using PointD = Point<double>;
using PathD = Path<double>;
using PathsD = Paths<double>;

} // namespace Clipper2Lib
