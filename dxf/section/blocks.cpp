#include "blocks.h"
#include "dxffile.h"

namespace Dxf {
SectionBLOCKS::SectionBLOCKS(File* file, QVector<CodeData>&& data)
    : SectionParser(std::move(data), file)
    , blocks(file->blocks())
{
}

void SectionBLOCKS::parse()
{
    CodeData code;
    while (code != "ENDSEC") {
        code = nextCode();
        if (code == "BLOCK") {
            auto block = new Block(blocks, this);
            block->parse(code);
            if (!block->blockName.isEmpty()) {
                blocks[block->blockName] = block;
            } else {
                qDebug() << "blockName ERR!";
                delete block;
            }
        }
    }
}
}
