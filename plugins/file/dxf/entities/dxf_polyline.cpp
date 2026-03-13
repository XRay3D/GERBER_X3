/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_polyline.h"
#include "dxf_file.h"
#include "dxf_insert.h"

#include <format>
#include <gi_dbg.h>

namespace Dxf {

PolyLine::PolyLine(SectionParser* sp)
    : Entity{sp} {
}

// void PolyLine::draw(const InsertEntity* const i) const
//{
// if (i) {
// for (int r{}; r < i->rowCount; ++r) {
// for (int c{}; c < i->colCount; ++c) {
// QPointF tr{r * i->rowSpacing, r * i->colSpacing};
// GraphicObject go(toGo());
// i->transform(go, tr);
// i->attachToLayer(std::move(go));
// }
// }
// } else {
// attachToLayer(toGo());
// }
// }

void PolyLine::parse(CodeData& code) {
    do {
        data.push_back(code);
        if(code != u"VERTEX"_s) {
            code = sp->nextCode();
            switch(code.code()) {
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
            polyLine.emplace_back(sp).parse(code);
        }
    } while(code != u"SEQEND"_s);
    do {
        code = sp->nextCode();
        Entity::parse(code);
    } while(code.code() != 0);
}

Entity::Type PolyLine::type() const { return Type::POLYLINE; }

DxfGo PolyLine::toGo() const {
    QPainterPath path;
    auto addSeg = [&path](const Vertex& source, const Vertex& target) mutable {
        addArcTo(path, source, target, source.bulge);
    };

    for(auto&& [from, to]: polyLine | v::pairwise) addSeg(from, to);

    if(polylineFlags & ClosedPolyline) addSeg(polyLine.back(), polyLine.front());

    QTransform m;
    m.scale(u, u);
    QPainterPath path2;

    // new Gi::Debug{path, Qt::yellow};

    for(auto& poly: path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QTransform m2;
    m2.scale(d, d);
    auto p(path2.toSubpathPolygons(m2));

    if(polylineFlags & ClosedPolyline) // FIXME
        return DxfGo{id, ~p.value(0), ~p};
    else
        return DxfGo{id, ~p.value(0), ~p};
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
