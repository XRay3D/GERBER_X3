#pragma once

#include "dxf_entity.h"

namespace Dxf {

struct SeqEnd final : Entity {
    using Entity::Entity;
    // Entity interface
    void parse(CodeData& code) override;
    Type type() const override { return SEQEND; };
    DxfGo toGo() const override;
};

} // namespace Dxf
