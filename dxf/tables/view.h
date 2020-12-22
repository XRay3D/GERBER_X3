#pragma once
#include "tableitem.h"
namespace Dxf {
struct VIEW : TableItem {
public:
    VIEW(SectionParser* sp);
    // TableItem interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return TableItem::VIEW; };
};
}
