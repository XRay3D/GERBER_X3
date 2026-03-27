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

// #include "cancelation.h"
#include "mvector.h"
#include "utils.h"
#include <QDebug>
#include <QIcon>
#include <QPolygonF>
#include <QTransform>
#include <clipper2/clipper.h>
#include <numbers>
#include <ranges>

using namespace Qt ::Literals;
namespace v = std ::views;
namespace r = std ::ranges;

enum {
    IconSize = 24
};

using std::numbers::pi;

constexpr auto sqrt1_2 = std::numbers::sqrt2 * 0.5;
constexpr auto two_pi = pi * 2;

constexpr int uScale = 1e+5;
constexpr double dScale = 1e-5;

constexpr std::numeric_limits<int32_t> LimitI32;

namespace CL2 = Clipper2Lib;

// type
using Clipper = Clipper2Lib::Clipper64;
// using ClipperOffset = Clipper2Lib::ClipperOffset;

using Point = Clipper2Lib::Point64;

using Path = Clipper2Lib::Path64;
using Paths = Clipper2Lib::Paths64;
using Pathss = std::vector<Paths>;

using PolyTree = Clipper2Lib::PolyTree64;
using Rect = Clipper2Lib::Rect64;

// func
using Clipper2Lib::Area;
using Clipper2Lib::GetBounds;
using Clipper2Lib::InflatePaths;
using Clipper2Lib::PointInPolygon;

// enum
using Clipper2Lib::ClipType;
using Clipper2Lib::EndType;
using Clipper2Lib::FillRule;
using Clipper2Lib::JoinType;
using Clipper2Lib::PathType;
using Clipper2Lib::PointInPolygonResult;

Q_DECLARE_METATYPE(Point)

constexpr bool operator<(const QPointF& r, const QPointF& l) noexcept {
    return std::tuple{r.x(), r.y()} < std::tuple{l.x(), l.y()};
}

constexpr bool operator<(const Point& r, const Point& l) noexcept {
    return std::tuple{r.x, r.y} < std::tuple{l.x, l.y};
}

// constexpr bool operator<(const Point& l, const Point& r) noexcept {
// return std::tie(l.x, l.y) < std::tie(r.x, r.y);
// };

// template <>
// struct std::less<Point> : public std::binary_function<Point, Point, bool> {
//     bool operator()(const Point& l, const Point& r) const {
//         return std::tie(l.x, l.y) < std::tie(r.x, r.y);
//     }
//     bool operator()(Point& l, Point& r) const {
//         return std::tie(l.x, l.y) < std::tie(r.x, r.y);
//     }
// };

void TestPaths(const Paths& paths);

Point GetC(const Point& dst);
void SetC(Point& dst, const Point& center);
void SetCForce(Point& dst, const Point& center);
void SetCSelf(Point& dst);

//------------------------------------------------------------------------------

template <typename T> concept Arithmetic = std::is_arithmetic_v<T>;

inline Point& operator*=(Point& pt, Arithmetic auto v) noexcept {
    return pt.x *= v, pt.y *= v, pt;
}

inline Path& operator*=(Path& path, Arithmetic auto v) noexcept {
    for(Point& pt: path) pt *= v; // FIME maybe and center
    return path;
}

constexpr Point toPoint(const QPointF& p) noexcept { return {
    p.x() * uScale,
    p.y() * uScale,
}; }
constexpr QPointF toQPointF(const Point& p) noexcept { return {
    p.x * dScale,
    p.y * dScale,
}; }

constexpr Point operator~(const QPointF& p) noexcept { return toPoint(p); }
constexpr QPointF operator~(const Point& p) noexcept { return toQPointF(p); }

constexpr Point operator!(const Point& p) noexcept { return GetC(p); }

#define TRANSFORM(FROM, TO)                                           \
    inline TO operator~(std::span<const FROM> val) {                  \
        auto it = v::transform(val, [](auto&& val) { return ~val; }); \
        TO ret;                                                       \
        ret.reserve(Cast{val.size()});                                \
        r::move(it, std::back_inserter(ret));                         \
        return ret;                                                   \
    }

TRANSFORM(QPointF, Path)
TRANSFORM(Point, QPolygonF)

TRANSFORM(QPolygonF, Paths)
TRANSFORM(Path, QList<QPolygonF>)

#undef TRANSFORM

template <>
template <>
constexpr Cast<QPointF>::operator Point() const { return ~val; }

template <>
template <>
constexpr Cast<Point>::operator QPointF() const { return ~val; }
//------------------------------------------------------------------------------

constexpr double Radius(Point p) {
    Point c = GetC(p);
    if(p == Point{}) return std::nan("");
    p = p - c;
    return hypot(p.x, p.y) * dScale;
}

