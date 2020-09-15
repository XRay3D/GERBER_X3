// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "myclipper.h"
#include "qmath.h"
#include "settings.h"
#include <QElapsedTimer>
#include <QLineF>

#include "scene.h"
#include <QDateTime>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QtMath>
#include <cmath>
#include <list>
#include <math.h>

// Construct a polygon from a circle.
Polygon_2 construct_polygon(const Circle_2& circle)
{
    // Subdivide the circle into two x-monotone arcs.
    Traits_2 traits;
    Curve_2 curve(circle);
    std::list<CGAL::Object> objects;
    traits.make_x_monotone_2_object()(curve, std::back_inserter(objects));
    CGAL_assertion(objects.size() == 2);
    // Construct the polygon.
    Polygon_2 pgn;
    X_monotone_curve_2 arc;
    std::list<CGAL::Object>::iterator iter;
    for (iter = objects.begin(); iter != objects.end(); ++iter) {
        CGAL::assign(arc, *iter);
        pgn.push_back(arc);
    }
    return pgn;
}
// Construct a polygon from a rectangle.
Polygon_2 construct_polygon(const Point_2& p1, const Point_2& p2, const Point_2& p3, const Point_2& p4)
{
    Polygon_2 pgn;
    X_monotone_curve_2 s1(p1, p2);
    pgn.push_back(s1);
    X_monotone_curve_2 s2(p2, p3);
    pgn.push_back(s2);
    X_monotone_curve_2 s3(p3, p4);
    pgn.push_back(s3);
    X_monotone_curve_2 s4(p4, p1);
    pgn.push_back(s4);
    return pgn;
}

//typedef typename Kernel::Point_2 CGAL_Point_2;
//typedef typename Kernel::Segment_2 CGAL_Segment_2;
//typedef typename Kernel::Ray_2 CGAL_Ray_2;
//typedef typename Kernel::Line_2 CGAL_Line_2;
//typedef typename Kernel::Triangle_2 CGAL_Triangle_2;
//typedef typename Kernel::Iso_rectangle_2 CGAL_Iso_rectangle_2;
//QPointF operator()(const CGAL_Point_2& p)
//{
//    return QPointF(to_double(p.x()), to_double(p.y()));
//}
//QPointF operator()(const CGAL_Circular_arc_point_2& p) const
//{
//    return QPointF(to_double(p.x()), to_double(p.y()));
//}
//CGAL_Segment_2 operator()(const QLineF& qs) const
//{
//    return CGAL_Segment_2(operator()(qs.p1()), operator()(qs.p2()));
//}
//QLineF operator()(const CGAL_Segment_2& s) const
//{
//    return QLineF(operator()(s.source()), operator()(s.target()));
//}
//CGAL_Iso_rectangle_2 operator()(const QRectF& qr) const
//{
//    return CGAL_Iso_rectangle_2(operator()(qr.bottomLeft()), operator()(qr.topRight()));
//}
//QRectF operator()(const CGAL_Iso_rectangle_2& r) const
//{
//    return QRectF(operator()(r[3]), operator()(r[1])).normalized(); // top left, bottom right
//}
//QRectF operator()(const CGAL::Bbox_2& bb) const
//{
//    return QRectF(bb.xmin(),
//        bb.ymin(),
//        bb.xmax() - bb.xmin(),
//        bb.ymax() - bb.ymin());
//}

