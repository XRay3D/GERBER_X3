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

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>

#include <boost/function_output_iterator.hpp>

using Kernel = /*              */ CGAL::Exact_predicates_exact_constructions_kernel;
using Traits = /*              */ CGAL::Gps_circle_segment_traits_2<Kernel>;
using Polygon_set_2 = /*       */ CGAL::General_polygon_set_2<Traits>;
using Curve_2 = /*             */ Traits::Curve_2;
using Polygon_2 = /*           */ Traits::Polygon_2;
using Polygon_with_holes_2 = /**/ Traits::Polygon_with_holes_2;
using X_monotone_curve_2 = /*  */ Traits::X_monotone_curve_2;
using Circle_2 = /*            */ Kernel::Circle_2;
using Point_2 = /*             */ Kernel::Point_2;

#include <QPainterPath>
#include <QPen>

Kernel::FT area(Polygon_2 const& P);
Kernel::FT area(Polygon_with_holes_2 const& P);

inline Point_2 operator!(const QPointF pt) { return {pt.x(), pt.y()}; }

std::tuple<Point_2, Point_2, double> arcToBulge(const QPointF& center, double startAngle, double endAngle, double radius);

void TestPolygon(const Polygon_2& pgn, const QPen& pen = {Qt::white, 1.0});
QPainterPath ConstructPath(const Polygon_2& pgn);

void OffsetPoly(const Polygon_2& pgn, double val);

// Construct a polygon from a circle.
Polygon_2 ConstructPolygon(const Circle_2& circle);

// Construct a polygon from a rectangle.
Polygon_2 ConstructPolygon(
    const Point_2& p1, const Point_2& p2,
    const Point_2& p3, const Point_2& p4);

void construct_arc(const Point_2& ps, const Point_2& pt, double bulge, Polygon_2& pgn);

void construct_arc2(const Point_2& ps, const Point_2& pt, const Point_2& center,
    double bulge, double rad, bool ccw, Polygon_2& pgn);
