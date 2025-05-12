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
#include "dxf_blocks.h"
#include "dxf_file.h"

namespace Dxf {
SectionBLOCKS::SectionBLOCKS(File* file, Codes::iterator from, Codes::iterator to)
    : SectionParser(from, to, file)
    , blocks(file->blocks()) {
    parse();
}

void SectionBLOCKS::parse() {
    CodeData code;
    do {
        code = nextCode();
        if(code == "BLOCK") {
            auto block = new Block{blocks, this};
            if(!block->blockName.isEmpty()) {
                blocks[block->blockName] = block;
            } else {
                delete block;
                throw Exception{DxfObj::tr("blockName ERR!")};
            }
        }
    } while(code != "ENDSEC");
}

} // namespace Dxf
