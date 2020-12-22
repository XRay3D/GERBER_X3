#include "block_record.h"
namespace Dxf {
BLOCK_RECORD::BLOCK_RECORD(SectionParser* sp)
    : TableItem(sp)
{
}

void BLOCK_RECORD::parse(CodeData& code)
{
    do {
        data << code;
        code = sp->nextCode();
    } while (code.code() != 0);
    //    code = sp->prevCode();
}
}
