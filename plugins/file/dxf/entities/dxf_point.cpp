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
#include "dxf_point.h"

namespace Dxf {

void Point::parse(Dxf::CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker          : break;
        case Thickness               : thickness = code; break;
        case PointX                  : point.rx() = code; break;
        case PointY                  : point.ry() = code; break;
        case PointZ                  : break;
        case ExtrusionDirectionX     :
        case ExtrusionDirectionY     :
        case ExtrusionDirectionZ     : break;
        case AngleOfTheXZxisForTheUCS: break;
        default                      : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Point::type() const { return POINT; }

DxfGo Point::toGo() const {
    qInfo("Point");
    // QPolygonF p;
    // p.append(point);

    DxfGo go{id, {{point}}, {}}; // return {id, p, {}};
    return go;
}

} // namespace Dxf
