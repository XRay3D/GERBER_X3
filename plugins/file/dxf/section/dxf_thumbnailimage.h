/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  * * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "dxf_sectionparser.h"

namespace Dxf {

class File;

struct SectionTHUMBNAILIMAGE final : SectionParser {
    SectionTHUMBNAILIMAGE(File* file, Codes::iterator from, Codes::iterator to);
    virtual ~SectionTHUMBNAILIMAGE() = default;
    // Section interface
    void parse() override { }
};

} // namespace Dxf
