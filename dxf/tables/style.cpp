#include "style.h"
namespace Dxf {
STYLE::STYLE(SectionParser* sp)
    : TableItem(sp)
{
}

void STYLE::parse(CodeData& code)
{
    do {
        data << code;
        code = sp->nextCode();
    } while (code.code() != 0);
    //    code = sp->prevCode();
}
}
