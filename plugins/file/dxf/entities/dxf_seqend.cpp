// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "dxf_seqend.h"

namespace Dxf {
SeqEnd::SeqEnd(SectionParser* sp)
    : Entity{sp} {
}

void SeqEnd::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

DxfGo SeqEnd::toGo() const {
    return {};
}

} // namespace Dxf
