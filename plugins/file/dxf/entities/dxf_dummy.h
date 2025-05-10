/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "dxf_entity.h"

namespace Dxf {

class Dummy final : public Entity {
    Type type_;

public:
    Dummy(SectionParser* sp, Type type = NULL_ENT);

    // Entity interface
public:
    void draw(const InsertEntity* const) const override;
    void parse(CodeData& code) override;
    Type type() const override { return NULL_ENT; }
    DxfGo toGo() const override {
        qWarning("%s NOT IMPLEMENTED!", __FUNCTION__);
        return {};
    }
    // void write(QDataStream&) const override { }
    // void read(QDataStream&) override { }
};

} // namespace Dxf
