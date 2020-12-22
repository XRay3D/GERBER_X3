#include "dxf_circle.h"
#include "dxf_insert.h"
#include "myclipper.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <section/blocks.h>
#include <section/entities.h>

namespace Dxf {
CIRCLE::CIRCLE(SectionParser* sp)
    : Entity(sp)
{
}

void CIRCLE::draw(const INSERT_ET* const i) const
{
    if (i) {
        for (int r = 0; r < i->rowCount; ++r) {
            for (int c = 0; c < i->colCount; ++c) {
                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
                auto cir(CirclePath(radius * 2 * uScale, centerPoint));
                cir.append(cir.first());
                GraphicObject go(sp->file, this, cir, {});
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        auto cir(CirclePath(radius * 2 * uScale, centerPoint));
        cir.append(cir.first());
        GraphicObject go(sp->file, this, cir, {});
        attachToLayer(std::move(go));
    }
}

void CIRCLE::parse(CodeData& code)
{
    do {
        data << code;
        switch (static_cast<VarType>(code.code())) {
        case SubclassMarker:
            break;
        case Thickness:
            thickness = code;
            break;
        case CenterPointX:
            centerPoint.rx() = code;
            break;
        case CenterPointY:
            centerPoint.ry() = code;
            break;
        case CenterPointZ:
            break;
        case Radius:
            radius = code;
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
