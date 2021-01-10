// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "entities/dxf_entity.h"

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
    entitiesMap.clear();
    do {
        entities.append(entityParse(code));
        entities.last()->parse(code);
        entitiesMap[key] << entities.last();
    } while (code != "ENDBLK");
    qDebug() << entitiesMap.keys();
}

SectionENTITIES::~SectionENTITIES()
{
    qDeleteAll(entities);
}

void SectionENTITIES::parse()
{
    CodeData code = nextCode();
    code = nextCode();
    code = nextCode();
    do {
        entities.append(entityParse(code));
        entities.last()->parse(code);
        entitiesMap[key] << entities.last();
    } while (hasNext());
    for (auto e : qAsConst(entities))
        e->draw();
    qDebug() << entitiesMap.keys();
}

Entity* SectionENTITIES::entityParse(CodeData& code)
{
    key = Entity::toType(code);
    switch (key) {
    case Entity::ACAD_PROXY_ENTITY:
        break;
    case Entity::ARC:
        return new Arc(sp); //return new Dummy(sp);
    case Entity::ATTDEF:
        return new AttDef(sp); //return new Dummy(sp);
    case Entity::ATTRIB:
    case Entity::BODY:
        break;
    case Entity::CIRCLE:
        return new Circle(sp); //return new Dummy(sp);
    case Entity::DIMENSION:
        break;
    case Entity::ELLIPSE:
        return new Ellipse(sp); //return new Dummy(sp);
    case Entity::HATCH:
        return new Hatch(sp); //return new Dummy(sp);
    case Entity::HELIX:
    case Entity::IMAGE:
        break;
    case Entity::INSERT:
        return new InsertEntity(blocks, sp); //return new Dummy(sp);
    case Entity::LEADER:
    case Entity::LIGHT:
        break;
    case Entity::LINE:
        return new Line(sp); //return new Dummy(sp);
    case Entity::LWPOLYLINE:
        return new LwPolyline(sp); //return new Dummy(sp);
    case Entity::MESH:
    case Entity::MLEADER:
    case Entity::MLEADERSTYLE:
    case Entity::MLINE:
        break;
    case Entity::MTEXT:
        return new MText(sp); //return new Dummy(sp);
    case Entity::OLE2FRAME:
    case Entity::OLEFRAME:
        break;
    case Entity::POINT:
        return new Point(sp); //return new Dummy(sp);
    case Entity::POLYLINE:
        return new PolyLine(sp); //entities.append(new Dummy(sp, Entity::POLYLINE));
    case Entity::RAY:
    case Entity::REGION:
    case Entity::SECTION:
    case Entity::SEQEND:
    case Entity::SHAPE:
        break;
    case Entity::SOLID:
        return new Solid(sp); //return new Dummy(sp);
    case Entity::SPLINE:
        return new Spline(sp); //return new Dummy(sp);
    case Entity::SUN:
    case Entity::SURFACE:
    case Entity::TABLE:
        break;
    case Entity::TEXT:
        return new Text(sp); //return new Dummy(sp);
    case Entity::TOLERANCE:
    case Entity::TRACE:
    case Entity::UNDERLAY:
    case Entity::VERTEX:
        break;
    case Entity::VIEWPORT:
        return new Dummy(sp); //return new Dummy(sp);
    case Entity::WIPEOUT:
    case Entity::XLINE:
        break;
    default:
        throw DxfObj::tr("Unknown Entity: %1, %2").arg(key).arg(code.operator QString());
    }
    throw DxfObj::tr("Not implemented: %1, %2").arg(key).arg(code.operator QString());
}
}
