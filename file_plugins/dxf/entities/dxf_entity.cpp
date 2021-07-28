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
#include "dxf_entity.h"
#include "dxf_allentities.h"
#include "dxf_file.h"
#include "tables/dxf_layer.h"
#include <QDebug>
#include <QMetaEnum>

namespace Dxf {

QDataStream& operator<<(QDataStream& stream, const std::shared_ptr<Entity>& entity)
{
    stream << static_cast<int>(entity->type());
    entity->Entity::write(stream);
    entity->write(stream);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, std::shared_ptr<Entity>& entity)
{
    static Blocks blocks;
    int type;
    stream >> type;
    switch (type) {
    case Entity::NULL_ENT:
        entity = std::make_shared<Dummy>(nullptr);
        break;
    case Entity::ACAD_PROXY_ENTITY:
        break;
    case Entity::ARC:
        entity = std::make_shared<Arc>(nullptr);
        break;
    case Entity::ATTDEF:
        entity = std::make_shared<AttDef>(nullptr);
        break;
    case Entity::ATTRIB:
        entity = std::make_shared<Attrib>(nullptr);
        break;
    case Entity::BODY:
        break;
    case Entity::CIRCLE:
        entity = std::make_shared<Circle>(nullptr);
        break;
    case Entity::DIMENSION:
        break;
    case Entity::ELLIPSE:
        entity = std::make_shared<Ellipse>(nullptr);
        break;
    case Entity::HATCH:
        entity = std::make_shared<Hatch>(nullptr);
        break;
    case Entity::HELIX:
    case Entity::IMAGE:
        break;
    case Entity::INSERT:
        entity = std::make_shared<InsertEntity>(blocks, nullptr);
    case Entity::LEADER:
    case Entity::LIGHT:
        break;
    case Entity::LINE:
        entity = std::make_shared<Line>(nullptr);
        break;
    case Entity::LWPOLYLINE:
        entity = std::make_shared<LwPolyline>(nullptr);
        break;
    case Entity::MESH:
    case Entity::MLEADER:
    case Entity::MLEADERSTYLE:
    case Entity::MLINE:
        break;
    case Entity::MTEXT:
        entity = std::make_shared<MText>(nullptr);
        break;
    case Entity::OLE2FRAME:
    case Entity::OLEFRAME:
        break;
    case Entity::POINT:
        entity = std::make_shared<Point>(nullptr);
        break;
    case Entity::POLYLINE:
        entity = std::make_shared<PolyLine>(nullptr);
        break;
    case Entity::RAY:
    case Entity::REGION:
    case Entity::SECTION:
    case Entity::SEQEND:
        entity = std::make_shared<SeqEnd>(nullptr);
        break;
    case Entity::SHAPE:
        break;
    case Entity::SOLID:
        entity = std::make_shared<Solid>(nullptr);
        break;
    case Entity::SPLINE:
        entity = std::make_shared<Spline>(nullptr);
        break;
    case Entity::SUN:
    case Entity::SURFACE:
    case Entity::TABLE:
        break;
    case Entity::TEXT:
        entity = std::make_shared<Text>(nullptr);
        break;
    case Entity::TOLERANCE:
    case Entity::TRACE:
    case Entity::UNDERLAY:
    case Entity::VERTEX:
        break;
    case Entity::VIEWPORT:
        entity = std::make_shared<Dummy>(nullptr);
        break;
    case Entity::WIPEOUT:
    case Entity::XLINE:
        break;
    default:
        break;
    }
    if (entity) {
        entity->Entity::read(stream);
        entity->read(stream);
    }
    return stream;
}

Entity::Entity(SectionParser* sp)
    : sp(sp)
{
}

Entity::~Entity() { }

void Entity::draw(const InsertEntity* const i) const
{
    if (i) {
        for (int r = 0; r < i->rowCount; ++r) {
            for (int c = 0; c < i->colCount; ++c) {
                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
                GraphicObject go(toGo());
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        attachToLayer(toGo());
    }
}

void Entity::parse(CodeData& code)
{
    switch (code.code()) {
        //    case LayerName:
        //        layerName = code.string();
        //        break;
        //    case Handle:
        //        handle = code.string();
        //        break;
        //    case ColorNumber:
        //        colorNumber = code;
        //        break;
        //    case SoftPointerID:
        //        softPointerID = code.string();
        //        break;
        //    case NumberOfBytes:
        //        break;

    case EntityName: //  -1
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case EntityType: // 0
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case Handle: // 5
        handle = code.string();
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case SoftPointerID: // 330
        softPointerID = code.string();
        //        qDebug() << DataEnum(code.code()) << code;
        break;
    case HardOwnerID: // 360
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case SubclassMarker: // 100
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case E67: // 67
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case E410: // 410
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case LayerName: // 8
        layerName = code.string();
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case LineType: // 6
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case E347: // 347
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case ColorNumber: // 62
        colorNumber = code;
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case LineWeight: // 370
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case LineTypeScale: // 48
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case Visibility: // 60
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case NumberOfBytes: // 92
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case BinaryChunk: // 310
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case A24bitColor: // 420
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case ColorName: // 430
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case TransparencyValue: // 440
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case PlotStyleID: // 390
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case ShadowMode: // 284
        // qDebug() << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    default:
        qDebug() << "default" << code;
        break;
    }
}

void Entity::write(QDataStream& stream) const
{
    stream << layerName;
    stream << handle;
    stream << softPointerID;
    stream << colorNumber;
    stream << id;
}

void Entity::read(QDataStream& stream)
{
    stream >> layerName;
    stream >> handle;
    stream >> softPointerID;
    stream >> colorNumber;
    stream >> id;
}

Entity::Type Entity::toType(const QString& key)
{
    return Type(staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().toUpper().data()));
}

QString Entity::typeName(int key)
{
    return staticMetaObject.enumerator(0).valueToKey(key);
}

QString Entity::name() const
{
    return staticMetaObject.enumerator(0).valueToKey(type());
}

QColor Entity::color() const
{
    if (auto layer = sp->file->layer(layerName); layer != nullptr) {
        QColor c(dxfColors[layer->colorNumber()]);
        c.setAlpha(200);
        return c;
    }

    return QColor(255, 0, 255, 100);
}

void Entity::attachToLayer(GraphicObject&& go) const
{
    if (sp == nullptr)
        throw DxfObj::tr("SectionParser is null!");
    else if (sp->file == nullptr)
        throw DxfObj::tr("File in SectionParser is null!");
    else if (sp->file->layer(layerName) == nullptr)
        throw DxfObj::tr("Layer '%1' not found in file!").arg(layerName);

    sp->file->layer(layerName)->addGraphicObject(std::move(go));
}

Entity::DataEnum Entity::toDataEnum(const QString& key)
{
    return DataEnum(staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().toUpper().data()));
}

QPointF polar(QPointF p, float angle, float distance)
{
    // Returns the point at a specified `angle` and `distance` from point `p`.
    return p + QPointF(cos(angle) * distance, sin(angle) * distance);
}

double angle(QPointF p1, QPointF p2)
{
    // Returns angle a line defined by two endpoints and x-axis in radians.
    p2 -= p1;
    return atan2(p2.y(), p2.x());
}

double signedBulgeRadius(QPointF start_point, QPointF end_point, double bulge)
{
    return QLineF(start_point, end_point).length() * (1.0 + (bulge * bulge)) / 4.0 / bulge;
}

std::tuple<QPointF, double, double, double> bulgeToArc(QPointF start_point, QPointF end_point, float bulge)
{
    /*
    Returns arc parameters from bulge parameters.
    Based on Bulge to Arc by `Lee Mac`_.
    Args:
        start_point: start vertex as :class:`Vec2` compatible object
        end_point: end vertex as :class:`Vec2` compatible object
        bulge: bulge value
    Returns:
        Tuple: (center, start_angle, end_angle, radius)
    */
    double r = signedBulgeRadius(start_point, end_point, bulge);
    double a = angle(start_point, end_point) + (M_PI / 2.0 - atan(bulge) * 2.0);
    QPointF c = polar(start_point, a, r);
    if (bulge < 0.0)
        return { c, angle(c, end_point), angle(c, start_point), abs(r) };
    else
        return { c, angle(c, start_point), angle(c, end_point), abs(r) };
}

}
