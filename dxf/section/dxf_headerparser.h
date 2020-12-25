#pragma once

#include "dxf_sectionparser.h"
#include "dxf_values.h"

namespace Dxf {

class File;

struct SectionHEADER final : SectionParser {
    SectionHEADER(File* file, Codes::iterator from, Codes::iterator to);

    virtual ~SectionHEADER() = default;
    // Section interface
    void parse() override;
    
    
    HeaderData& header;
};

}
