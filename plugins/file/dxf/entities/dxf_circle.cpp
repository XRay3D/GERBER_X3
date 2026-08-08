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
    Geo::Polyline path = CircleCurve(radius * 2 + thickness, centerPoint);
    assert(thickness == 0);
    DxfGo go{id, Geo::Polyline{path}, {std::move(path)}};
    go.raw = radius * 2;
    go.name = layerName; // u"T%1|Ø%2"_s.arg(hole.state.toolId).arg(tools_.at(hole.state.toolId)).toUtf8(); // name;
    go.type = DxfGo::Type(DxfGo::FlStamp /*| DxfGo::FlDrawn*/ | DxfGo::Circle);
    go.GraphicObject::pos = centerPoint;
    return go;
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
