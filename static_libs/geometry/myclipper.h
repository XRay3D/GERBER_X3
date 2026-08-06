/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
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

namespace cl = Clipper2Lib;

// type
using Point64 = cl::Point64;
using Path64 = cl::Path64;
using Paths64 = cl::Paths64;
using Pathss64 = std::vector<Paths64>;
using PolyTree = cl::PolyTree64;
using Rect = cl::Rect64;

// func
using cl::Area;
using cl::GetBounds;
// using cl::InflatePaths;
using cl::PointInPolygon;

// enum
// using cl::ClipType;
// using cl::EndType;
// using cl::FillRule;
// using cl::JoinType;
// using cl::PathType;
using cl::PointInPolygonResult;

Q_DECLARE_METATYPE(Point64)

constexpr bool operator<(const QPointF& r, const QPointF& l) noexcept {
    return std::tuple{r.x(), r.y()} < std::tuple{l.x(), l.y()};
}

constexpr bool operator<(const Point64& r, const Point64& l) noexcept {
    return std::tuple{r.x, r.y} < std::tuple{l.x, l.y};
}

// constexpr bool operator<(const Point64& l, const Point64& r) noexcept {
// return std::tie(l.x, l.y) < std::tie(r.x, r.y);
// };

// template <>
// struct std::less<Point64> : public std::binary_function<Point64, Point64, bool> {
//     bool operator()(const Point64& l, const Point64& r) const {
//         return std::tie(l.x, l.y) < std::tie(r.x, r.y);
//     }
//     bool operator()(Point64& l, Point64& r) const {
//         return std::tie(l.x, l.y) < std::tie(r.x, r.y);
//     }
// };

void TestPaths(const Paths64& paths);

Point64 GetC(const Point64& dst);
void SetC(Point64& dst, const Point64& center);
void SetCForce(Point64& dst, const Point64& center);
void SetCSelf(Point64& dst);

//----Индексное хранилище центров дуг (для конвертации Curve <-> Path64)--------------
// Point64::z хранит пару 32-битных индексов (prev, next) в глобальном реестре центров
// вместо самих координат: prev - индекс центра дуги, заканчивающейся в этой точке,
// next - индекс центра дуги, начинающейся в этой точке. 0 означает "не дуга".
// Индексы никогда не переиспользуются и не инвалидируются (append-only), начинаются с 1.
enum class CenterKind {
    Source,    // центр дуги исходной кривой (Curve -> Path64, CirclePath, arc())
    RoundJoin, // новый центр скругления угла, созданный при офсетинге (ClipperOffset, cl::JoinType::Round)
};
int32_t RegisterCenter(const QPointF& center, CenterKind kind = CenterKind::Source);
QPointF CenterAt(int32_t index);
CenterKind CenterKindAt(int32_t index);
int32_t GetCPrevIndex(const Point64& dst);
int32_t GetCNextIndex(const Point64& dst);
void SetCIndices(Point64& dst, int32_t prevIndex, int32_t nextIndex);

//------------------------------------------------------------------------------

template <typename T> concept Arithmetic = std::is_arithmetic_v<T>;

inline Point64& operator*=(Point64& pt, Arithmetic auto v) noexcept {
    return pt.x *= v, pt.y *= v, pt;
}

inline Path64& operator*=(Path64& path, Arithmetic auto v) noexcept {
    for(Point64& pt: path) pt *= v; // FIME maybe and center
    return path;
}

constexpr Point64 toPoint(const QPointF& p) noexcept { return {
    p.x() * uScale,
    p.y() * uScale,
}; }
constexpr QPointF toQPointF(const Point64& p) noexcept { return {
    p.x * dScale,
    p.y * dScale,
}; }

constexpr Point64 operator~(const QPointF& p) noexcept { return toPoint(p); }
constexpr QPointF operator~(const Point64& p) noexcept { return toQPointF(p); }

constexpr Point64 operator!(const Point64& p) noexcept { return GetC(p); }