double Perimeter(std::span<const Point> path, bool open = {});
//------------------------------------------------------------------------------
Path CirclePath(double diametr, const Point& center = Point{});
Path RectanglePath(double width, double height, const Point& center = Point{});
//------------------------------------------------------------------------------
void RotatePath(Path& path, double angle, const Point& center = Point{});

Path& TranslatePath(Path& path, const Point& pos);
Paths& TranslatePaths(Paths& path, const Point& pos);

Path& TransformPath(Path& path, const QTransform& m);
Paths& TransformPaths(Paths& paths, const QTransform& m);
//------------------------------------------------------------------------------

void mergeSegments(Paths& paths, double glue = {});

/////////////////////////////////////////////////
/// \brief склеивает пути при совпадении конечных точек
/// \param paths - пути
/// \param maxDist - максимальное расстояние между конечными точками
void mergePaths(Paths& paths, const double dist = {});

QIcon drawIcon(const QPainterPath& pPath, QColor color = Qt::black, bool stroke = false);
QIcon drawIcon(const Paths& paths, QColor color = Qt::black);

QIcon drawDrillIcon(QColor color = Qt::black);

Paths& normalize(Paths& paths);

inline constexpr auto skipFront = v::drop(1);

//------------------------------------------------------------------------------

Paths Inflate(const Paths& paths, double delta,
    JoinType jt, EndType et,
    double miterLimit = 2.0, double arcTolerance = {});

Paths InflateRoundPolygon(const Paths& paths, double delta,
    double miterLimit = 2.0, double arcTolerance = {});

Paths InflateMiterPolygon(const Paths& paths, double delta,
    double miterLimit = 2.0, double arcTolerance = {});

template <typename T>
inline void CleanPaths(Clipper2Lib::Path<T>& path, double k) {
    path = Clipper2Lib::RamerDouglasPeucker(path, k);
}

template <typename T>
inline void CleanPaths(Clipper2Lib::Paths<T>& paths, double k) {
    paths = Clipper2Lib::RamerDouglasPeucker(paths, k);
}

//------------------------------------------------------------------------------

template <typename T>
inline Clipper2Lib::Path<T>& ReversePath(Clipper2Lib::Path<T>& path) {
    std::reverse(path.begin(), path.end());
    return path;
}
//------------------------------------------------------------------------------

template <typename T>
inline Clipper2Lib::Paths<T>& ReversePaths(Clipper2Lib::Paths<T>& paths) {
    std::for_each(paths.begin(), paths.end(), ReversePath<T>);
    return paths;
}

QDataStream& operator<<(QDataStream& stream, const Point& pt);
QDataStream& operator>>(QDataStream& stream, Point& pt);
QDebug operator<<(QDebug d, const Point& p);

//----Container helpers----------------------------------------------------------------

template <typename T> concept Container = r::range<T> && requires(T c) {
    c.reserve(size_t{});
    c.append_range(c);

    // { typename T::value_type{} } -> std::same_as<std::decay_t<decltype(*c.begin())>>;
    // T::value_type;
    // { std::is_same<T, QByteArray> } -> std::same_as<std::false_type>;
    // { std::is_same<T, QString> } -> std::same_as<std::false_type>;
};

template <Container C>
inline auto join(C&& cont) {
    auto j = v::join(cont);
    return std::decay_t<decltype(*cont.begin())>{j.begin(), j.end()};
};

template <typename Cont>
inline int indexOf(const Cont& c, const typename Cont::value_type& v) {
    auto it = std::find(c.begin(), c.end(), v);
    return it == c.end() ? -1 : std::distance(c.begin(), it);
}

auto operator+=(Container auto& c, r::range auto&& v) {
    c.reserve(c.size() + r::size(v));
    c.append_range(std::forward<decltype(v)>(v));
    return c;
}

// auto operator+=(Container auto& c, const Container auto& v) {
//     c.reserve(c.size() + v.size());
//     return r::copy(v, std::back_inserter(c)), c;
// }

auto operator-=(Container auto& c, size_t index) {
    return c.erase(c.begin() + index), c;
}

// template <typename T>
// inline Clipper2Lib::Rect<T> GetBounds(const Clipper2Lib::Paths<T>& paths) {
//     Clipper2Lib::Rect<T> rect;

//    if (paths.size() == 0 || paths.front().size() == 0)
//        return rect;

//    auto pt {paths.front().front()};
//    rect.bottom = pt.y;
//    rect.top = pt.y;
//    rect.left = pt.x;
//    rect.right = pt.x;

//    for (auto&& path : paths) {
//        for (auto&& pt : path) {
//            rect.bottom = std::max(rect.bottom, pt.y);
//            rect.top = std::min(rect.top, pt.y);
//            rect.left = std::min(rect.left, pt.x);
//            rect.right = std::max(rect.right, pt.x);
//        }
//    }
//    return rect;
//}

//------------------------------------------------------------------------------

