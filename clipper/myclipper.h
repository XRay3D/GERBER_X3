#pragma once

#include "clipper.hpp"
#include <QDebug>
#include <QPolygonF>

#ifndef M_2PI
#define M_2PI (6.28318530717958647692528676655900576)
#endif

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

#include <QPainterPath>
#include <boost/function_output_iterator.hpp>

//#include <CGAL/Arr_conic_traits_2.h>
//#include <CGAL/Arrangement_2.h>
//#include <CGAL/CORE_algebraic_number_traits.h>
//#include <CGAL/Cartesian.h>

//#include <CGAL/Gps_traits_2.h>
//#include <CGAL/offset_polygon_2.h>

//typedef CGAL::CORE_algebraic_number_traits Nt_traits;
//typedef Nt_traits::Rational Rational;
//typedef CGAL::Cartesian<Rational> Rat_kernel;
//typedef Rat_kernel::Point_2 Point_2 /*Rat_point*/;
//typedef Rat_kernel::Segment_2 Segment_2 /*Rat_segment*/;
//typedef Rat_kernel::Circle_2 Circle_2 /*Rat_circle*/;
//typedef Nt_traits::Algebraic Algebraic;
//typedef CGAL::Cartesian<Algebraic> Alg_kernel;

//typedef CGAL::Arr_conic_traits_2<Rat_kernel, Alg_kernel, Nt_traits> Traits;
//typedef CGAL::Arr_conic_traits_2<Rat_kernel, Alg_kernel, Nt_traits> Traits_2;
//typedef Traits::Point_2 Point;
//typedef Traits::Curve_2 Curve_2 /*Conic_arc*/;
//typedef Traits::X_monotone_curve_2 X_monotone_curve_2 /*X_monotone_conic_arc*/;
//typedef CGAL::Arrangement_2<Traits> Arrangement;

////typedef CGAL::Polygon_2<Rat_kernel> Polygon_2;
//typedef CGAL::Gps_traits_2<Traits> Gps_traits;
//typedef Gps_traits::Polygon_with_holes_2 Offset_polygon_with_holes_2;
//typedef Gps_traits::Polygon_2 Polygon_2;
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Boolean_set_operations_2.h>

using Kernel
    = CGAL::Exact_predicates_exact_constructions_kernel;

using Point_2
    = Kernel::Point_2;
using Circle_2
    = Kernel::Circle_2;


using Traits_2
    = CGAL::Gps_circle_segment_traits_2<Kernel>;

using Arc_point_2
    = Traits_2::Point_2;

using Polygon_set_2
    = CGAL::General_polygon_set_2<Traits_2>;
using Polygon_2
    = Traits_2::Polygon_2;
using Polygon_with_holes_2
    = Traits_2::Polygon_with_holes_2;
using Curve_2
    = Traits_2::Curve_2;
using X_monotone_curve_2
    = Traits_2::X_monotone_curve_2;

#include <CGAL/Aff_transformation_2.h>

using Transformation
    = CGAL::Aff_transformation_2<Kernel>;
using Point
    = CGAL::Point_2<Kernel>;
using Vector
    = CGAL::Vector_2<Kernel>;
using Direction
    = CGAL::Direction_2<Kernel>;

Polygon_2 construct_polygon(const Circle_2& circle);
QPainterPath construct_path(const Polygon_2& pgn);

using namespace ClipperLib;

using Pathss = QVector /*std::vector*/<Paths>;

constexpr cInt uScale = 100000;
constexpr double dScale = 1.0 / uScale;

Path toPath(const QPolygonF& p);
Paths toPaths(const QVector<QPolygonF>& p);

QPolygonF toQPolygon(const Path& p);
QVector<QPolygonF> toQPolygons(const Paths& p);

inline QPointF toQPointF(const IntPoint& p) { return QPointF(p.X * dScale, p.Y * dScale); }
inline IntPoint toIntPoint(const QPointF& p) { return IntPoint(static_cast<cInt>(p.x() * uScale), static_cast<cInt>(p.y() * uScale)); }

double Angle(const IntPoint& pt1, const IntPoint& pt2);
double Length(const IntPoint& pt1, const IntPoint& pt2);
double Perimeter(const Path& path);

//IntPoint Center(const IntPoint& pt1, const IntPoint& pt2)
//{
//    return IntPoint(int((qint64(pt1.X) + pt2.X) / 2), int((qint64(pt1.Y) + pt2.Y) / 2));
//}

Polygon_2 CirclePath2(double diametr, const Point_2& center = {});
Polygon_2 RectanglePath2(double width, double height, const Point_2& center = {});

Path CirclePath(double diametr, const IntPoint& center = IntPoint());
Path RectanglePath(double width, double height, const IntPoint& center = IntPoint());
void RotatePath(Path& poligon, double angle, const IntPoint& center = IntPoint());
void TranslatePath(Path& path, const IntPoint& pos);
