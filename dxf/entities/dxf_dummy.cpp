#include "dxf_dummy.h"

namespace Dxf {

Dxf::Dummy::Dummy(Dxf::SectionParser* sp, Dxf::Entity::Type type)
    : Entity(sp)
    , m_type(type)
{
}

void Dummy::draw(const Dxf::InsertEntity* const) const
{
}

void Dummy::parse(Dxf::CodeData& code)
{
    switch (m_type) {
    case Type::POLYLINE:
        do {
            code = sp->nextCode();
        } while (code != "SEQEND");
        do {
            code = sp->nextCode();
        } while (code.code() != 0);
        break;
    default:
        do {
            data.push_back(code);
            //        switch (static_cast<VarType>(code.code())) {
            //        default:
            //            parseEntity(code);
            //        }
            code = sp->nextCode();
        } while (code.code() != 0);
    }
}

}
