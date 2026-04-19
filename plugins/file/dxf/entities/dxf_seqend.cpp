#include "dxf_seqend.h"

namespace Dxf {

void SeqEnd::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        default: Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

DxfGo SeqEnd::toGo() const {
    qInfo("SeqEnd");
    return {};
}

} // namespace Dxf
