/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_circle.h"
#include "dxf_insert.h"
#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include <QGraphicsEllipseItem>
#include <gi_dbg.h>
#include <myclipper.h>

namespace Dxf {

void Circle::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker     : break;
        case Thickness          : thickness = code; break;
        case CenterPointX       : centerPoint.rx() = code; break;
        case CenterPointY       : centerPoint.ry() = code; break;
        case CenterPointZ       : break;
        case Radius             : radius = code; break;
        case ExtrusionDirectionX: break;
        case ExtrusionDirectionY: break;
        case ExtrusionDirectionZ: break;
        default                 : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Circle::type() const { return Type::CIRCLE; }

DxfGo Circle::toGo() const {
    qInfo("Circle");
#if 0
    QPainterPath path;
    QPointF r{radius, radius};
    path.addEllipse(QRectF(centerPoint + r, centerPoint - r));

    QTransform m;
    m.scale(u, u);
    QPainterPath path2;
    for(auto& poly: path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QTransform m2;
    m2.scale(d, d);
    auto p(path2.toSubpathPolygons(m2));

    DxfGo go{id, ~p.value(0), {}}; // return {id, ~p.value(0), {}};

    go.raw = radius * 2;
    go.name = layerName; // u"T%1|Ø%2"_s.arg(hole.state.toolId).arg(tools_.at(hole.state.toolId)).toUtf8(); // name;
    go.fill.emplace_back(~p.value(0));
    go.path.clear();

    // new Gi::Debug{path, Qt::green};

    go.type = DxfGo::Type(DxfGo::FlStamp | DxfGo::Circle);
    go.GraphicObject::pos = ~centerPoint;

    return go;
#else
    Path path = CirclePath(radius * 2 + thickness, ~centerPoint);
    r::for_each(path, std::bind(SetC, _1, ~centerPoint));

    DxfGo go{id, path, {path}};
    go.name = layerName; // u"T%1|Ø%2"_s.arg(hole.state.toolId).arg(tools_.at(hole.state.toolId)).toUtf8(); // name;
    go.type = DxfGo::Type(DxfGo::FlStamp | DxfGo::FlDrawn | DxfGo::Circle);
    return /*go*/ {};
#endif
}

void Circle::write(QDataStream& stream) const {
    stream << centerPoint;
    stream << thickness;
    stream << radius;
}

void Circle::read(QDataStream& stream) {
    stream >> centerPoint;
    stream >> thickness;
    stream >> radius;
}

} // namespace Dxf
