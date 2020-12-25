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
    void entityParse(CodeData& code);

    Entity::Type key;
    SectionParser* sp;
    Blocks& blocks;
};
}
