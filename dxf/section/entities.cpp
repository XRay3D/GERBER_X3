#include "entities.h"
#include "dxffile.h"
#include "entities/dxf_entity.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>

namespace Dxf {
SectionENTITIES::SectionENTITIES(File* file, QVector<CodeData>&& data)
    : SectionParser(std::move(data), file)
    , sp(this)
    , blocks(file->blocks())
{
}

SectionENTITIES::SectionENTITIES(QMap<QString, Block*>& blocks, CodeData& code, SectionParser* sp)
    : SectionParser({ {}, {} }, sp->file)
    , sp(sp)
    , blocks(sp->file->blocks())
{
    do {
        iParse(code);
    } while (code != "ENDBLK");
}

void SectionENTITIES::parse()
{
    code = nextCode();
    code = nextCode();
    code = nextCode();
    while (code != "ENDSEC") {
        if (code == "ENDSEC")
            break;
        iParse(code);
    }

    for (auto e : qAsConst(entities)) {
        e->draw();
    }

    qDebug() << entitiesMap.keys();

    //    QTimer::singleShot(100, [] {
    //        auto r = scene->itemsBoundingRect();
    //        r.moveTopLeft(r.topLeft() + QPointF { +1, +1 });
    //        r.moveBottomRight(r.bottomRight() + QPointF { -1, -1 });
    //        scene->setSceneRect(r);
    //        reinterpret_cast<QGraphicsView*>(scene->parent())->scale(100, 100);
    //        reinterpret_cast<QGraphicsView*>(scene->parent())->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    //    });
}

void SectionENTITIES::iParse(CodeData& code)
{
    key = Entity::TypeVal(code);
    qDebug() << key << code;
    switch (key) {
    case Entity::ACAD_PROXY_ENTITY:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::ARC:
        entities.append(new ARC(sp));
        break;
    case Entity::ATTDEF:
        entities.append(new ATTDEF(sp));
        break;
    case Entity::ATTRIB:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::BODY:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::CIRCLE:
        entities.append(new CIRCLE(sp));
        break;
    case Entity::DIMENSION:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::ELLIPSE:
        entities.append(new ELLIPSE(sp));
        break;
    case Entity::HATCH:
        entities.append(new HATCH(sp));
        break;
    case Entity::HELIX:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::IMAGE:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::INSERT:
        entities.append(new INSERT_ET(blocks, sp));
        break;
    case Entity::LEADER:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::LIGHT:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::LINE:
        entities.append(new LINE(sp));
        break;
    case Entity::LWPOLYLINE:
        entities.append(new LWPOLYLINE(sp));
        break;
    case Entity::MESH:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::MLEADER:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::MLEADERSTYLE:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::MLINE:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::MTEXT:
        entities.append(new MTEXT(sp));
        break;
    case Entity::OLE2FRAME:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::OLEFRAME:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::POINT:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::POLYLINE: ////////////
        entities.append(new POLYLINE(sp));
        break;
    case Entity::RAY:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::REGION:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::SECTION:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::SEQEND:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::SHAPE:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::SOLID:
        entities.append(new SOLID(sp));
        break;
    case Entity::SPLINE:
        entities.append(new SPLINE(sp));
        break;
    case Entity::SUN:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::SURFACE:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::TABLE:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::TEXT:
        entities.append(new TEXT(sp));
        break;
    case Entity::TOLERANCE:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::TRACE:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::UNDERLAY:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::VERTEX:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::VIEWPORT:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::WIPEOUT:
        qDebug() << key << code;
        exit(-1000);
        break;
    case Entity::XLINE:
        qDebug() << key << code;
        exit(-1000);
        break;
    default:
        break;
    }

    entities.last()->parse(code);
    entitiesMap[key] << entities.last();
}
}
