#pragma once
#include "tableitem.h"
namespace Dxf {
struct LTYPE : TableItem {
public:
    LTYPE(SectionParser* sp);
    // TableItem interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return TableItem::LTYPE; };
};
}
