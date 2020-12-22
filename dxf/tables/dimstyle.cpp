#include "dimstyle.h"
namespace Dxf {
DIMSTYLE::DIMSTYLE(SectionParser* sp)
    : TableItem(sp)
{
}

void DIMSTYLE::parse(CodeData& code)
{
    do {
        data << code;
        code = sp->nextCode();
    } while (code.code() != 0);
    //    code = sp->prevCode();
}
}
