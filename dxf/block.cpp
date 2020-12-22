#include "block.h"
#include "dxffile.h"

#include <section/entities.h>
namespace Dxf {
Block::Block(QMap<QString, Block*>& blocks, SectionParser* sp)
    : sp(sp)
    , blocks(blocks)
{
}

Block::~Block()
{ /*qDeleteAll(entities);*/
}

void Block::parse(CodeData& code)
{
    parseHeader(code);
    parseData(code);
    do {
        code = sp->nextCode();
    } while (code.code() != 0);
    code = sp->prevCode();
}

void Block::parseHeader(CodeData& code)
{
    do { // Block header
        bData.append(code);
        switch (code.code()) {
        case EntityType:
            break;
        case Handle:
            break;
            //            case StartOfApplication_definedGroup:
            //                break;
            //            case EndOfGroup:
            //                break;
        case SoftPointerID:
            break;
        case SubclassMarker:
            break;
        case LayerName:
            layerName = QString(code);
            break;
            //            case SubclassMarker_2:
            //                break;
        case BlockName:
            blockName = QString(code);
            break;
        case BlockTypeFlags:
            flags = static_cast<BTFlags>(code.operator long long());
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
            blockName = QString(code);
            break;
        case XrefPathName:
            break;
        case BlockDescription:
            blockDescription = QString(code);
            break;
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

void Block::parseData(CodeData& code)
{
    do {
        if (code == "ENDBLK")
            break;
        SectionENTITIES se(blocks, code, sp);
        entities = std::move(se.entities);
        if (code == "ENDBLK")
            break;
        code = sp->nextCode();
    } while (code != "ENDBLK");
}
}
