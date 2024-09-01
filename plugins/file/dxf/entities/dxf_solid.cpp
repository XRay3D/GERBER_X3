// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_solid.h"
#include "dxf_file.h"
#include <QGraphicsPolygonItem>

#include <QPolygonF>

namespace Dxf {

Solid::Solid(SectionParser* sp)
    : Entity{sp} {
}

// void Solid::draw(const InsertEntity* const i) const
//{
//     if (i) {
//         for (int r = 0; r < i->rowCount; ++r) {
//             for (int c = 0; c < i->colCount; ++c) {
//                 QPointF tr(r * i->rowSpacing, r * i->colSpacing);
//                 GraphicObject go(toGo());
//                 i->transform(go, tr);
//                 i->attachToLayer(std::move(go));
//             }
//         }
//     } else {
//         attachToLayer(toGo());
//     }
// }

void Solid::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker:
            break;
        case Thickness:
            thickness = code;
            break;
        case FirstCornerX:
            firstCorner.rx() = code;
            corners |= FirstCorner;
            break;
        case FirstCornerY:
            firstCorner.ry() = code;
            break;
        case FirstCornerZ:
            break;
        case SecondCornerX:
            secondCorner.rx() = code;
            corners |= SecondCorner;
            break;
        case SecondCornerY:
            secondCorner.ry() = code;
            break;
        case SecondCornerZ:
            break;
        case ThirdCornerX:
            thirdCorner.rx() = code;
            corners |= ThirdCorner;
            break;
        case ThirdCornerY:
            thirdCorner.ry() = code;
            break;
        case ThirdCornerZ:
            break;
        case FourthCornerX:
            fourthCorner.rx() = code;
            corners |= FourthCorner;
            break;
        case FourthCornerY:
            fourthCorner.ry() = code;
            break;
        case FourthCornerZ:
            break;
        case ExtrusionDirectionX:
            break;
        case ExtrusionDirectionY:
            break;
        case ExtrusionDirectionZ:
            break;
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Solid::type() const { return Type::SOLID; }

DxfGo Solid::toGo() const {
    QPolygonF poly;
    if(corners == 15) {
        poly.reserve(5);
        poly << firstCorner;
        poly << secondCorner;
        poly << fourthCorner;
        poly << thirdCorner;
        poly << firstCorner;
    } else {
        throw DxfObj::tr("Unsupported type Solid: corners %1!").arg(corners);
    }
    Path path{~poly};
    ReversePath(path);
    DxfGo go{id, path, {path}}; // return {id, path, {path}};
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
