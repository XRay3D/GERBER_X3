#include "ucs.h"
namespace Dxf {
UCS::UCS(SectionParser* sp)
    : TableItem(sp)
{
}

void UCS::parse(CodeData& code)
{
    do {
        data << code;
        code = sp->nextCode();
    } while (code.code() != 0);
    //    code = sp->prevCode();
}
}
