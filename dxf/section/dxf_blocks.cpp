#include "dxf_blocks.h"
#include "dxf_file.h"

namespace Dxf {
SectionBLOCKS::SectionBLOCKS(File* file, Codes::iterator from, Codes::iterator to)
    : SectionParser(from, to, file)
    , blocks(file->blocks())
{
}

void SectionBLOCKS::parse()
{
    CodeData code;
    do {
        code = nextCode();
        if (code == "BLOCK") {
            auto block = new Block(blocks, this);
            if (!block->blockName.isEmpty()) {
                blocks[block->blockName] = block;
            } else {
                delete block;
                throw QString("blockName ERR!");
            }
        }
    } while (code != "ENDSEC");
}

}
