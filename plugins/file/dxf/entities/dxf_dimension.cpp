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
#include "dxf_dimension.h"
#include "dxf_file.h"

namespace Dxf {

void Dimension::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        case BlockName: blockName = code.string(); break;
        default       : Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

void Dimension::draw(const InsertEntity* const i) const {
    Blocks& blocks = sp->file->blocks();
    if(!blocks.contains(blockName))
        return;
    for(Entity* entity: blocks[blockName]->entities)
        entity->draw(i);
}

} // namespace Dxf
