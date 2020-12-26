/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "dxf_sectionparser.h"
#include "dxf_types.h"

namespace Dxf {

class File;

struct SectionHEADER final : SectionParser {
    SectionHEADER(File* file, Codes::iterator from, Codes::iterator to);

    virtual ~SectionHEADER() = default;
    // Section interface
    void parse() override;
    
    
    HeaderData& header;
};

}
