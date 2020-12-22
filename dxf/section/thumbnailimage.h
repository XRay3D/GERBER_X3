#pragma once

#include "sectionparser.h"

namespace Dxf {

class File;

struct SectionTHUMBNAILIMAGE final : SectionParser {
    SectionTHUMBNAILIMAGE(File* file, QVector<CodeData>&& data);
    virtual ~SectionTHUMBNAILIMAGE() = default;
    // Section interface
    void parse() override { }
};

}
