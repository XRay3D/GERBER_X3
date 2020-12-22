#pragma once

#include "header.h"
#include "sectionparser.h"

namespace Dxf {

class File;

struct SectionHEADER final : SectionParser {
    SectionHEADER(File* file, QVector<CodeData>&& data);
    virtual ~SectionHEADER() = default;
    // Section interface
    void parse() override;
    Header& header;
};

}