#define TRANSFORM(FROM, TO)                                           \
    inline TO operator~(std::span<const FROM> val) {                  \
        auto it = v::transform(val, [](auto&& val) { return ~val; }); \
        TO ret;                                                       \
        ret.reserve(Cast{val.size()});                                \
        r::move(it, std::back_inserter(ret));                         \
        return ret;                                                   \
    }

TRANSFORM(QPointF, Path64)
TRANSFORM(Point64, QPolygonF)

TRANSFORM(QPolygonF, Paths64)
TRANSFORM(Path64, QList<QPolygonF>)

#undef TRANSFORM

template <>
template <>
constexpr Cast<QPointF>::operator Point64() const { return ~val; }

template <>
template <>
constexpr Cast<Point64>::operator QPointF() const { return ~val; }
//------------------------------------------------------------------------------

constexpr double Radius(Point64 p) {
    Point64 c = GetC(p);
    if(p == Point64{}) return std::nan("");
    p = p - c;
    return hypot(p.x, p.y) * dScale;
}

double Perimeter(std::span<const Point64> path, bool open = {});
//------------------------------------------------------------------------------
Path64 CirclePath(double diametr, const Point64& center = Point64{});
Path64 RectanglePath(double width, double height, const Point64& center = Point64{});
//------------------------------------------------------------------------------
void RotatePath(Path64& path, double angle, const Point64& center = Point64{});

Path64& TranslatePath(Path64& path, const Point64& pos);
Paths64& TranslatePaths(Paths64& path, const Point64& pos);

Path64& TransformPath(Path64& path, const QTransform& m);
Paths64& TransformPaths(Paths64& paths, const QTransform& m);
//------------------------------------------------------------------------------

void mergeSegments(Paths64& paths, double glue = {});

/////////////////////////////////////////////////
/// \brief склеивает пути при совпадении конечных точек
/// \param paths - пути
/// \param maxDist - максимальное расстояние между конечными точками
void mergePaths(Paths64& paths, const double dist = {});

QIcon drawIcon(const QPainterPath& pPath, QColor color = Qt::black, bool stroke = false);
QIcon drawIcon(const Paths64& paths, QColor color = Qt::black);

QIcon drawDrillIcon(QColor color = Qt::black);

Paths64& normalize(Paths64& paths);

inline constexpr auto skipFront = v::drop(1);

//------------------------------------------------------------------------------
Paths64 InflatePathsZ(const Paths64& paths, double delta,
    cl::JoinType jt, cl::EndType et, double miterLimit = 2.0,
    double arcTolerance = 0.0);

Paths64 Inflate64(const Paths64& paths, double delta,
    cl::JoinType jt, cl::EndType et,
    double miterLimit = 2.0, double arcTolerance = {});

Paths64 InflateRoundPolygon(const Paths64& paths, double delta,
    double miterLimit = 2.0, double arcTolerance = {});

Paths64 InflateMiterPolygon(const Paths64& paths, double delta,
    double miterLimit = 2.0, double arcTolerance = {});

template <typename T>
inline void CleanPaths(cl::Path<T>& path, double k) {
    path = cl::RamerDouglasPeucker(path, k);
}

template <typename T>
inline void CleanPaths(cl::Paths<T>& paths, double k) {
    paths = cl::RamerDouglasPeucker(paths, k);
}

//------------------------------------------------------------------------------

template <typename T>
inline cl::Path<T>& ReversePath(cl::Path<T>& path) {
    std::reverse(path.begin(), path.end());
    return path;
}
//------------------------------------------------------------------------------

template <typename T>
inline cl::Paths<T>& ReversePaths(cl::Paths<T>& paths) {
    std::for_each(paths.begin(), paths.end(), ReversePath<T>);
    return paths;
}

QDataStream& operator<<(QDataStream& stream, const Point64& pt);
QDataStream& operator>>(QDataStream& stream, Point64& pt);
QDebug operator<<(QDebug d, const Point64& p);

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

