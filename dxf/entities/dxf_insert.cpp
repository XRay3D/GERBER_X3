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
#include "dxf_insert.h"

namespace Dxf {

InsertEntity::InsertEntity(Blocks& blocks, SectionParser* sp)
    : Entity(sp)
    , blocks(blocks)
{
}

void InsertEntity::draw(const InsertEntity* const i) const
{
    if (!blocks.contains(blockName))
        return;
    if (blocks[blockName]->entities.isEmpty())
        return;

    basePoint = blocks[blockName]->basePoint;

    for (auto e : blocks[blockName]->entities) {

        if (i) {
            InsertEntity t(*this);
            if (layerName == "0")
                t.layerName = i->layerName;
            if (insPos.isNull())
                t.insPos = i->insPos;

            if (qFuzzyIsNull(rotationAngle))
                t.rotationAngle = i->rotationAngle;

            if (e->type() != INSERT) {
                if (t.layerName == "0")
                    t.layerName = e->layerName;
            }
            e->draw(&t);
        } else if (e->type() != INSERT) {
            InsertEntity t(*this);
            if (t.layerName == "0")
                t.layerName = e->layerName;
            e->draw(&t);
        } else {
            e->draw(this);
        }
    }
}

void InsertEntity::parse(CodeData& code)
{
    do {
        switch (code.code()) {
        case SubclassMrker:
        case VariableAttributes:
            break;
        case BlockName:
            blockName = code.string();
            break;
        case InsPtX:
            insPos.rx() = code;
            break;
        case InsPtY:
            insPos.ry() = code;
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
        case ExtrusionDirectionY:
        case ExtrusionDirectionZ:
            break;
        default:
            parseEntity(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

void InsertEntity::transform(GraphicObject& item, QPointF tr) const
{
    item.setPos(-basePoint);
    item.setScale(scaleX, scaleY);
    item.setRotation(rotationAngle);
    item.setPos(insPos + tr);
}

}
