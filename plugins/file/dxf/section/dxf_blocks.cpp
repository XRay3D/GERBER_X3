// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_blocks.h"
#include "dxf_file.h"

namespace Dxf {
SectionBLOCKS::SectionBLOCKS(File* file, Codes::iterator from, Codes::iterator to)
    : SectionParser(from, to, file)
    , blocks(file->blocks()) {
}

void SectionBLOCKS::parse() {
    CodeData code;
    do {
        code = nextCode();
        if (code == "BLOCK") {
            auto block = new Block(blocks, this);
            if (!block->blockName.isEmpty()) {
                blocks[block->blockName] = block;
            } else {
                delete block;
                throw DxfObj::tr("blockName ERR!");
            }
        }
    } while (code != "ENDSEC");
}

} // namespace Dxf
