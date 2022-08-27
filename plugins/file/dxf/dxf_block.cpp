// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_block.h"
#include "dxf_file.h"
#include "section/dxf_entities.h"

namespace Dxf {

Block::Block(Blocks& blocks, SectionParser* sp)
    : sp(sp)
    , blocks(blocks) {
    CodeData code(*(sp->it - 1));
    parseHeader(code);
    parseData(code);
    do {
        code = sp->nextCode();
    } while (code != "BLOCK" && sp->hasNext());
    code = sp->prevCode();
}

Block::~Block() {
}

void Block::parseHeader(CodeData& code) {
    do { // Block header
        bData.push_back(code);
        switch (code.code()) {
        case EntityType:
        case Handle:
        case StartOfApplication_definedGroup:
            //        case EndOfGroup:
        case SoftPointerID:
        case SubclassMarker:
            break;
        case LayerName:
            layerName = code.string();
            break;
            //            case SubclassMarker_2:
            //                break;
        case BlockName:
            blockName = code.string();
            break;
        case BlockTypeFlags:
            flags = code;
            break;
        case BasePointX:
            basePoint.rx() = code;
            break;
        case BasePointY:
            basePoint.ry() = code;
            break;
        case BasePointZ:
            break;
        case BlockName_2:
            blockName = code.string();
            break;
        case XrefPathName:
            xrefPathName = code.string();
            break;
        case BlockDescription:
            blockDescription = code.string();
            break;
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

void Block::parseData(CodeData& code) {
    do {
        if (code == "ENDBLK")
            break;
        //        sp->prevCode(); // unwind parser
        SectionENTITIES se(blocks, code, sp);
        entities = std::move(se.entities);
        if (code == "ENDBLK")
            break;
        code = sp->nextCode();
    } while (code != "ENDBLK");
}

} // namespace Dxf
