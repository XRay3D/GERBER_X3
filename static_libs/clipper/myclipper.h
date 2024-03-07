/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "mvector.h"
#include <clipper2/clipper.h>
// #include "clipper.hpp"
#include <QDebug>
#include <QIcon>
#include <QPolygonF>
#include <concepts>
#include <numbers>
#include <ranges>

class cancelException : public std::exception {
public:
    cancelException(const char* description)
        : m_descr(description) {
    }
    ~cancelException() noexcept override = default;
    const char* what() const noexcept override { return m_descr.c_str(); }

private:
    std::string m_descr;
};

class ProgressCancel {
    static inline int max_;
    static inline int current_;
    static inline bool cancel_;

public:
    static void reset() {
        current_ = 0;
        max_ = 0;
        cancel_ = 0;
    }

    /////////////////
    /// \brief Progress max
    /// \return
    ///
    static int max() { return max_; }
    /////////////////
    /// \brief Progress setMax
    /// \param max
    ///
    static void setMax(int max) { max_ = max; }

    /////////////////
    /// \brief Progress current
    /// \return
    ///
    static int current() { return current_; }
    /////////////////
    /// \brief Progress setCurrent
    /// \param current
    ///
    static void setCurrent(int current = 0) { current_ = current; }
    /////////////////
    /// \brief Progress incCurrent
    ///
    static void incCurrent() { ++current_; }
    static bool isCancel() { return cancel_; }
    static void ifCancelThenThrow(/*const sl location = sl::current()*/) {
        ++current_;
        if(cancel_) [[unlikely]] {
            //            static std::stringstream ss;
            //            ss.clear();
            //            ss << "file: "
            //               << location.file_name() << "("
            //               << location.line() << ":"
            //               << location.column() << ") `"
            //               << location.function_name();
            //            throw cancelException(ss.str().data() /*__FUNCTION__*/);
            throw cancelException(__FUNCTION__);
        }
    }
    static void setCancel(bool cancel) { cancel_ = cancel; }
};

inline void ifCancelThenThrow() { ProgressCancel::ifCancelThenThrow(); }

enum {
    IconSize = 24
};

constexpr auto sqrt1_2 = std::numbers::sqrt2 * 0.5;
constexpr auto two_pi = std::numbers::pi * 2;
using std::numbers::pi;

// type
using Clipper = Clipper2Lib::Clipper64;
using ClipperOffset = Clipper2Lib::ClipperOffset;
using Path = Clipper2Lib::Path64;
using Paths = Clipper2Lib::Paths64;
using Pathss = mvector<Paths>;
using Point = Clipper2Lib::Point64;
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

using CT = Clipper2Lib::ClipType;
using ET = Clipper2Lib::EndType;
using FR = Clipper2Lib::FillRule;
using JT = Clipper2Lib::JoinType;
using PT = Clipper2Lib::PathType;
using PIPResult = Clipper2Lib::PointInPolygonResult;

namespace CL2 = Clipper2Lib;

Q_DECLARE_METATYPE(Point)

double Perimeter(const Path& path);

Path CirclePath(double diametr, const Point& center = Point{});

Path RectanglePath(double width, double height, const Point& center = Point{});

void RotatePath(Path& poligon, double angle, const Point& center = Point{});

Path& TranslatePath(Path& path, const Point& pos);
Paths& TranslatePaths(Paths& path, const Point& pos);

void mergeSegments(Paths& paths, double glue = 0.0);

void mergePaths(Paths& paths, const double dist = 0.0);

QIcon drawIcon(const Paths& paths, QColor color = Qt::black);

QIcon drawDrillIcon(QColor color = Qt::black);

Paths& normalize(Paths& paths);

inline constexpr auto skipFront = std::views::drop(1);

//------------------------------------------------------------------------------

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
inline QDebug operator<<(QDebug d, const Point& p) {
    return d << "Point(" << p.x << ", " << p.y << ")";
}

//----Container helpers----------------------------------------------------------------

template <typename T> concept Container = requires(T c) {
    c.begin();
    c.end();
    // { typename T::value_type{} } -> std::same_as<std::decay_t<decltype(*c.begin())>>;
    // T::value_type;
    // { std::is_same<T, QByteArray> } -> std::same_as<std::false_type>;
    // { std::is_same<T, QString> } -> std::same_as<std::false_type>;
};

template <Container C>
inline auto join(C&& cont) {
    auto j = std::views::join(cont);
    return std::decay_t<decltype(*cont.begin())>{j.begin(), j.end()};
};

