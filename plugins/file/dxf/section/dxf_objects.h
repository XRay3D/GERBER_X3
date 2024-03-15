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

struct SectionOBJECTS final : SectionParser {
    SectionOBJECTS(File* file, Codes::iterator from, Codes::iterator to);
    virtual ~SectionOBJECTS() = default;
    // Section interface
    void parse() override { }
};

} // namespace Dxf
