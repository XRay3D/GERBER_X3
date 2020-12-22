#include "appid.h"
namespace Dxf {
APPID::APPID(SectionParser* sp)
    : TableItem(sp)
{
}

void APPID::parse(CodeData& code)
{
    do {
        data << code;
        code = sp->nextCode();
        switch (code.code()) {
        case SubclassMarker:
            break;
        case ApplicationName:
            applicationName = code;
            break;
        case StandardFlag:
            standardFlag = code;
            break;
        }
    } while (code.code() != 0);
}
}
