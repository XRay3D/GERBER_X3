
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_lwpolyline.h"
#include "gi_dbg.h"

// #include <QPainterPath>

// QPainterPath construct_path(const Polygon_2& pgn)
//{
// QPainterPath result;
// auto toQRectF = [](const CGAL::Bbox_2& bb) -> QRectF {
// return QRectF(
// bb.xmin(),
// bb.ymin(),
// bb.xmax() - bb.xmin(),
// bb.ymax() - bb.ymin());
// };
// auto r = toQRectF(pgn.bbox());
// if (r.width() * r.height() < 0.001)
// return result;
// if constexpr (0) {
// auto current = pgn.curves_begin();
// auto end = pgn.curves_end();
// result.moveTo(QPointF(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y())));
// do {
// const auto& curve = *current;
// const QPointF target = QPointF(CGAL::to_double(curve.target().x()), CGAL::to_double(curve.target().y()));
// if (curve.is_linear()) {
// result.lineTo(target);
// } else if (curve.is_circular()) {
// const bool isClockwise = (curve.supporting_circle().orientation() == CGAL::CLOCKWISE);
// const QRectF rect = toQRectF(curve.supporting_circle().bbox());
// const QPointF center = QPointF(CGAL::to_double(curve.supporting_circle().center().x()), CGAL::to_double(curve.supporting_circle().center().y()));
// const QPointF source = QPointF(CGAL::to_double(curve.source().x()), CGAL::to_double(curve.source().y()));
// const QPointF p1 = source - center;
// const QPointF p2 = target - center;
// const double asource = qAtan2(p1.y(), p1.x());
// const double atarget = qAtan2(p2.y(), p2.x());
// double span = atarget - asource;
// if (span < -CGAL_PI || (qFuzzyCompare(span, -CGAL_PI) && !isClockwise))
// span += 2.0 * CGAL_PI;
// else if (span > CGAL_PI || (qFuzzyCompare(span, CGAL_PI) && isClockwise))
// span -= 2.0 * CGAL_PI;
// result.arcTo(rect.normalized(), qRadiansToDegrees(-asource), qRadiansToDegrees(-span));
// }
// } while (++current != end);
// } else {
// Q_ASSERT(pgn.orientation() == CGAL::CLOCKWISE || pgn.orientation() == CGAL::COUNTERCLOCKWISE);
// // Degenerate polygon, ring.size() < 3
// if (pgn.orientation() == CGAL::ZERO) {
// qWarning() << u"construct_path: Ignoring degenerated polygon"_s;
// return result;
// }
// const bool isClockwise = pgn.orientation() == CGAL::CLOCKWISE;
// auto current = pgn.curves_begin();
// auto end = pgn.curves_end();
// result.moveTo(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y()));
// do {
// const auto& curve = *current;
// const auto& source = curve.source();
// const auto& target = curve.target();
// if (curve.is_linear()) {
// result.lineTo(QPointF(CGAL::to_double(target.x()), CGAL::to_double(target.y())));
// } else if (curve.is_circular()) {
// const QRectF rect(toQRectF(curve.supporting_circle().bbox()));
// const auto center = curve.supporting_circle().center();
// const double asource = qAtan2(CGAL::to_double(source.y() - center.y()), CGAL::to_double(source.x() - center.x()));
// const double atarget = qAtan2(CGAL::to_double(target.y() - center.y()), CGAL::to_double(target.x() - center.x()));
// double span = atarget - asource;
// if (span < -CGAL_PI || (qFuzzyCompare(span, -CGAL_PI) && !isClockwise))
// span += 2.0 * CGAL_PI;
// else if (span > CGAL_PI || (qFuzzyCompare(span, CGAL_PI) && isClockwise))
// span -= 2.0 * CGAL_PI;
// result.arcTo(rect, qRadiansToDegrees(-asource), qRadiansToDegrees(-span));
// } else { // ?!?
// Q_UNREACHABLE();
// }
// } while (++current != end);
// result.lineTo(CGAL::to_double(pgn.curves_begin()->source().x()), CGAL::to_double(pgn.curves_begin()->source().y()));
// }
// return result;
// }

namespace Dxf {

void LwPolyline::parse(CodeData& code) {
    Segment pt;
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMrker      : break;
        case NumberOfVertices   : numberOfVertices = code; break;
        case PolylineFlag       : polylineFlag = code; break;
        case ConstantWidth      : constantWidth = code; break;
        case Elevation          : elevation = code; break;
        case Thickness          : thickness = code; break;
        case VertexCoordinatesX : poly.emplace_back().rx() = code; break;
        case VertexCoordinatesY : poly.back().ry() = code; break;
        case VertexID           : break;
        case StartWidth         : startWidth = code; break;
        case EndWidth           : endWidth = code; break;
        case Bulge              : poly.back().bulge = code; break; // betven points
        case ExtrusionDirectionX: break;
        case ExtrusionDirectionY: break;
        case ExtrusionDirectionZ: break;
        default                 : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type LwPolyline::type() const { return Type::LWPOLYLINE; }

DxfGo LwPolyline::toGo() const {
    if(startWidth || endWidth)
        qWarning("TODO startWidth | endWidth");

    if(polylineFlag == Closed
        && poly.size() == 2
        && qFuzzyCompare(poly.front().bulge, 1.)
        && qFuzzyCompare(poly.back().bulge, 1.)) {
        QPointF center = (poly.front() + poly.back()) / 2;
        Curve circle = CircleCurve((geo::Length(poly.front(), poly.back()) + constantWidth), center);

        // if(auto arc = geo::arcOf(poly.front(), poly.back(), poly.front().bulge); arc && arc->theta < 0) circle.reverse();

        DxfGo go{
            id,
            Curve{circle},
            {std::move(circle)},
        };
        go.GraphicObject::pos = center;
        go.type = DxfGo::Type{DxfGo::FlStamp | DxfGo::Circle};
        go.name = u"id|LwPolyline/Circle"_s;
        return go;
    }

    // Прогиб LWPOLYLINE и прогиб Curve -- одно и то же и лежат на одной и той
    // же (начальной) вершине сегмента, так что переводить нечего: вершины
    // переносятся как есть.
    Curve curve{
        std::from_range,
        poly | v::transform([](const Segment& v) { return geo::Vertex{v, v.bulge}; }),
    };

    if(polylineFlag == Closed) curve.close();

    Curves curves = Inflate({curve}, // TODO
        constantWidth,
        JoinType::Round,
        EndType::Round, 2.0, 1000);

    DxfGo go{id, std::move(curve), std::move(curves)}; // return {id, ~p.value(0), paths};

    go.type = DxfGo::Type(DxfGo::FlDrawn | DxfGo::PolyLine);

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
