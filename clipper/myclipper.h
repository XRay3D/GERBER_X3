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

//QDebug operator<<(QDebug debug, const IntPoint& p)
//{
//    //QDebugStateSaver saver(debug);
//    debug.nospace() << '(' << p.X << ", " << p.Y << ')';
//    return debug;
//}

#include <CGAL/Aff_transformation_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Polygon_2.h>
#include <QPainterPath>
#include <boost/function_output_iterator.hpp>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = Kernel::Point_2;
using Circle_2 = Kernel::Circle_2;
using Traits_2 = CGAL::Gps_circle_segment_traits_2<Kernel>;
using Polygon_set_2 = CGAL::General_polygon_set_2<Traits_2>;
using Polygon_2 = Traits_2::General_polygon_2;
using Polygon_with_holes_2 = Traits_2::General_polygon_with_holes_2;
using Curve_2 = Traits_2::Curve_2;
using X_monotone_curve_2 = Traits_2::X_monotone_curve_2;

using Transformation = CGAL::Aff_transformation_2<Kernel>;
using Point = CGAL::Point_2<Kernel>;
using Vector = CGAL::Vector_2<Kernel>;
using Direction = CGAL::Direction_2<Kernel>;

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
