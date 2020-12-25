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
    do {
        entityParse(code);
    } while (code != "ENDBLK");
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
        entityParse(code);
    } while (hasNext());

    for (auto e : qAsConst(entities)) {
        e->draw();
    }
    qDebug() << entitiesMap.keys();
}

void SectionENTITIES::entityParse(CodeData& code)
{
    key = Entity::TypeVal(code);
    switch (key) {
    case Entity::ACAD_PROXY_ENTITY:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::ARC:
        //entities.append(new Dummy(sp));
        entities.append(new Arc(sp));
        break;
    case Entity::ATTDEF:
        //entities.append(new Dummy(sp));
        entities.append(new AttDef(sp));
        break;
    case Entity::ATTRIB:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::BODY:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::CIRCLE:
        //entities.append(new Dummy(sp));
        entities.append(new Circle(sp));
        break;
    case Entity::DIMENSION:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::ELLIPSE:
        //entities.append(new Dummy(sp));
        entities.append(new Ellipse(sp));
        break;
    case Entity::HATCH:
        //entities.append(new Dummy(sp));
        entities.append(new Hatch(sp));
        break;
    case Entity::HELIX:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::IMAGE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::INSERT:
        //entities.append(new Dummy(sp));
        entities.append(new InsertEntity(blocks, sp));
        break;
    case Entity::LEADER:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::LIGHT:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::LINE:
        //entities.append(new Dummy(sp));
        entities.append(new Line(sp));
        break;
    case Entity::LWPOLYLINE:
        //entities.append(new Dummy(sp));
        entities.append(new LwPolyline(sp));
        break;
    case Entity::MESH:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::MLEADER:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::MLEADERSTYLE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::MLINE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::MTEXT:
        //entities.append(new Dummy(sp));
        entities.append(new MText(sp));
        break;
    case Entity::OLE2FRAME:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::OLEFRAME:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::POINT:
        //entities.append(new Dummy(sp));
        entities.append(new Point(sp));
        break;
    case Entity::POLYLINE:
        //entities.append(new Dummy(sp, Entity::POLYLINE));
        entities.append(new PolyLine(sp));
        break;
    case Entity::RAY:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::REGION:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::SECTION:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::SEQEND:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::SHAPE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::SOLID:
        //entities.append(new Dummy(sp));
        entities.append(new Solid(sp));
        break;
    case Entity::SPLINE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        //entities.append(new Dummy(sp));
        entities.append(new Spline(sp));
        break;
    case Entity::SUN:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::SURFACE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::TABLE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::TEXT:
        //entities.append(new Dummy(sp));
        entities.append(new Text(sp));
        break;
    case Entity::TOLERANCE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::TRACE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::UNDERLAY:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::VERTEX:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::VIEWPORT:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::WIPEOUT:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    case Entity::XLINE:
        throw QString("Not implemented: %1, %2").arg(key).arg(code.operator QString());
        break;
    default:
        throw QString("Unknown Entity: %1, %2").arg(key).arg(code.operator QString());
        break;
    }

    entities.last()->parse(code);
    entitiesMap[key] << entities.last();
}
}
