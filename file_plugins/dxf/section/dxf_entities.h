/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "dxf_block.h"
#include "dxf_codedata.h"
#include "dxf_sectionparser.h"
#include "entities/dxf_entity.h"

namespace Dxf {

class File;
struct Entity;

struct SectionENTITIES final : SectionParser {
    explicit SectionENTITIES(File* file, Codes::iterator from, Codes::iterator to);
    SectionENTITIES(Blocks& blocks, CodeData& code, SectionParser* sp);

    virtual ~SectionENTITIES();
    // Section interface
    void parse() override;

    QVector<Entity*> entities;
    QMap<Entity::Type, QVector<Entity*>> entitiesMap;

private:
    Entity *entityParse(CodeData& code);

    Entity::Type key;
    SectionParser* sp;
    Blocks& blocks;
};
}
