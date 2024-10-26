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
#include "dxf_insert.h"

#include "dxf_block.h"
#include "dxf_types.h"

namespace Dxf {

InsertEntity::InsertEntity(Blocks& blocks, SectionParser* sp)
    : Entity{sp}
    , blocks(blocks) {
}

void InsertEntity::draw(const InsertEntity* const i) const {
    if(!blocks.contains(blockName))
        return;
    if(blocks[blockName]->entities.empty())
        return;

    basePoint = blocks[blockName]->basePoint;

    for(auto entity: blocks[blockName]->entities) {
        if(i) {
            InsertEntity copy(*this);
            if(layerName == "0")
                copy.layerName = i->layerName;
            if(insPos.isNull())
                copy.insPos = i->insPos;

            if(qFuzzyIsNull(rotationAngle))
                copy.rotationAngle = i->rotationAngle;

            if(entity->type() != INSERT) {
                if(copy.layerName == "0")
                    copy.layerName = entity->layerName;
            }
            entity->draw(&copy);
        } else if(entity->type() != INSERT) {
            InsertEntity copy(*this);
            if(copy.layerName == "0")
                copy.layerName = entity->layerName;
            entity->draw(&copy);
        } else {
            entity->draw(this);
        }
    }
}

void InsertEntity::parse(CodeData& code) {
    do {
        switch(code.code()) {
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
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

void InsertEntity::transform(DxfGo& item, QPointF tr) const {
    item.setPos(-basePoint);
    item.setScale(scaleX, scaleY);
    item.setRotation(rotationAngle);
    item.setPos(insPos + tr);
}

} // namespace Dxf
