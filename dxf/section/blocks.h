#pragma once

#include "block.h"
#include "sectionparser.h"

namespace Dxf {

class File;

struct SectionBLOCKS final : SectionParser {
    SectionBLOCKS(File* file, QVector<CodeData>&& data);
    virtual ~SectionBLOCKS() = default;
    // Section interface
    void parse() override;
    QMap<QString, Block*>& blocks;
};

}
