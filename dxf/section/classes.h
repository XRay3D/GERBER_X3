#pragma once

#include "sectionparser.h"
namespace Dxf {

class File;

struct SectionCLASSES final : SectionParser {
    SectionCLASSES(File* file, QVector<CodeData>&& data);
    virtual ~SectionCLASSES() = default;
    // Section interface
    void parse() override { }
};

}
