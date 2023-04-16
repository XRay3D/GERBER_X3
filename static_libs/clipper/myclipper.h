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

#include "Clipper2Lib/include/clipper2/clipper.h"
// #include "clipper.hpp"
#include <QDebug>
#include <QIcon>
#include <QPolygonF>
#include <numbers>

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
using Clipper2Lib::Bounds;
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

namespace C2 = Clipper2Lib;

Q_DECLARE_METATYPE(Point)

double Perimeter(const Path& path);

Path CirclePath(double diametr, const Point& center = {});

Path RectanglePath(double width, double height, const Point& center = {});

void RotatePath(Path& poligon, double angle, const Point& center = {});

Path& TranslatePath(Path& path, const Point& pos);
Paths& TranslatePaths(Paths& path, const Point& pos);

void mergeSegments(Paths& paths, double glue = 0.0);

void mergePaths(Paths& paths, const double dist = 0.0);

QIcon drawIcon(const Paths& paths, QColor color = Qt::black);

QIcon drawDrillIcon(QColor color = Qt::black);

Paths& normalize(Paths& paths);

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

template <typename T>
inline Clipper2Lib::Paths<T>& ReversePaths(Clipper2Lib::Paths<T>& paths) {
    std::for_each(paths.begin(), paths.end(), ReversePath<T>);
    return paths;
}

//------------------------------------------------------------------------------

// template <typename T>
// inline Clipper2Lib::Rect<T> Bounds(const Clipper2Lib::Paths<T>& paths) {
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
        QLineF l1(pt1, pt2);
        if(QLineF::BoundedIntersection == l1.intersects(l2, &p)) {
            if(ret)
                *ret = (p);
            return true;
        }
    }
    return false;
}
