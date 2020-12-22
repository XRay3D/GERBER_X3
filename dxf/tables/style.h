#pragma once
#include "tableitem.h"
namespace Dxf {
struct STYLE : TableItem {
public:
    STYLE(SectionParser* sp);
    // TableItem interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return TableItem::STYLE; };
};
}
