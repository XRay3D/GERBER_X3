#include "dxf_seqend.h"

namespace Dxf {
SeqEnd::SeqEnd(SectionParser* sp)
    : Entity(sp)
{
}

void SeqEnd::parse(CodeData& code)
{
    do {
        data.push_back(code);
        switch (code.code()) {
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

GraphicObject SeqEnd::toGo() const
{
    return {};
}
}
