/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
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

enum { IconSize = 24 };
constexpr auto sqrt1_2 = std::numbers::sqrt2 * 0.5;
constexpr auto two_pi = std::numbers::pi * 2;
using std::numbers::pi;

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
// type
using Clipper = Clipper2Lib::Clipper64;
using ClipperOffset = Clipper2Lib::ClipperOffset;
using Path = Clipper2Lib::PathI;
using Paths = Clipper2Lib::PathsI;
using Pathss = mvector<Paths>;
using Point = Clipper2Lib::PointI;
using PolyTree = Clipper2Lib::PolyTreeI;
using Rect = Clipper2Lib::RectI;
// func
using Clipper2Lib::Area;
using Clipper2Lib::Bounds;
using Clipper2Lib::PointInPolygon;
// enum
using Clipper2Lib::ClipType;
using Clipper2Lib::EndType;
using Clipper2Lib::FillRule;
using Clipper2Lib::JoinType;
using Clipper2Lib::PathType;
using Clipper2Lib::PointInPolygonResult;

Q_DECLARE_METATYPE(Point)

double Perimeter(const Path& path);

Path CirclePath(double diametr, const Point& center = {});

Path RectanglePath(double width, double height, const Point& center = {});

void RotatePath(Path& poligon, double angle, const Point& center = {});

void TranslatePath(Path& path, const Point& pos);

void mergeSegments(Paths& paths, double glue = 0.0);

void mergePaths(Paths& paths, const double dist = 0.0);

QIcon drawIcon(const Paths& paths);

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
