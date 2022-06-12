// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_lwpolyline.h"
#include "app.h"
#include "scene.h"

#include <QPainterPath>
#include <numbers>

/*
static QPainterPath construct_path(const Polygon_2& pgn) {
    QPainterPath result;

    Q_ASSERT(pgn.orientation() == CGAL::CLOCKWISE || pgn.orientation() == CGAL::COUNTERCLOCKWISE);

    // Degenerate polygon, ring.size() < 3
    if (pgn.orientation() == CGAL::ZERO) {
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

        if (curve.is_linear()) {
            result.lineTo(QPointF(CGAL::to_double(target.x()), CGAL::to_double(target.y())));
        } else if (curve.is_circular()) {
            const auto bbox = curve.supporting_circle().bbox();
            const QRectF rect(QPointF(bbox.xmin(), bbox.ymin()), QPointF(bbox.xmax(), bbox.ymax()));
            const auto center = curve.supporting_circle().center();
            const double asource = qAtan2(CGAL::to_double(source.y() - center.y()), CGAL::to_double(source.x() - center.x()));
            const double atarget = qAtan2(CGAL::to_double(target.y() - center.y()), CGAL::to_double(target.x() - center.x()));
            double aspan = atarget - asource;
            if (aspan < -CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && !isClockwise))
                aspan += 2.0 * CGAL_PI;
            else if (aspan > CGAL_PI || (qFuzzyCompare(aspan, CGAL_PI) && isClockwise))
                aspan -= 2.0 * CGAL_PI;
            result.arcTo(rect, ToDeg(-asource), ToDeg(-aspan));
        } else { // ?!?
            Q_UNREACHABLE();
        }
    } while (++current != end);

    return result;
}
*/

void swap(Dxf::LwPolyline::Segment l, Dxf::LwPolyline::Segment r) {
    auto t = l;
    l = r;
    r = t;
}

