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
#include "dxf_ellipse.h"

#include "dxf_insert.h"
#include <QGraphicsEllipseItem>

#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"

#include <QPainter>
#include <qmath.h>

namespace Dxf {
Ellipse::Ellipse(SectionParser* sp)
    : Entity{sp} {
}

void Ellipse::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker             : break; // 100 Маркер подкласса (AcDbEllipse)

        case CenterPointX               : CenterPoint.setX(code); break; // 10 Центральная точка (в МСК)
        case CenterPointY               : CenterPoint.setY(code); break; // 20 // Файл DXF: значение X; приложение: 3D-точка
        case CenterPointZ               : break;                         // 30 // 20, 30 Файл DXF: значения Y и Z для центральной точки (в МСК)

        case EndpointOfMajorAxisX       : EndpointOfMajorAxis.setX(code); break; // 11 Конечная точка главной оси относительно центральной точки (в МСК)
        case EndpointOfMajorAxisY       : EndpointOfMajorAxis.setY(code); break; // 21 // Файл DXF: значение X; приложение: 3D-точка
        case EndpointOfMajorAxisZ       : break;                                 // 31 // 21, 31 Файл DXF: значения Y и Z для конечной точки главной оси относительно центральной точки (в МСК)

        case ExtrusionDirectionX        :        // 210
        case ExtrusionDirectionY        :        // 220
        case ExtrusionDirectionZ        : break; // 230 //

        case RatioOfMinorAxisToMajorAxis: ratioOfMinorAxisToMajorAxis = code; break; // 40 Соотношение малой и главной осей

        case StartParameter             : startParameter = code; break; // 41 Начальный параметр (значение для полного эллипса — 0,0)
        case EndParameter               : endParameter = code; break;   // 42 Конечный параметр (значение для полного эллипса — 2 пи)
        default                         : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Ellipse::type() const { return Entity::ELLIPSE; }

DxfGo Ellipse::toGo() const {
    qInfo("Ellipse");
    qInfo("TODO");
    return {};
}

void Ellipse::write(QDataStream& stream) const {
    stream << startParameter;
    stream << endParameter;
    stream << ratioOfMinorAxisToMajorAxis;
    stream << CenterPoint;
    stream << EndpointOfMajorAxis;
}

void Ellipse::read(QDataStream& stream) {
    stream >> startParameter;
    stream >> endParameter;
    stream >> ratioOfMinorAxisToMajorAxis;
    stream >> CenterPoint;
    stream >> EndpointOfMajorAxis;
}

} // namespace Dxf
