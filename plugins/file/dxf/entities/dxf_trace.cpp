/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_trace.h"
#include "dxf_file.h"

namespace Dxf {

void Trace::parse(CodeData& code) {
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
    // Четвертая угловая точка не обязательна: если задано только три, она совпадает с третьей.
    if(!(corners & FourthCorner)) {
        fourthCorner = thirdCorner;
        corners |= FourthCorner;
    }
}

Entity::Type Trace::type() const { return Type::TRACE; }

DxfGo Trace::toGo() const {
    if(corners != 0xF)
        throw DxfObj::tr("Unsupported type Trace: corners %1!").arg(corners, 2);

    Geo::Polyline poly{
        {firstCorner},
        {secondCorner},
        {fourthCorner},
        {thirdCorner},
    };
    poly.close();

    if(!poly.isPositive())
        poly.reverse();

    DxfGo go{id, Geo::Polyline{poly}, {std::move(poly)}};
    go.type = DxfGo::Type(DxfGo::FlDrawn | DxfGo::FlStamp | DxfGo::Rect);
    return go;
}

void Trace::write(QDataStream& stream) const {
    stream << firstCorner;
    stream << secondCorner;
    stream << thirdCorner;
    stream << fourthCorner;

    stream << corners;

    stream << thickness;
}

void Trace::read(QDataStream& stream) {
    stream >> firstCorner;
    stream >> secondCorner;
    stream >> thirdCorner;
    stream >> fourthCorner;

    stream >> corners;

    stream >> thickness;
}

} // namespace Dxf
