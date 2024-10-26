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

// #include <boost/function_output_iterator.hpp>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>

#include <QDebug>
#include <QPainterPath>
#include <qmath.h>

// #include <QGraphicsScene>
// #include <QGraphicsView>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Gps_circle_segment_traits_2<Kernel> Traits_2;
typedef CGAL::General_polygon_set_2<Traits_2> Polygon_set_2;
typedef Traits_2::General_polygon_2 Polygon_2;
typedef Traits_2::General_polygon_with_holes_2 Polygon_with_holes_2;
typedef Traits_2::Curve_2 Curve_2;
typedef Traits_2::X_monotone_curve_2 X_monotone_curve_2;

static QPainterPath construct_path(const Polygon_2& pgn) {
    QPainterPath result;

    Q_ASSERT(pgn.orientation() == CGAL::CLOCKWISE || pgn.orientation() == CGAL::COUNTERCLOCKWISE);

    // Degenerate polygon, ring.size() < 3
    if(pgn.orientation() == CGAL::ZERO) {
        qWarning() << "construct_path: Ignoring degenerated polygon";
        return result;
    }

    const bool isClockwise = pgn.orientation() == CGAL::CLOCKWISE;

    Polygon_2::Curve_const_iterator current = pgn.curves_begin();
    Polygon_2::Curve_const_iterator end = pgn.curves_end();

    result.moveTo(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y()));

    do {
        const Polygon_2::X_monotone_curve_2& curve = *current;
        const auto& source = curve.source();
        const auto& target = curve.target();

        if(curve.is_linear()) {
            result.lineTo(QPointF(CGAL::to_double(target.x()),
                CGAL::to_double(target.y())));
        } else if(curve.is_circular()) {
            const auto bbox = curve.supporting_circle().bbox();
            const QRectF rect(QPointF(bbox.xmin(), bbox.ymin()), QPointF(bbox.xmax(), bbox.ymax()));
            const auto center = curve.supporting_circle().center();
            const double asource = std::atan2(CGAL::to_double(source.y() - center.y()), CGAL::to_double(source.x() - center.x()));
            const double atarget = std::atan2(CGAL::to_double(target.y() - center.y()), CGAL::to_double(target.x() - center.x()));
            double aspan = atarget - asource;
            if(aspan < -CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && !isClockwise)) aspan += 2.0 * CGAL_PI;
            else if(aspan > +CGAL_PI || (qFuzzyCompare(aspan, +CGAL_PI) && isClockwise)) aspan -= 2.0 * CGAL_PI;
            result.arcTo(rect, qRadiansToDegrees(-asource), qRadiansToDegrees(-aspan));
        } else { // ?!?
            Q_UNREACHABLE();
        }
    } while(++current != end);

    return result;
}

// #include "cavc/polyline.hpp"
// #include "cavc/vector2.hpp"
// #include <QColor>
// #include <QDataStream>
// #include <QIcon>
// #include <QLineF>
// #include <QPainterPath>
// #include <QPolygonF>
// #include <iterator>
// #include <numeric>
// #include <qchar.h>
// #include <qnamespace.h>

// #include "cavc/intrcircle2circle2.hpp"
// #include "cavc/intrlineseg2circle2.hpp"
// #include "cavc/intrlineseg2lineseg2.hpp"
// #include "cavc/mathutils.hpp"
// #include "cavc/plinesegment.hpp"
// #include "cavc/polyline.hpp"
// #include "cavc/polylinecombine.hpp"
// #include "cavc/polylineintersects.hpp"
// #include "cavc/polylineoffset.hpp"
// #include "cavc/polylineoffsetislands.hpp"
// #include "cavc/staticspatialindex.hpp"
// #include "cavc/vector.hpp"

// // static constexpr double two_pi = 2.0 * std::numbers::pi;