template <typename Cont>
inline int indexOf(const Cont& c, const typename Cont::value_type& v) {
    auto it = std::find(c.begin(), c.end(), v);
    return it == c.end() ? -1 : std::distance(c.begin(), it);
}

auto operator+=(Container auto& c, Container auto&& v) {
    c.reserve(c.size() + v.size());
    return std::ranges::move(v, std::back_inserter(c)), c;
}

auto operator+=(Container auto& c, const Container auto& v) {
    c.reserve(c.size() + v.size());
    return std::ranges::copy(v, std::back_inserter(c)), c;
}

auto operator-=(Container auto& c, size_t index) {
    return c.erase(c.begin() + index), c;
}

//------------------------------------------------------------------------------

template <typename T> concept Arithmetic = std::is_arithmetic_v<T>;
inline Point& operator*=(Point& pt, Arithmetic auto v) {
    return pt.x *= v, pt.y *= v, pt;
}

static constexpr auto uScale{100'000};
static constexpr auto dScale{1. / uScale};

inline Point operator~(const QPointF pt) { return Point{pt.x() * uScale, pt.y() * uScale}; }
inline QPointF operator~(const Point pt) { return {pt.x * dScale, pt.y * dScale}; }

#define TRANSFORM(FROM, TO)                                                    \
    inline TO operator~(const FROM& val) {                                     \
        auto it = std::views::transform(val, [](auto&& val) { return ~val; }); \
        TO ret;                                                                \
        ret.reserve(val.size());                                               \
        std::ranges::move(it, std::back_inserter(ret));                        \
        return ret;                                                            \
    }

TRANSFORM(QPolygonF, Path)
TRANSFORM(Path, QPolygonF)

TRANSFORM(QList<QPolygonF>, Paths)
TRANSFORM(Paths, QList<QPolygonF>)

#undef TRANSFORM
//------------------------------------------------------------------------------

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

inline void SimplifyPolygon(const Path& in_poly, Paths& out_polys, Clipper2Lib::FillRule fillType = Clipper2Lib::FillRule::EvenOdd) {
    //    Clipper c;
    //    c.StrictlySimple(true);
    //    c.AddPath(in_poly, PathType::Subject, true);
    //    c.Execute(ClipType::Union, out_polys, fillType, fillType);
}

inline void SimplifyPolygons(const Paths& in_polys, Paths& out_polys, Clipper2Lib::FillRule fillType = Clipper2Lib::FillRule::EvenOdd) {
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
        return !qFuzzyIsNull(a) | !qFuzzyIsNull(b);
    }
    double distance(const QPointF& p) const {
        return abs(a * p.x() + b * p.y() + c) / sqrt(a * a + b * b);
    }
    double lenght() const { return sqrt(a * a + b * b); }
};

inline bool pointOnPolygon(const QLineF& l2, const Path& path, Point* ret) {
    const size_t cnt = path.size();
    if(cnt < 2)
        return false;
    QPointF p;
    for(size_t i = 0; i < cnt; ++i) {
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
    const double dx = pt2.x - pt1.x;
    const double dy = pt2.y - pt1.y;
    const double theta = atan2(-dy, dx) * 360.0 / (pi * 2);
    const double theta_normalized = theta < 0 ? theta + 360 : theta;
    if(qFuzzyCompare(theta_normalized, double(360)))
        return 0.0;
    else
        return theta_normalized;
}

inline /*constexpr*/ double angleRadTo(const Point& pt1, const Point& pt2) noexcept {
    const double dx = pt2.x - pt1.x;
    const double dy = pt2.y - pt1.y;
    const double theta = atan2(-dy, dx);
    return theta;
    const double theta_normalized = theta < 0 ? theta + (pi * 2) : theta; // NOTE theta_normalized
    if(qFuzzyCompare(theta_normalized, (pi * 2)))
        return 0.0;
    else
        return theta_normalized;
}

inline /*constexpr*/ double distTo(const Point& pt1, const Point& pt2) noexcept {
    double x_ = pt2.x - pt1.x;
    double y_ = pt2.y - pt1.y;
    return sqrt(x_ * x_ + y_ * y_);
}

inline constexpr double distToSq(const Point& pt1, const Point& pt2) noexcept {
    double x_ = pt2.x - pt1.x;
    double y_ = pt2.y - pt1.y;
    return (x_ * x_ + y_ * y_);
}
