
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_lwpolyline.h"

#include <QPainterPath>
#include <qglobal.h>

// QPainterPath construct_path(const Polygon_2& pgn)
//{
//     QPainterPath result;
//     auto toQRectF = [](const CGAL::Bbox_2& bb) -> QRectF {
//         return QRectF(
//             bb.xmin(),
//             bb.ymin(),
//             bb.xmax() - bb.xmin(),
//             bb.ymax() - bb.ymin());
//     };
//     auto r = toQRectF(pgn.bbox());
//     if (r.width() * r.height() < 0.001)
//         return result;
//     if constexpr (0) {
//         auto current = pgn.curves_begin();
//         auto end = pgn.curves_end();
//         result.moveTo(QPointF(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y())));
//         do {
//             const auto& curve = *current;
//             const QPointF target = QPointF(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
//             if (curve.is_linear()) {
//                 result.lineTo(target);
//             } else if (curve.is_circular()) {
//                 const bool isClockwise = (curve.supporting_circle().orientation() == CGAL::CLOCKWISE);
//                 const QRectF rect = toQRectF(curve.supporting_circle().bbox());
//                 const QPointF center = QPointF(CGAL::to_double(curve.supporting_circle().center().x()), CGAL::to_double(curve.supporting_circle().center().y()));
//                 const QPointF source = QPointF(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
//                 const QPointF p1 = source - center;
//                 const QPointF p2 = target - center;
//                 const double asource = qAtan2(p1.y(), p1.x());
//                 const double atarget = qAtan2(p2.y(), p2.x());
//                 double span = atarget - asource;
//                 if (span < -CGAL_PI || (qFuzzyCompare(span, -CGAL_PI) && !isClockwise))
//                     span += 2.0 * CGAL_PI;
//                 else if (span > CGAL_PI || (qFuzzyCompare(span, CGAL_PI) && isClockwise))
//                     span -= 2.0 * CGAL_PI;
//                 result.arcTo(rect.normalized(), qRadiansToDegrees(-asource), qRadiansToDegrees(-span));
//             }
//         } while (++current != end);
//     } else {
//         Q_ASSERT(pgn.orientation() == CGAL::CLOCKWISE || pgn.orientation() == CGAL::COUNTERCLOCKWISE);
//         // Degenerate polygon, ring.size() < 3
//         if (pgn.orientation() == CGAL::ZERO) {
//             qWarning() << "construct_path: Ignoring degenerated polygon";
//             return result;
//         }
//         const bool isClockwise = pgn.orientation() == CGAL::CLOCKWISE;
//         auto current = pgn.curves_begin();
//         auto end = pgn.curves_end();
//         result.moveTo(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y()));
//         do {
//             const auto& curve = *current;
//             const auto& source = curve.source();
//             const auto& target = curve.target();
//             if (curve.is_linear()) {
//                 result.lineTo(QPointF(CGAL::to_double(target.x()), CGAL::to_double(target.y())));
//             } else if (curve.is_circular()) {
//                 const QRectF rect(toQRectF(curve.supporting_circle().bbox()));
//                 const auto center = curve.supporting_circle().center();
//                 const double asource = qAtan2(CGAL::to_double(source.y() - center.y()), CGAL::to_double(source.x() - center.x()));
//                 const double atarget = qAtan2(CGAL::to_double(target.y() - center.y()), CGAL::to_double(target.x() - center.x()));
//                 double span = atarget - asource;
//                 if (span < -CGAL_PI || (qFuzzyCompare(span, -CGAL_PI) && !isClockwise))
//                     span += 2.0 * CGAL_PI;
//                 else if (span > CGAL_PI || (qFuzzyCompare(span, CGAL_PI) && isClockwise))
//                     span -= 2.0 * CGAL_PI;
//                 result.arcTo(rect, qRadiansToDegrees(-asource), qRadiansToDegrees(-span));
//             } else { // ?!?
//                 Q_UNREACHABLE();
//             }
//         } while (++current != end);
//         result.lineTo(CGAL::to_double(pgn.curves_begin()->source().x()), CGAL::to_double(pgn.curves_begin()->source().y()));
//     }
//     return result;
// }

namespace Dxf {

LwPolyline::LwPolyline(SectionParser* sp)
    : Entity{sp} {
}

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
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
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
    } while(code.code() != 0);
}

Entity::Type LwPolyline::type() const { return Type::LWPOLYLINE; }

