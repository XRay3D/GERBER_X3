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

    // По спецификации DXF дуга ARC всегда идёт против часовой стрелки (Ccw) от
    // startAngle к endAngle, независимо от величины дуги -- размах здесь не
    // вычисляется по кратчайшему углу, а берётся согласно спецификации.
    const double a1 = qDegreesToRadians(startAngle);
    double sweep = qDegreesToRadians(endAngle) - a1;
    while(sweep <= 0.0) sweep += 2.0 * pi;

    // Geo::arc режет размах на куски не длиннее полуокружности: прогиб
    // почти полного оборота уходит в бесконечность.
    Geo::Polyline curve = Geo::arc(centerPoint, radius, a1, sweep);

    DxfGo go{id, std::move(curve)};
    return go;
    return {};
}

} // namespace Dxf
