// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_solid.h"
#include "dxf_file.h"
#include <QGraphicsPolygonItem>

#include <QPolygonF>

namespace Dxf {

Solid::Solid(SectionParser* sp)
    : Entity(sp)
{
}

void Solid::draw(const InsertEntity* const i) const
{
    if (i) {
        for (int r = 0; r < i->rowCount; ++r) {
            for (int c = 0; c < i->colCount; ++c) {
                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
                GraphicObject go(toGo());
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        attachToLayer(toGo());
    }
}

void Solid::parse(CodeData& code)
{
    do {
        data.push_back(code);
        switch (static_cast<VarType>(code.code())) {
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
            parseEntity(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

GraphicObject Solid::toGo() const
{
    QPolygonF poly;
    if (corners == 15) {
        poly.reserve(5);
        poly << firstCorner;
        poly << secondCorner;
        poly << fourthCorner;
        poly << thirdCorner;
        poly << firstCorner;
    } else {
        throw QString("Unsupported type Solid: corners %1!").arg(corners);
    }
    Path path(poly);
    ReversePath(path);
    return { sp->file, this, path, {} };
}

}