// using Poly = cavc::Polyline<double>;
// using Polys = std::vector<Poly>;
// using Polyss = std::vector<Polys>;
// using Vec2 = cavc::Vector2<double>;
// using PlineVert = cavc::PlineVertex<double>;

// // static inline cavc::PlineVertex<double> operator~(const QPointF pv) { return {pv.x(), pv.y(), 0.0}; }

// // inline QPolygonF operator~(const cavc::Polyline<double>& poly) {
// //     QPolygonF ret;
// //     auto aprox = cavc::convertArcsToLines(poly, 0.01);
// //     ret.reserve(aprox.size());
// //     for(auto&& vert: aprox.vertexes()) ret.push_back({vert.x(), vert.y()});
// //     return ret;
// // }

// inline QPointF operator~(const Vec2& vec2) { return {vec2.x(), vec2.y()}; }
// inline QPointF operator~(const PlineVert& vec2) { return {vec2.x(), vec2.y()}; }
// inline QRectF operator~(const cavc::AABB<double>& aabb) {
//     return {
//         aabb.xMin,
//         aabb.yMin,
//         aabb.xMax - aabb.xMin,
//         aabb.yMax - aabb.yMin,
//     };
// }

// std::tuple<Vec2, Vec2, double> arcToBulge(const Vec2& center, double startAngle, double endAngle, double radius);

// inline QDataStream& operator>>(QDataStream& stream, Vec2& vec2) {
//     stream >> vec2.x();
//     stream >> vec2.y();
//     return stream;
// }

// inline QDataStream& operator<<(QDataStream& stream, const Vec2& vec2) {
//     stream << vec2.x();
//     stream << vec2.y();
//     return stream;
// }

// inline QDataStream& operator>>(QDataStream& stream, cavc::PlineVertex<double>& vertex) {
//     stream >> vertex.pos();
//     stream >> vertex.bulge();
//     return stream;
// }

// inline QDataStream& operator<<(QDataStream& stream, const cavc::PlineVertex<double>& vertex) {
//     stream << vertex.pos();
//     stream << vertex.bulge();
//     return stream;
// }

// inline QDataStream& operator>>(QDataStream& stream, Poly& poly) {
//     uint32_t n;
//     stream >> n;
//     poly.vertexes().resize(n);
//     for(auto&& vertex: poly.vertexes())
//         stream >> vertex;
//     return stream;
// }

// inline QDataStream& operator<<(QDataStream& stream, const Poly& poly) {
//     stream << uint32_t(poly.size());
//     for(auto&& vertex: poly.vertexes())
//         stream << vertex;
//     return stream;
// }

// void RotatePoly(Poly& poly, double angle, const Vec2& center = {});

// Poly& ReversePoly(Poly& poly);

// inline double Length(const Poly& poly) { return cavc::getPathLength(poly); }

// inline double Area(const Poly& poly) { return cavc::getArea(poly); }

// inline double Area(const Polys& polys) {
//     double area{};
//     for(auto&& poly: polys) area += Area(poly);
//     return area;
// }

// inline Polys& ReversePolys(Polys& polys) {
//     std::ranges::for_each(polys, ReversePoly);
//     return polys;
// }

// Poly CirclePoly(double diametr, const Vec2& center = {});

// Poly RectanglePoly(double width, double height, const Vec2& center = {});

// QPainterPath polyToPPath(const Polys& polys);

// inline QPainterPath polyToPPath(const Poly& poly) { return polyToPPath(Polys{poly}); }

// Poly& CleanPoly(Poly& poly);

// enum class PolyType {
//     Line,
//     Poly
// };

// Polys OffsetPoly(const Poly& poly, double offset, PolyType line = {});

// QIcon drawIcon(const Polys& polys, const QColor& color);

// void TestPoly(const Polys& polys, const QColor& color = Qt::white, const QString& toolTip = {});

// // inline void TestPoly(const Poly& poly, const QColor& color) { TestPoly(Polys{poly}, color); }