QPainterPath construct_path(const Polygon_2& pgn)
{
    QPainterPath result;
    auto toQRectF = [](const CGAL::Bbox_2& bb) -> QRectF {
        return QRectF(
            bb.xmin(),
            bb.ymin(),
            bb.xmax() - bb.xmin(),
            bb.ymax() - bb.ymin());
    };
    auto r = toQRectF(pgn.bbox());
    if (r.width() * r.height() < 0.001)
        return result;
    if constexpr (0) {
        auto current = pgn.curves_begin();
        auto end = pgn.curves_end();
        result.moveTo(QPointF(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y())));
        do {
            const auto& curve = *current;
            const QPointF target = QPointF(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
            if (curve.is_linear()) {
                result.lineTo(target);
            } else if (curve.is_circular()) {
                const bool isClockwise = (curve.supporting_circle().orientation() == CGAL::CLOCKWISE);
                const QRectF rect = toQRectF(curve.supporting_circle().bbox());
                const QPointF center = QPointF(CGAL::to_double(curve.supporting_circle().center().x()), CGAL::to_double(curve.supporting_circle().center().y()));
                const QPointF source = QPointF(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
                const QPointF p1 = source - center;
                const QPointF p2 = target - center;
                const double asource = qAtan2(p1.y(), p1.x());
                const double atarget = qAtan2(p2.y(), p2.x());
                double aspan = atarget - asource;
                if (aspan < -CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && !isClockwise))
                    aspan += 2.0 * CGAL_PI;
                else if (aspan > CGAL_PI || (qFuzzyCompare(aspan, CGAL_PI) && isClockwise))
                    aspan -= 2.0 * CGAL_PI;
                result.arcTo(rect.normalized(), qRadiansToDegrees(-asource), qRadiansToDegrees(-aspan));
            }
        } while (++current != end);
    } else {
        Q_ASSERT(pgn.orientation() == CGAL::CLOCKWISE || pgn.orientation() == CGAL::COUNTERCLOCKWISE);
        // Degenerate polygon, ring.size() < 3
        if (pgn.orientation() == CGAL::ZERO) {
            qWarning() << "construct_path: Ignoring degenerated polygon";
            return result;
        }
        const bool isClockwise = pgn.orientation() == CGAL::CLOCKWISE;
        auto current = pgn.curves_begin();
        auto end = pgn.curves_end();
        result.moveTo(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y()));
        do {
            const auto& curve = *current;
            const auto& source = curve.source();
            const auto& target = curve.target();
            if (curve.is_linear()) {
                result.lineTo(QPointF(CGAL::to_double(target.x()), CGAL::to_double(target.y())));
            } else if (curve.is_circular()) {
                const QRectF rect(toQRectF(curve.supporting_circle().bbox()));
                const auto center = curve.supporting_circle().center();
                const double asource = qAtan2(CGAL::to_double(source.y() - center.y()), CGAL::to_double(source.x() - center.x()));
                const double atarget = qAtan2(CGAL::to_double(target.y() - center.y()), CGAL::to_double(target.x() - center.x()));
                double aspan = atarget - asource;
                if (aspan < -CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && !isClockwise))
                    aspan += 2.0 * CGAL_PI;
                else if (aspan > CGAL_PI || (qFuzzyCompare(aspan, CGAL_PI) && isClockwise))
                    aspan -= 2.0 * CGAL_PI;
                result.arcTo(rect, qRadiansToDegrees(-asource), qRadiansToDegrees(-aspan));
            } else { // ?!?
                Q_UNREACHABLE();
            }
        } while (++current != end);
        result.lineTo(CGAL::to_double(pgn.curves_begin()->source().x()), CGAL::to_double(pgn.curves_begin()->source().y()));
    }
    return result;
}

/////////////////////////////////////////////////////////////////////
/// \brief toPath
/// \param p
/// \return
///
///
///
///
///
///

Path toPath(const QPolygonF& p)
{
    Path path;
    path.reserve(p.size());
    for (const QPointF& pt : p)
        path.push_back(toIntPoint(pt));
    return path;
}

Paths toPaths(const QVector<QPolygonF>& p)
{
    Paths paths;
    paths.reserve(p.size());
    for (const QPolygonF& pl : p)
        paths.push_back(toPath(pl));
    return paths;
}

QPolygonF toQPolygon(const Path& p)
{
    QPolygonF polygon;
    polygon.reserve(p.size());
    for (const IntPoint& pt : p)
        polygon.push_back(toQPointF(pt));
    return polygon;
}

QVector<QPolygonF> toQPolygons(const Paths& p)
{
    QVector<QPolygonF> polygons;
    polygons.reserve(p.size());
    for (const Path& pl : p)
        polygons.push_back(toQPolygon(pl));
    return polygons;
}

double Angle(const IntPoint& pt1, const IntPoint& pt2)
{
    const double dx = pt2.X - pt1.X;
    const double dy = pt2.Y - pt1.Y;
    const double theta = atan2(-dy, dx) * 360.0 / M_2PI;
    const double theta_normalized = theta < 0 ? theta + 360 : theta;
    if (qFuzzyCompare(theta_normalized, double(360)))
        return 0.0;
    else
        return theta_normalized;
}

double Length(const IntPoint& pt1, const IntPoint& pt2)
{
    double x = pt2.X - pt1.X;
    double y = pt2.Y - pt1.Y;
    return sqrt(x * x + y * y);
}

