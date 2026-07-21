/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_arc.h"
#include "dxf_insert.h"
#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "settings.h"
#include <QGraphicsEllipseItem>

#include <QPainter>

namespace Dxf {

void Arc::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker     : break;
        case Thickness          : thickness = code; break;
        case CenterPointX       : centerPoint.rx() = code; break;
        case CenterPointY       : centerPoint.ry() = code; break;
        case CenterPointZ       : break;
        case Radius             : radius = code; break;
        case StartAngle         : startAngle = code; break;
        case EndAngle           : endAngle = code; break;
        case ExtrusionDirectionX: break;
        case ExtrusionDirectionY: break;
        case ExtrusionDirectionZ: break;
        default                 : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Arc::type() const { return Type::ARC; }

DxfGo Arc::toGo() const {
    assert(thickness == 0); // TODO thickness

    QPointF
        p1{cos(qDegreesToRadians(startAngle)), sin(qDegreesToRadians(startAngle))},
        p2{cos(qDegreesToRadians(endAngle)), sin(qDegreesToRadians(endAngle))};
    // По спецификации DXF дуга ARC всегда идёт против часовой стрелки (Ccw) от
    // startAngle к endAngle, независимо от величины дуги. geo::DIR() определяет
    // направление по кратчайшему углу между точками и для дуг больше 180° даёт
    // неверный результат (Cw вместо Ccw) — направление здесь не вычисляется,
    // а фиксировано согласно спецификации.
    Curve curve{
        {p1 * radius + centerPoint},
        {p2 * radius + centerPoint, centerPoint, geo::Vertex::Ccw}
    };

    DxfGo go{id, std::move(curve)};
    return go;
    return {};
}

void Arc::write(QDataStream& stream) const {
    stream << centerPoint;
    stream << thickness;
    stream << radius;
    stream << startAngle;
    stream << endAngle;
}

void Arc::read(QDataStream& stream) {
    stream >> centerPoint;
    stream >> thickness;
    stream >> radius;
    stream >> startAngle;
    stream >> endAngle;
}

} // namespace Dxf