// auto operator+=(Container auto& c, r::range auto&& v) {
//     c.reserve(c.size() + r::size(v));
//     c.append_range(std::forward<decltype(v)>(v));
//     return c;
// }

// auto operator+=(Container auto& c, const Container auto& v) {
//     c.reserve(c.size() + v.size());
//     return r::copy(v, std::back_inserter(c)), c;
// }

auto operator-=(Container auto& c, size_t index) {
    return c.erase(c.begin() + index), c;
}

// template <typename T>
// inline cl::Rect<T> GetBounds(const cl::Paths<T>& paths) {
//     cl::Rect<T> rect;

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

inline void SimplifyPolygon(const Path64& /*in_poly*/, Paths64& /*out_polys*/, cl::FillRule /*fillType*/ = cl::FillRule::EvenOdd) {
    //    cl::Clipper64 c;
    //    c.StrictlySimple(true);
    //    c.AddPath(in_poly, PathType::Subject, true);
    //    c.Execute(ClipType::Union, out_polys, fillType, fillType);
}

inline void SimplifyPolygons(const Paths64& /*in_polys*/, Paths64& /*out_polys*/, cl::FillRule /*fillType*/ = cl::FillRule::EvenOdd) {
    //    cl::Clipper64 c;
    //    c.StrictlySimple(true);
    //    c.AddPaths(in_polys, PathType::Subject, true);
    //    c.Execute(ClipType::Union, out_polys, fillType, fillType);
}

inline void SimplifyPolygons(Paths64& polys, cl::FillRule fillType = cl::FillRule::EvenOdd) {
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

void reductionOfDistance(Path64& path, Point64 point = Point64{});

bool pointOnPolygon(const QLineF& l2, const struct Curve& curve, QPointF* ret);

inline /*constexpr*/ double angleTo(const Point64& pt1, const Point64& pt2) noexcept {
    const double dx = static_cast<double>(pt2.x - pt1.x);
    const double dy = static_cast<double>(pt2.y - pt1.y);
    const double theta = atan2(-dy, dx) * 360.0 / (pi * 2);
    const double theta_normalized = theta < 0 ? theta + 360 : theta;
    if(qFuzzyCompare(theta_normalized, double(360)))
        return 0.0;
    else
        return theta_normalized;
}

inline /*constexpr*/ double angleRadTo(const Point64& pt1, const Point64& pt2) noexcept {
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

inline /*constexpr*/ double distTo(const Point64& pt1, const Point64& pt2) noexcept {
    double x_ = static_cast<double>(pt2.x - pt1.x);
    double y_ = static_cast<double>(pt2.y - pt1.y);
    return sqrt(x_ * x_ + y_ * y_);
}

inline constexpr double distToSq(const Point64& pt1, const Point64& pt2) noexcept {
    double x_ = static_cast<double>(pt2.x - pt1.x);
    double y_ = static_cast<double>(pt2.y - pt1.y);
    return (x_ * x_ + y_ * y_);
}

std::span<std::unique_ptr<cl::PolyPath64>> rwPolyTree(PolyTree& polyTree);

Path64 arc(const Point64& center, double radius, double start, double stop, int interpolation);
Path64 arc(Point64 p1, Point64 p2, Point64 center, int interpolation);

void markPolyTreeDByNesting(PolyTree& polynode);
void sortPolyTreeByNesting(PolyTree& polynode);
Pathss64 stacking(Paths64& paths);

Path64 boundOfPaths(const Paths64& paths, /*PType*/ int32_t k);

Paths64& sortB(Paths64& src, Point64 startPt);
Paths64& sortBeginEnd(Paths64& src, Point64 startPt);
Pathss64& sortB(Pathss64& src, Point64 startPt);
Pathss64& sortBeginEnd(Pathss64& src, Point64 startPt);

void addArcTo(QPainterPath& pPath, QPointF source, QPointF target, double bulge);

// #include "curve.h"
