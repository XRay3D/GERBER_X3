#include "dxf_solid.h"
#include "dxffile.h"
#include <QGraphicsPolygonItem>
#include <QGraphicsScene>
#include <QPolygonF>

namespace Dxf {

SOLID::SOLID(SectionParser* sp)
    : Entity(sp)
{
}

void SOLID::draw(const INSERT_ET* const i) const
{
    //    QPolygonF poly;
    //    qDebug() << "corners" << corners;
    //    if (corners == 15) {
    //        poly.reserve(5);
    //        poly << firstCorner;
    //        poly << secondCorner;
    //        poly << fourthCorner;
    //        poly << thirdCorner;
    //    } else {
    //        exit(-100);
    //    }
    //    if (i) {
    //        for (int r = 0; r < i->rowCount; ++r) {
    //            for (int c = 0; c < i->colCount; ++c) {
    //                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
    //                auto item = scene->addPolygon(poly, QPen(i->color(), thickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), i->color());
    //                item->setToolTip(layerName);
    //                i->transform(item, tr);
    //                i->attachToLayer(item);
    //            }
    //        }
    //    } else {
    //        attachToLayer(scene->addPolygon(poly, QPen(color(), thickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin), color()));
    //    }
}

void SOLID::parse(CodeData& code)
{
    do {
        data << code;
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

}
