#include "vport.h"
namespace Dxf {
VPORT::VPORT(SectionParser* sp)
    : TableItem(sp)
{
}

void VPORT::parse(CodeData& code)
{
    do {
        data << code;
        code = sp->nextCode();
    } while (code.code() != 0);
    //    code = sp->prevCode();
}
}
