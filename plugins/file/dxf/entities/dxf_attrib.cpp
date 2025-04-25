/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_attrib.h"
namespace Dxf {
Attrib::Attrib(SectionParser* sp)
    : Entity{sp} {
}

void Attrib::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

DxfGo Attrib::toGo() const {
    return {};
}

} // namespace Dxf
