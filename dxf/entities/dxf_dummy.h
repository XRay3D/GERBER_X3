#pragma once

#include "dxf_entity.h"

namespace Dxf {

class Dummy final : public Entity {
    Type m_type;

public:
    Dummy(SectionParser* sp, Type type = NULL_ENT);

    // Entity interface
public:
    void draw(const InsertEntity* const) const override;
    void parse(CodeData& code) override;
    Type type() const override { return NULL_ENT; }
};

}
