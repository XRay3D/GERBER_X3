// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "dxf_polyline.h"
#include "dxf_file.h"
#include "dxf_insert.h"

namespace Dxf {

PolyLine::PolyLine(SectionParser* sp)
    : Entity(sp) {
}

// void PolyLine::draw(const InsertEntity* const i) const
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

void PolyLine::parse(CodeData& code) {
    do {
        data.push_back(code);
        if (code != "VERTEX") {
            code = sp->nextCode();
            switch (code.code()) {
            case StartWidth:
                startWidth = code;
                break;
            case EndWidth:
                endWidth = code;
                break;
            case PolylineFlag:
                polylineFlags = code;
                break;
            default:
                Entity::parse(code);
            }
        } else {
            vertex.append(Dxf::Vertex(sp));
            vertex.last().parse(code);
        }
    } while (code != "SEQEND");
    do {
        code = sp->nextCode();
        Entity::parse(code);
    } while (code.code() != 0);
    //    qDebug() << data.size();
    //    qDebug() << data;
}

Entity::Type PolyLine::type() const { return Type::POLYLINE; }

GraphicObject PolyLine::toGo() const {
    QPainterPath path;
    auto addSeg = [&path](const Vertex& source, const Vertex& target) {
        if (path.isEmpty())
            path.moveTo(source);

        if (source.bulge == 0.0) {
            path.lineTo(target);
            return;
        }

        auto [center, start_angle_, end_angle_, radius] = bulgeToArc(source, target, source.bulge);

        const double start_angle = qRadiansToDegrees(qAtan2(center.y() - source.y, center.x() - source.x));
        const double end_angle = qRadiansToDegrees(qAtan2(center.y() - target.y, center.x() - target.x));
        double span = end_angle - start_angle;
        if /**/ (span < -180 || (qFuzzyCompare(span, -180) && !(end_angle > start_angle)))
            span += 360;
        else if (span > 180 || (qFuzzyCompare(span, 180) && (end_angle > start_angle)))
            span -= 360;

        QPointF pr(radius, radius);
        path.arcTo(QRectF(center + pr, center - pr),
            -start_angle,
            -span);
    };

    for (int i = 0; i < vertex.size() - 1; ++i) {
        addSeg(vertex[i], vertex[i + 1]);
    }
    if (polylineFlags & ClosedPolyline) {
        addSeg(vertex.last(), vertex.first());
    }

    QTransform m;
    m.scale(u, u);
    QPainterPath path2;
    for (auto& poly : path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QTransform m2;
    m2.scale(d, d);
    auto p(path2.toSubpathPolygons(m2));

    return {id, p.value(0), {}};
}

void PolyLine::write(QDataStream& stream) const {
    stream << polylineFlags;
    stream << startWidth;
    stream << endWidth;
}

void PolyLine::read(QDataStream& stream) {
    stream >> polylineFlags;
    stream >> startWidth;
    stream >> endWidth;
}

} // namespace Dxf