//static inline bool qt_is_finite(double d)
//{
//    uchar* ch = (uchar*)&d;
//#ifdef QT_ARMFPA
//    return (ch[3] & 0x7f) != 0x7f || (ch[2] & 0xf0) != 0xf0;
//#else
//    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
//        return (ch[0] & 0x7f) != 0x7f || (ch[1] & 0xf0) != 0xf0;
//    } else {
//        return (ch[7] & 0x7f) != 0x7f || (ch[6] & 0xf0) != 0xf0;
//    }
//#endif
//}

Path CirclePath(double diametr, const IntPoint& center)
{
    if (diametr == 0.0)
        return Path();

    const double radius = diametr * 0.5;
    const int intSteps = GlobalSettings::gbrGcCircleSegments(radius * dScale);
    Path poligon(intSteps);
    for (int i = 0; i < intSteps; ++i) {
        poligon[i] = IntPoint(
            static_cast<cInt>(cos(i * 2 * M_PI / intSteps) * radius) + center.X,
            static_cast<cInt>(sin(i * 2 * M_PI / intSteps) * radius) + center.Y);
    }
    return poligon;
}

Path RectanglePath(double width, double height, const IntPoint& center)
{

    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Path poligon {
        IntPoint(static_cast<cInt>(-halfWidth + center.X), static_cast<cInt>(+halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(-halfWidth + center.X), static_cast<cInt>(-halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(+halfWidth + center.X), static_cast<cInt>(-halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(+halfWidth + center.X), static_cast<cInt>(+halfHeight + center.Y)),
    };
    if (Area(poligon) < 0.0)
        ReversePath(poligon);

    return poligon;
}

void RotatePath(Path& poligon, double angle, const IntPoint& center)
{
    const bool fl = Area(poligon) < 0;
    for (IntPoint& pt : poligon) {
        const double dAangle = qDegreesToRadians(angle - Angle(center, pt));
        const double length = Length(center, pt);
        pt = IntPoint(static_cast<cInt>(cos(dAangle) * length), static_cast<cInt>(sin(dAangle) * length));
        pt.X += center.X;
        pt.Y += center.Y;
    }
    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

void TranslatePath(Path& path, const IntPoint& pos)
{
    if (pos.X == 0 && pos.Y == 0)
        return;
    for (Path::size_type i = 0, size = path.size(); i < size; ++i) {
        path[i].X += pos.X;
        path[i].Y += pos.Y;
    }
}

double Perimeter(const Path& path)
{
    double p = 0.0;
    for (int i = 0, j = path.size() - 1; i < path.size(); ++i) {
        double x = path[j].X - path[i].X;
        double y = path[j].Y - path[i].Y;
        p += x * x + y * y;
        j = i;
    }
    return sqrt(p);
}

Polygon_2 CirclePath2(double diametr, const Point_2& center)
{
    diametr *= 0.5;
    const Circle_2 circle(center, diametr * diametr);

    // Subdivide the circle into two x-monotone arcs.
    Traits_2 traits;
    Curve_2 curve(circle);
    std::list<CGAL::Object> objects;
    traits.make_x_monotone_2_object()(curve, std::back_inserter(objects));
    CGAL_assertion(objects.size() == 2);
    // Construct the polygon.
    Polygon_2 pgn;
    X_monotone_curve_2 arc;
    std::list<CGAL::Object>::iterator iter;
    for (iter = objects.begin(); iter != objects.end(); ++iter) {
        CGAL::assign(arc, *iter);
        pgn.push_back(arc);
    }
    return pgn;
}

Polygon_2 RectanglePath2(double width, double height, const Point_2& center)
{
    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    QVector<Point_2> p {
        Point_2(-halfWidth + center.x(), +halfHeight + center.y()),
        Point_2(-halfWidth + center.x(), -halfHeight + center.y()),
        Point_2(+halfWidth + center.x(), -halfHeight + center.y()),
        Point_2(+halfWidth + center.x(), +halfHeight + center.y()),
    };
    Polygon_2 pgn;
    pgn.push_back(X_monotone_curve_2(p[0], p[1]));
    pgn.push_back(X_monotone_curve_2(p[1], p[2]));
    pgn.push_back(X_monotone_curve_2(p[2], p[3]));
    pgn.push_back(X_monotone_curve_2(p[3], p[0]));
    if (pgn.orientation() == CGAL::NEGATIVE)
        pgn.reverse_orientation();
    return pgn;
}
