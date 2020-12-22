#pragma once

#include "sectionparser.h"

namespace Dxf {

class File;

struct SectionOBJECTS final : SectionParser {
    SectionOBJECTS(File* file, QVector<CodeData>&& data);
    virtual ~SectionOBJECTS() = default;
    // Section interface
    void parse() override { }
};

}
