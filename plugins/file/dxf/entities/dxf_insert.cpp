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
            if(layerName == u"0"_s)
                copy.layerName = i->layerName;
            if(insPos.isNull())
                copy.insPos = i->insPos;

            if(qFuzzyIsNull(rotationAngle))
                copy.rotationAngle = i->rotationAngle;

            if(entity->type() != INSERT) {
                if(copy.layerName == u"0"_s)
                    copy.layerName = entity->layerName;
            }
            entity->draw(&copy);
        } else if(entity->type() != INSERT) {
            InsertEntity copy(*this);
            if(copy.layerName == u"0"_s)
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
        case VariableAttributes : break;
        case BlockName          : blockName = code.string(); break;
        case InsPtX             : insPos.rx() = code; break;
        case InsPtY             : insPos.ry() = code; break;
        case InsPtZ             : break;
        case ScaleX             : scaleX = code; break;
        case ScaleY             : scaleY = code; break;
        case ScaleZ             : break;
        case RotationAngle      : rotationAngle = code; break;
        case ColCount           : colCount = code; break;
        case RowCount           : rowCount = code; break;
        case ColSpacing         : colSpacing = code; break;
        case RowSpacing         : rowSpacing = code; break;
        case ExtrusionDirectionX:
        case ExtrusionDirectionY:
        case ExtrusionDirectionZ: break;
        default                 : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

void InsertEntity::transform(DxfGo& item, QPointF tr) const {
    tr = insPos + tr;
    item.setPos(-basePoint);
    item.setScale(scaleX, scaleY);
    item.setRotation(rotationAngle);
    item.setPos(tr);

    QTransform transform;
    transform.translate(tr.x(), tr.y());
    transform.scale(scaleX, scaleY);
    transform.rotate(rotationAngle);
    transform.translate(-basePoint.x(), -basePoint.y());

    TransformPath(item.path, transform);
    TransformPaths(item.fill, transform);
}

} // namespace Dxf
