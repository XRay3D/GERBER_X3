/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "dxf_entity.h"

namespace Dxf {

struct Dummy final : Entity {
    Type type_;

    Dummy(SectionParser* sp, Type type = NULL_ENT);

    // Entity interface
    void draw(const InsertEntity* const) const override;
    void parse(CodeData& code) override;
    Type type() const override { return NULL_ENT; }
    DxfGo toGo() const override {
        qWarning("%s NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return {};
    }
};

} // namespace Dxf
