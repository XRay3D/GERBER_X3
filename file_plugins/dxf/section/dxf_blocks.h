/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include "dxf_block.h"
#include "dxf_sectionparser.h"

namespace Dxf {

class File;

struct SectionBLOCKS final : SectionParser {
    SectionBLOCKS(File* file, Codes::iterator from, Codes::iterator to);
    virtual ~SectionBLOCKS() = default;
    // Section interface
    void parse() override;

    Blocks& blocks;
};

} // namespace Dxf
