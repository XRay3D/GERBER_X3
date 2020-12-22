#pragma once
#include "tableitem.h"
namespace Dxf {
struct UCS final : TableItem {
public:
    UCS(SectionParser* sp);
    // TableItem interface
    void parse(CodeData& code) override;
    Type type() const override { return TableItem::UCS; };
};
}
