#pragma once

#include "dxf_block.h"
#include "dxf_sectionparser.h"

namespace Dxf {

class File;

struct SectionBLOCKS final : SectionParser {
    SectionBLOCKS(File* file, Codes::iterator from, Codes::iterator to);
    virtual ~SectionBLOCKS() = default;
    // Section interface
    void parse() override;

    Blocks& blocks;
};

}
