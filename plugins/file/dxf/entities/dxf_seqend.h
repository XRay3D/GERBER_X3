#pragma once

#include "dxf_entity.h"

namespace Dxf {

class SeqEnd final : public Entity {
public:
    SeqEnd(SectionParser* sp);
    // Entity interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return SEQEND; };
    GraphicObject toGo() const override;
};

} // namespace Dxf
