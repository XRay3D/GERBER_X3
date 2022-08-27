// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_ltype.h"

namespace Dxf {

LType::LType(SectionParser* sp)
    : TableItem(sp) {
}

void LType::parse(CodeData& code) {
    do {
        data.push_back(code);
        code = sp->nextCode();
    } while (code.code() != 0);
    //    code = sp->prevCode();
}

} // namespace Dxf
