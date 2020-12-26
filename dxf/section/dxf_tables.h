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

#include "dxf_file.h"
#include "dxf_sectionparser.h"

#include "tables/dxf_layer.h"
#include "tables/dxf_abstracttable.h"

//#include "dxf_appid.h"
//#include "dxf_block_record.h"
//#include "dxf_dimstyle.h"
//#include "dxf_ltype.h"
//#include "dxf_style.h"
//#include "dxf_ucs.h"
//#include "dxf_view.h"
//#include "dxf_vport.h"

namespace Dxf {

struct AbstractTable;
class Layer;

struct SectionTABLES final : SectionParser {
    SectionTABLES(File* file, Codes::iterator from, Codes::iterator to);
    ~SectionTABLES();

    // Section interface
    void parse() override;

    Tables tables;
    Layers& layers;
};

}
