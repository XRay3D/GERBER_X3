/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "dxf_entity.h"
namespace Dxf {
struct Wipeout final : Entity {
    Wipeout(SectionParser* sp);
    // Entity interface
public:
    Type type() const override { return Type::WIPEOUT; }
    DxfGo toGo() const override {
        qWarning("%s NOT IMPLEMENTED!", __FUNCTION__);
        return {};
    }
    void parse(CodeData& code) override {
        do {
            data.push_back(code);
            code = sp->nextCode();
        } while(code.code() != 0);
    }
};
} // namespace Dxf
