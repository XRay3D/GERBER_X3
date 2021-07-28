// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_entities.h"
#include "dxf_file.h"
#include "entities/dxf_allentities.h"

#include <QGraphicsView>
#include <QTimer>

namespace Dxf {
SectionENTITIES::SectionENTITIES(File* file, Codes::iterator from, Codes::iterator to)
    : SectionParser(from, to, file)
    , sp(this)
    , blocks(file->blocks())
{
}

SectionENTITIES::SectionENTITIES(Blocks& blocks, CodeData& code, SectionParser* sp)
    : SectionParser(sp->from, sp->to, sp->it, sp->file)
    , sp(sp)
    , blocks(blocks)
{
    do {
        file->m_entities.emplace_back(entityParse(code));
        file->m_entities.back()->parse(code);
        file->m_entities.back()->id = file->m_entities.size() - 1;
        entities.push_back(file->m_entities.back().get());
    } while (code != "ENDBLK");
}

SectionENTITIES::~SectionENTITIES()
{
}

void SectionENTITIES::parse()
{
    CodeData code = nextCode();
    code = nextCode();
    code = nextCode();
    do {
        file->m_entities.emplace_back(entityParse(code));
        file->m_entities.back()->parse(code);
        file->m_entities.back()->id = file->m_entities.size() - 1;
    } while (hasNext());
    for (auto& e : qAsConst(file->m_entities))
        e->draw();
}

std::shared_ptr<Entity> SectionENTITIES::entityParse(CodeData& code)
{
    key = Entity::toType(code);
    switch (key) {
    case Entity::ACAD_PROXY_ENTITY:
        break;
    case Entity::ARC:
        return std::make_shared<Arc>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::ATTDEF:
        return std::make_shared<AttDef>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::ATTRIB:
        return std::make_shared<Attrib>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::BODY:
        break;
    case Entity::CIRCLE:
        return std::make_shared<Circle>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::DIMENSION:
        break;
    case Entity::ELLIPSE:
        return std::make_shared<Ellipse>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::HATCH:
        return std::make_shared<Hatch>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::HELIX:
    case Entity::IMAGE:
        break;
    case Entity::INSERT:
        return std::make_shared<InsertEntity>(blocks, sp); //return std::make_shared<Dummy>(sp);
    case Entity::LEADER:
    case Entity::LIGHT:
        break;
    case Entity::LINE:
        return std::make_shared<Line>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::LWPOLYLINE:
        return std::make_shared<LwPolyline>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::MESH:
    case Entity::MLEADER:
    case Entity::MLEADERSTYLE:
    case Entity::MLINE:
        break;
    case Entity::MTEXT:
        return std::make_shared<MText>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::OLE2FRAME:
    case Entity::OLEFRAME:
        break;
    case Entity::POINT:
        return std::make_shared<Point>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::POLYLINE:
        return std::make_shared<PolyLine>(sp); //entities.append(make_shared<Dummy>(sp, Entity::POLYLINE));
    case Entity::RAY:
    case Entity::REGION:
    case Entity::SECTION:
    case Entity::SEQEND:
        return std::make_shared<SeqEnd>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::SHAPE:
        break;
    case Entity::SOLID:
        return std::make_shared<Solid>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::SPLINE:
        return std::make_shared<Spline>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::SUN:
    case Entity::SURFACE:
    case Entity::TABLE:
        break;
    case Entity::TEXT:
        return std::make_shared<Text>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::TOLERANCE:
    case Entity::TRACE:
    case Entity::UNDERLAY:
    case Entity::VERTEX:
        break;
    case Entity::VIEWPORT:
        return std::make_shared<Dummy>(sp); //return std::make_shared<Dummy>(sp);
    case Entity::WIPEOUT:
    case Entity::XLINE:
        break;
    default:
        throw DxfObj::tr("Unknown Entity: %1, %2").arg(key).arg(code.operator QString());
    }
    throw DxfObj::tr("Not implemented: %1, %2").arg(key).arg(code.operator QString());
}
}
