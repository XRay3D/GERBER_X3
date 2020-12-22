#include "dxf_insert.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
//#include <dxf/dxf.h>

namespace Dxf {
INSERT_ET::INSERT_ET(QMap<QString, Block*>& blocks, SectionParser* sp)
    : Entity(sp)
    , blocks(blocks)
{
}

void INSERT_ET::draw(const INSERT_ET* const i) const
{
    if (!blocks.contains(blockName))
        return;

    if (blocks[blockName]->entities.isEmpty())
        return;

    for (auto e : blocks[blockName]->entities) {
        if (i) {
            INSERT_ET t(*this);
            if (layerName == "0")
                t.layerName = i->layerName;
            if (insPt.isNull())
                t.insPt = i->insPt;

            if (qFuzzyIsNull(rotationAngle))
                t.rotationAngle = i->rotationAngle;

            if (e->type() != INSERT) {
                if (t.layerName == "0")
                    t.layerName = e->layerName;
            }
            e->draw(&t);
        } else if (e->type() != INSERT) {
            INSERT_ET t(*this);
            if (t.layerName == "0")
                t.layerName = e->layerName;
            e->draw(&t);
        } else {
            e->draw(this);
        }
    }
}

void INSERT_ET::parse(CodeData& code)
{
    data << code;
    do {
        switch (code.code()) {
        case SubclassMrker:
            break;
        case var66:
            break;
        case BlockName:
            blockName = QString(code);
            break;
        case InsPtX:
            insPt.rx() = code;
            break;
        case InsPtY:
            insPt.ry() = code;
            break;
        case InsPtZ:
            break;
        case ScaleX:
            scaleX = code;
            break;
        case ScaleY:
            scaleY = code;
            break;
        case ScaleZ:
            break;
        case RotationAngle:
            rotationAngle = code;
            break;
        case ColCount:
            colCount = code;
            break;
        case RowCount:
            rowCount = code;
            break;
        case ColSpacing:
            colSpacing = code;
            break;
        case RowSpacing:
            rowSpacing = code;
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
        data << (code = sp->nextCode());
    } while (code.code() != 0);
}

void INSERT_ET::transform(GraphicObject& item, QPointF tr) const
{
    item.setRotation(rotationAngle);
    item.setScale(scaleX, scaleY);
    item.setPos(insPt + tr);
}
}
