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
#include "dxf_line.h"

#include <QPainterPath>

namespace Dxf {

Line::Line(SectionParser* sp)
    : Entity{sp} {
}

void Line::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker     : break;
        case Thickness          : thickness = code; break;
        case StartPointX        : startPoint.rx() = code; break;
        case StartPointY        : startPoint.ry() = code; break;
        case StartPointZ        : break;
        case EndPointX          : endPoint.rx() = code; break;
        case EndPointY          : endPoint.ry() = code; break;
        case EndPointZ          : break;
        case ExtrusionDirectionX: break;
        case ExtrusionDirectionY: break;
        case ExtrusionDirectionZ: break;
        default                 : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Line::type() const { return Type::LINE; }

DxfGo Line::toGo() const {
    qInfo("Line");
    qInfo() << thickness;
    Path path{~startPoint, ~endPoint};
    r::for_each(path, SetCSelf);

    // TODO ClipperOffset offset;
    // offset.AddPath(p, JoinType::Round, EndType::Round);
    // paths = offset.Execute(thickness * uScale);

    DxfGo go{id, path, {}}; // FIXME return {id, p, paths};
    go.type = DxfGo::Type(DxfGo::FlDrawn | DxfGo::Line);
    return go;
}

void Line::write(QDataStream& stream) const {
    stream << startPoint;
    stream << endPoint;
    stream << thickness;
}

void Line::read(QDataStream& stream) {
    stream >> startPoint;
    stream >> endPoint;
    stream >> thickness;
}

} // namespace Dxf
