#pragma once

#include "dxffile.h"
#include "sectionparser.h"

namespace Dxf {

struct TableItem;
struct LAYER;

struct SectionTABLES final : SectionParser {
    SectionTABLES(File* file, QVector<CodeData>&& data);
    virtual ~SectionTABLES() = default;
    // Section interface
    void parse() override;

    QVector<QVector<TableItem*>> tables;
    Layers& layers;
};

}