inline void SimplifyPolygon(const Path& /*in_poly*/, Paths& /*out_polys*/, Clipper2Lib::FillRule /*fillType*/ = Clipper2Lib::FillRule::EvenOdd) {
    //    Clipper c;
    //    c.StrictlySimple(true);
    //    c.AddPath(in_poly, PathType::Subject, true);
    //    c.Execute(ClipType::Union, out_polys, fillType, fillType);
}

inline void SimplifyPolygons(const Paths& /*in_polys*/, Paths& /*out_polys*/, Clipper2Lib::FillRule /*fillType*/ = Clipper2Lib::FillRule::EvenOdd) {
    //    Clipper c;
    //    c.StrictlySimple(true);
    //    c.AddPaths(in_polys, PathType::Subject, true);
    //    c.Execute(ClipType::Union, out_polys, fillType, fillType);
}

inline void SimplifyPolygons(Paths& polys, Clipper2Lib::FillRule fillType = Clipper2Lib::FillRule::EvenOdd) {
    SimplifyPolygons(polys, polys, fillType);
}

struct LineABC {
    // ax + by + c = 0
    double a;
    double b;
    double c;
    LineABC(const QLineF& l)
        : a{l.p1().y() - l.p2().y()}
        , b{l.p2().x() - l.p1().x()}
        , c{l.p1().x() * l.p2().y() - l.p2().x() * l.p1().y()} { }
    operator bool() const {
        return !qFuzzyIsNull(a) || !qFuzzyIsNull(b);
    }
    double distance(const QPointF& p) const {
        return abs(a * p.x() + b * p.y() + c) / sqrt(a * a + b * b);
    }
    double lenght() const { return sqrt(a * a + b * b); }
};

void reductionOfDistance(Path& path, Point point = Point{});

inline bool pointOnPolygon(const QLineF& l2, const Path& path, Point* ret) {
    const size_t cnt = path.size();
    if(cnt < 2)
        return false;
    QPointF p;
    for(size_t i{}; i < cnt; ++i) {
        const Point& pt1 = path[(i + 1) % cnt];
        const Point& pt2 = path[i];
        QLineF l1(~pt1, ~pt2);
        if(QLineF::BoundedIntersection == l1.intersects(l2, &p)) {
            if(ret)
                *ret = ~p;
            return true;
        }
    }
    return false;
}

inline /*constexpr*/ double angleTo(const Point& pt1, const Point& pt2) noexcept {
    const double dx = static_cast<double>(pt2.x - pt1.x);
    const double dy = static_cast<double>(pt2.y - pt1.y);
    const double theta = atan2(-dy, dx) * 360.0 / (pi * 2);
    const double theta_normalized = theta < 0 ? theta + 360 : theta;
    if(qFuzzyCompare(theta_normalized, double(360)))
        return 0.0;
    else
        return theta_normalized;
}

inline /*constexpr*/ double angleRadTo(const Point& pt1, const Point& pt2) noexcept {
    const double dx = static_cast<double>(pt2.x - pt1.x);
    const double dy = static_cast<double>(pt2.y - pt1.y);
    const double theta = atan2(-dy, dx);
    return theta;
    const double theta_normalized = theta < 0 ? theta + (pi * 2) : theta; // NOTE theta_normalized
    if(qFuzzyCompare(theta_normalized, (pi * 2)))
        return 0.0;
    else
        return theta_normalized;
}

inline /*constexpr*/ double distTo(const Point& pt1, const Point& pt2) noexcept {
    double x_ = static_cast<double>(pt2.x - pt1.x);
    double y_ = static_cast<double>(pt2.y - pt1.y);
    return sqrt(x_ * x_ + y_ * y_);
}

inline constexpr double distToSq(const Point& pt1, const Point& pt2) noexcept {
    double x_ = static_cast<double>(pt2.x - pt1.x);
    double y_ = static_cast<double>(pt2.y - pt1.y);
    return (x_ * x_ + y_ * y_);
}

std::span<std::unique_ptr<CL2::PolyPath64>> rwPolyTree(PolyTree& polyTree);

Path arc(const Point& center, double radius, double start, double stop, int interpolation);
Path arc(Point p1, Point p2, Point center, int interpolation);

void markPolyTreeDByNesting(PolyTree& polynode);
void sortPolyTreeByNesting(PolyTree& polynode);
Pathss stacking(Paths& paths);

Path boundOfPaths(const Paths& paths, /*PType*/ int32_t k);

Paths& sortB(Paths& src, Point startPt);
Paths& sortBeginEnd(Paths& src, Point startPt);
Pathss& sortB(Pathss& src, Point startPt);
Pathss& sortBeginEnd(Pathss& src, Point startPt);

void addArcTo(QPainterPath& pPath, QPointF source, QPointF target, double bulge);

#include "curve.h"
