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
#include "dxf_solid.h"
#include "dxf_file.h"
#include <QGraphicsPolygonItem>

#include <QPolygonF>

namespace Dxf {

void Solid::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker     : break;
        case Thickness          : thickness = code; break;
        case FirstCornerX       : firstCorner.rx() = code, corners |= FirstCorner; break;
        case FirstCornerY       : firstCorner.ry() = code; break;
        case FirstCornerZ       : break;
        case SecondCornerX      : secondCorner.rx() = code, corners |= SecondCorner; break;
        case SecondCornerY      : secondCorner.ry() = code; break;
        case SecondCornerZ      : break;
        case ThirdCornerX       : thirdCorner.rx() = code, corners |= ThirdCorner; break;
        case ThirdCornerY       : thirdCorner.ry() = code; break;
        case ThirdCornerZ       : break;
        case FourthCornerX      : fourthCorner.rx() = code, corners |= FourthCorner; break;
        case FourthCornerY      : fourthCorner.ry() = code; break;
        case FourthCornerZ      : break;
        case ExtrusionDirectionX: break;
        case ExtrusionDirectionY: break;
        case ExtrusionDirectionZ: break;
        default                 : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Solid::type() const { return Type::SOLID; }

DxfGo Solid::toGo() const {
    qInfo("Solid");

    if(corners != 0xF)
        throw DxfObj::tr("Unsupported type Solid: corners %1!").arg(corners, 2);

    Curve poly{
        {firstCorner},
        {secondCorner},
        {fourthCorner},
        {thirdCorner},
    };
    poly.close();

    // Path path{~poly};
    // r::for_each(path, SetCSelf);
    // ReverseCurves(poly);
    if(!poly.isPositive())
        poly.reverse();

    DxfGo go{id, Curve{poly}, {std::move(poly)}}; // return {id, path, {path}};
    go.type = DxfGo::Type(DxfGo::FlDrawn | DxfGo::FlStamp | DxfGo::Rect);
    return go;
}

void Solid::write(QDataStream& stream) const {
    stream << firstCorner;
    stream << secondCorner;
    stream << thirdCorner;
    stream << fourthCorner;

    stream << corners;

    stream << thickness;
    stream << radius;
}

void Solid::read(QDataStream& stream) {
    stream >> firstCorner;
    stream >> secondCorner;
    stream >> thirdCorner;
    stream >> fourthCorner;

    stream >> corners;

    stream >> thickness;
    stream >> radius;
}

} // namespace Dxf