DxfGo LwPolyline::toGo() const {
    QPainterPath path;
    const bool dbg = false; // NOTE data.first().line() == 17844; /*|| data.first().line() == 18422;*/

    auto addSeg = [&path, dbg](const Segment& source, const Segment& target) {
        if(path.isEmpty())
            path.moveTo(source);

        if(qFuzzyIsNull(source.bulge)) {
            path.lineTo(target);
            return;
        }

        //        auto [center, start_angle_, end_angle_, radius] = bulgeToArc(source, target, source.bulge);

        const QLineF l1(source, target);
        const double lenght = l1.length() * 0.5;
        const double height = lenght * source.bulge;
        const double radius = (height * height + lenght * lenght) / (height * 2);

        QLineF l2((source + target) / 2, target);
        l2 = l2.normalVector();
        l2.setLength(height);
        QPointF c(l2.p2());

        QPointF center;
        {
            double ax2 = source.x() * source.x();
            double bx2 = target.x() * target.x();
            double cx2 = c.x() * c.x();
            double ay2 = source.y() * source.y();
            double by2 = target.y() * target.y();
            double cy2 = c.y() * c.y();
            double d = source.x() * target.y() + source.y() * c.x() - target.y() * c.x() - source.x() * c.y() - target.x() * source.y() + target.x() * c.y();
            center = QPointF(
                +0.5 / d * (                                                                                                //
                    source.y() * cx2 + source.y() * cy2 + target.y() * ax2 + target.y() * ay2 + c.y() * bx2 + c.y() * by2 - //
                    source.y() * bx2 - source.y() * by2 - target.y() * cx2 - target.y() * cy2 - c.y() * ax2 - c.y() * ay2),
                -0.5 / d * (                                                                                                //
                    source.x() * cx2 + source.x() * cy2 + target.x() * ax2 + target.x() * ay2 + c.x() * bx2 + c.x() * by2 - //
                    source.x() * bx2 - source.x() * by2 - target.x() * cx2 - target.x() * cy2 - c.x() * ax2 - c.x() * ay2));
        }

        //        const double start_angle = qRadiansToDegrees(start_angle_);
        //        const double end_angle = qRadiansToDegrees(end_angle_);
        double start_angle = qRadiansToDegrees(atan2(center.y() - source.y(), center.x() - source.x()));
        double end_angle = qRadiansToDegrees(atan2(center.y() - target.y(), center.x() - target.x()));

        if(end_angle <= start_angle)
            end_angle += 360;

        double span = end_angle - start_angle;

        const QPointF rad(radius, radius);
        const QRectF br(center + rad, center - rad);
        path.arcTo(br, -start_angle, -span);
    };

    for(size_t i = 0, size = poly.size() - 1; i < size; ++i)
        addSeg(poly[i], poly[i + 1]);
    if(polylineFlag == Closed)
        addSeg(poly.back(), poly.front());

    QTransform m;
    m.scale(u, u);
    QPainterPath path2;
    for(auto& poly: path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QTransform m2;
    m2.scale(d, d);
    auto p(path2.toSubpathPolygons(m2));

    if(p.empty())
        return {id, {}, {}}; // return {id, ~p.value(0), paths};

    // ClipperOffset offset;
    // offset.AddPath(Path{p.value(0)}, JT::Round, polylineFlag == Closed ? ET::Polygon : ET::Round);
    // Paths paths{offset.Execute(constantWidth * uScale * (poly.size() == 2 && polylineFlag == Closed ? 0.5 : 1.0))};
    const double offset = constantWidth * uScale * (poly.size() == 2 && polylineFlag == Closed ? 0.5 : 1.0);
    Paths paths = Inflate({~p.front()}, offset, JT::Round, polylineFlag == Closed ? ET::Polygon : ET::Round);
    DxfGo go{id, ~p.front(), paths}; // return {id, ~p.value(0), paths};

    if(polylineFlag == Closed
        && poly.size() == 2
        && qFuzzyCompare(poly.front().bulge, 1)
        && qFuzzyCompare(poly.back().bulge, 1)) {
        go.type = DxfGo::Type(DxfGo::FlStamp | DxfGo::Circle);
        go.GraphicObject::pos = ~QLineF{poly.front(), poly.back()}.center();
        go.path.clear();
    } else {
        go.type = DxfGo::Type(DxfGo::FlDrawn | DxfGo::PolyLine);
    }
    go.name = "id|LwPolyline";
    return go;
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
