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

#include "dxf_sectionparser.h"

namespace Dxf {

class File;

struct SectionCLASSES final : SectionParser {
    SectionCLASSES(File* file, Codes::iterator from, Codes::iterator to);
    virtual ~SectionCLASSES() = default;
    // Section interface
    void parse() override { }
};

} // namespace Dxf
