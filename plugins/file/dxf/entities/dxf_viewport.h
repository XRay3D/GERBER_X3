/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "dxf_entity.h"
namespace Dxf {
struct Viewport final : Entity {
    Viewport(SectionParser* sp);

    // Entity interface
public:
    Type type() const override {
        ret/*urn Type::WIPEOUT;*/
    }

    GraphicObject toGo() const override {
        return { id, {}, {} };
    }
};
} // namespace Dxf
