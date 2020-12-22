#pragma once

#include "block.h"
#include "codedata.h"
#include "entities/dxf_entity.h"
#include "sectionparser.h"

namespace Dxf {

class File;

struct SectionENTITIES final : SectionParser {
    explicit SectionENTITIES(File* file, QVector<CodeData>&& data);
    SectionENTITIES(QMap<QString, Block*>& blocks, CodeData& code, SectionParser* sp = nullptr);

    virtual ~SectionENTITIES() { qDeleteAll(entities); }
    // Section interface
    void parse() override;

    QVector<Entity*> entities;
    QMap<Entity::Type, QVector<Entity*>> entitiesMap;

private:
    void iParse(CodeData& code);
    CodeData code;
    Entity::Type key;
    SectionParser* sp;
    QMap<QString, Block*>& blocks;
};
}
