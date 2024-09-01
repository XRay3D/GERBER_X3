// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_dummy.h"

namespace Dxf {

Dxf::Dummy::Dummy(Dxf::SectionParser* sp, Dxf::Entity::Type type)
    : Entity{sp}
    , type_(type) {
}

void Dummy::draw(const Dxf::InsertEntity* const) const {
}

void Dummy::parse(Dxf::CodeData& code) {
    switch(type_) {
    case Type::POLYLINE:
        do {
            code = sp->nextCode();
        } while(code != "SEQEND");
        do {
            code = sp->nextCode();
        } while(code.code() != 0);
        break;
    default:
        do {
            data.push_back(code);
            //        switch (static_cast<DataEnum>(code.code())) {
            //        default:
            //            Entity::parse(code);
            //        }
            code = sp->nextCode();
        } while(code.code() != 0);
    }
}

} // namespace Dxf
