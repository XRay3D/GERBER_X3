// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_point.h"

namespace Dxf {

Point::Point(SectionParser* sp)
    : Entity(sp) {
}

// void Point::draw(const Dxf::InsertEntity* const i) const
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

void Point::parse(Dxf::CodeData& code) {
    do {
        data.push_back(code);
        switch (static_cast<DataEnum>(code.code())) {
        case SubclassMarker:
            break;
        case Thickness:
            thickness = code;
            break;
        case PointX:
            point.rx() = code;
            break;
        case PointY:
            point.ry() = code;
            break;
        case PointZ:
            break;
        case ExtrusionDirectionX:
        case ExtrusionDirectionY:
        case ExtrusionDirectionZ:
            break;
        case AngleOfTheXZxisForTheUCS:
            break;
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

Entity::Type Point::type() const { return POINT; }

GraphicObject Point::toGo() const {
    QPolygonF p;
    p.append(point);

    return {id, p, {}};
}

void Point::write(QDataStream& stream) const {
    stream << point;
    stream << thickness;
}

void Point::read(QDataStream& stream) {
    stream >> point;
    stream >> thickness;
}

} // namespace Dxf