namespace Dxf {

LwPolyline::LwPolyline(SectionParser* sp)
    : Entity(sp) { }

// void LwPolyline::draw(const InsertEntity* const i) const
//{
//     if (i) {
//         for (int r = 0; r < i->rowCount; ++r) {
//             for (int c = 0; c < i->colCount; ++c) {
//                 QPointF tr(r * i->rowSpacing, r * i->colSpacing);
//                 GraphicObject go(toGo());
//                 i->transform(go, tr);
//                 i->attachToLayer(std::move(go));
//             }
//         }
//     } else {
//         attachToLayer(toGo());
//     }
// }

void LwPolyline::parse(CodeData& code) {
    Segment pt;
    do {
        // data.push_back(code);
        switch (static_cast<DataEnum>(code.code())) {
        case SubclassMrker:
            break;
        case NumberOfVertices:
            numberOfVertices = code;
            break;
        case PolylineFlag:
            polylineFlag = code;
            break;
        case ConstantWidth:
            constantWidth = code;
            break;
        case Elevation:
            elevation = code;
            break;
        case Thickness:
            thickness = code;
            break;
        case VertexCoordinatesX:
            pt.rx() = code;
            break;
        case VertexCoordinatesY:
            pt.ry() = code;
            poly << pt;
            break;
        case VertexID:
            break;
        case StartWidth:
            startWidth = code;
            break;
        case EndWidth:
            endWidth = code;
            break;
        case Bulge: // betven points
            poly.back().bulge = code;
            break;
        case ExtrusionDirectionX:
            break;
        case ExtrusionDirectionY:
            break;
        case ExtrusionDirectionZ:
            break;
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

Entity::Type LwPolyline::type() const { return Type::LWPOLYLINE; }

#define ToDeg qRadiansToDegrees
#define ToRad qDegreesToRadians

GraphicObject LwPolyline::toGo() const {
    QPainterPath path;
    const bool dbg = false; // data.first().line() == 17844; /*|| data.first().line() == 18422;*/

    auto addSeg = [&path, dbg](const Segment& p1, const Segment& p2, bool fl) {
        if (path.isEmpty())
            path.moveTo(p1);

        if (qFuzzyIsNull(p1.bulge)) {
            path.lineTo(p2);
            return;
        }

        const QLineF l1(p1, p2);
        const double lenght = l1.length() * 0.5;
        const double height = lenght * p1.bulge;
        // const double radius = (height * height + lenght * lenght) / (height * 2);
        const double radius = height * 0.5 + l1.length() * l1.length() / (8 * height);

        QLineF l2((p1 + p2) / 2, p2);
        l2 = l2.normalVector();
        l2.setLength(height);
        QPointF c(l2.p2());

        //;; Bulge Center  -  Lee Mac
        //;; p1 - start vertex
        //;; p2 - end vertex
        //;; b  - bulge
        //;; Returns the center of the arc described by the given bulge and vertices

        {
            //                        (defun LM:BulgeCenter ( p1 p2 b )
            //                         (polar p1
            //                          (+ (angle p1 p2) (- (/ pi 2) (* 2 (atan b))))
            //                          (/ (* (distance p1 p2) (1+ (* b b))) 4 b)
            //                          )
            //                         )
        }
        auto b = p1.bulge;
        auto угол = QLineF(p1, p2).angle() + ToDeg((pi / 2) - (2 * atan(b)));

        qDebug("\tугол  %3.4f", угол);

        auto расстояние = (QLineF(p1, p2).length() * (1 + b * b) / 4 / b);
        auto cl = QLineF::fromPolar(расстояние, -угол).p2() + p1;

        ////////////////////////
        QPointF center;
        {
            const double ax2 = p1.x() * p1.x();
            const double ay2 = p1.y() * p1.y();

            const double bx2 = p2.x() * p2.x();
            const double by2 = p2.y() * p2.y();

            const double cx2 = c.x() * c.x();
            const double cy2 = c.y() * c.y();

            double d = p1.x() * p2.y() + p1.y() * c.x() - p2.y() * c.x() - p1.x() * c.y() - p2.x() * p1.y() + p2.x() * c.y();
            center = QPointF(
                +0.5 / d * (                                                                                //
                    p1.y() * cx2 + p1.y() * cy2 + p2.y() * ax2 + p2.y() * ay2 + c.y() * bx2 + c.y() * by2 - //
                    p1.y() * bx2 - p1.y() * by2 - p2.y() * cx2 - p2.y() * cy2 - c.y() * ax2 - c.y() * ay2),
                -0.5 / d * (                                                                                //
                    p1.x() * cx2 + p1.x() * cy2 + p2.x() * ax2 + p2.x() * ay2 + c.x() * bx2 + c.x() * by2 - //
                    p1.x() * bx2 - p1.x() * by2 - p2.x() * cx2 - p2.x() * cy2 - c.x() * ax2 - c.x() * ay2));
        }

        QPointF p;
        p = p1 - center;
        double a1 = atan2(p.x(), p.y()) + (fl ? +.5 : -.5) * pi;
        p = p2 - center;
        double a2 = atan2(p.x(), p.y()) + (fl ? +.5 : -.5) * pi;

        double aspan = a2 - a1;
        const bool isClockwise { p1.bulge < 0 };

        /**/ if (aspan < -pi || (qFuzzyCompare(aspan, -pi) && !isClockwise))
            aspan += 2.0 * pi;
        else if (aspan > +pi || (qFuzzyCompare(aspan, +pi) && isClockwise))
            aspan -= 2.0 * pi;

        const QPointF rad(radius, radius);
        const QRectF rect(center + rad, center - rad);

        path.arcTo(rect, ToDeg(a1), ToDeg(aspan));

        qDebug("\tS %.3f T %.3f SP %.3f", ToDeg(a1), ToDeg(a2), ToDeg(aspan));

        {
            QPainterPath myPath;
            myPath.moveTo(center);
            myPath.arcTo(rect, ToDeg(a1), ToDeg(aspan));
            auto item = App::scene()->addPath(myPath, { Qt::magenta, 0 });
            item->setZValue(std::numeric_limits<double>::max());
            item->setToolTip(QString("%1\n%2\n%3\n").arg(ToDeg(a1)).arg(ToDeg(a2)).arg(ToDeg(aspan)));
        }
        auto lll = QLineF::fromPolar(расстояние, угол).translated(p1);
        App::scene()->addLine(lll, { Qt::green, 0 });
        App::scene()->addLine({ lll.p2(), center }, { Qt::yellow, 0 });
    };

    Path path_;
    for (QPointF p : poly)
        path_.push_back(p);
    bool fl = ClipperLib::Area(path_) > 0;

    for (size_t i = 0, size = poly.size() - 1; i < size; ++i)
        addSeg(poly[i], poly[i + 1], fl);

    if (polylineFlag == Closed)
        addSeg(poly.back(), poly.front(), fl);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMatrix m;
    m.scale(u, u);
    QPainterPath path2;
    for (auto& poly : path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QMatrix m2;
    m2.scale(d, d);
    auto p(path2.toSubpathPolygons(m2));
#else
    QTransform m;
    m.scale(u, u);
    QPainterPath path2;
    for (auto& poly : path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QTransform m2;
    m2.scale(d, d);
    auto p(path2.toSubpathPolygons(m2));
#endif

    Paths paths;
    if (constantWidth) {
        ClipperOffset offset;
        offset.AddPath(p.value(0), jtRound, polylineFlag == Closed ? etClosedLine : etOpenRound);
        offset.Execute(paths, constantWidth * uScale * 0.5);
    }

    return { id, p.value(0), paths };
}

void LwPolyline::write(QDataStream& stream) const {
    stream << poly;
    stream << counter;
    stream << polylineFlag;
    stream << numberOfVertices;
    stream << startWidth;
    stream << endWidth;
    stream << constantWidth;
    stream << elevation;
    stream << thickness;
}

void LwPolyline::read(QDataStream& stream) {
    stream >> poly;
    stream >> counter;
    stream >> polylineFlag;
    stream >> numberOfVertices;
    stream >> startWidth;
    stream >> endWidth;
    stream >> constantWidth;
    stream >> elevation;
    stream >> thickness;
}
} // namespace Dxf
