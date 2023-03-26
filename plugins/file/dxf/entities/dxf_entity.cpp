// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_entity.h"
#include "dxf_allentities.h"
#include "dxf_file.h"
#include "tables/dxf_layer.h"
#include <QDebug>
#include <QMetaEnum>

namespace Dxf {

std::shared_ptr<Entity> createEntity(Entity::Type key, Blocks& blocks, SectionParser* sp) {
    switch (key) {
    case Entity::ACAD_PROXY_ENTITY:
        return std::make_shared<Dummy /*Dimension*/>(sp);
        break; // return std::make_shared<ACADProxyEntity>(sp);
    case Entity::ARC:
        return std::make_shared<Arc>(sp);
    case Entity::ATTDEF:
        return std::make_shared<AttDef>(sp);
    case Entity::ATTRIB:
        return std::make_shared<Attrib>(sp);
    case Entity::BODY:
        return std::make_shared<Body>(sp);
    case Entity::CIRCLE:
        return std::make_shared<Circle>(sp);
    case Entity::DIMENSION:
        return std::make_shared<Dimension>(sp);
    case Entity::ELLIPSE:
        return std::make_shared<Ellipse>(sp);
    case Entity::HATCH:
        return std::make_shared<Hatch>(sp);
    case Entity::HELIX:
        return std::make_shared<Helix>(sp);
    case Entity::IMAGE:
        return std::make_shared<Image>(sp);
    case Entity::INSERT:
        return std::make_shared<InsertEntity>(blocks, sp);
    case Entity::LEADER:
        return std::make_shared<Leader>(sp);
    case Entity::LIGHT:
        return std::make_shared<Light>(sp);
    case Entity::LINE:
        return std::make_shared<Line>(sp);
    case Entity::LWPOLYLINE:
        return std::make_shared<LwPolyline>(sp);
    case Entity::MESH:
        return std::make_shared<Mesh>(sp);
    case Entity::MLEADER:
        return std::make_shared<MLeader>(sp);
    case Entity::MLEADERSTYLE:
        return std::make_shared<MLeaderStyle>(sp);
    case Entity::MLINE:
        return std::make_shared<MLine>(sp);
    case Entity::MTEXT:
        return std::make_shared<MText>(sp);
    case Entity::OLE2FRAME:
        return std::make_shared<Ole2Frame>(sp);
    case Entity::OLEFRAME:
        return std::make_shared<OleFrame>(sp);
    case Entity::POINT:
        return std::make_shared<Point>(sp);
    case Entity::POLYLINE:
        return std::make_shared<PolyLine>(sp);
    case Entity::RAY:
        return std::make_shared<Ray>(sp);
    case Entity::REGION:
        return std::make_shared<Region>(sp);
    case Entity::SECTION:
        return std::make_shared<Section>(sp);
    case Entity::SEQEND:
        return std::make_shared<SeqEnd>(sp);
    case Entity::SHAPE:
        return std::make_shared<Shape>(sp);
    case Entity::SOLID:
        return std::make_shared<Solid>(sp);
    case Entity::SPLINE:
        return std::make_shared<Spline>(sp);
    case Entity::SUN:
        return std::make_shared<Sun>(sp);
    case Entity::SURFACE:
        return std::make_shared<Surface>(sp);
    case Entity::TABLE:
        return std::make_shared<Table>(sp);
    case Entity::TEXT:
        return std::make_shared<Text>(sp);
    case Entity::TOLERANCE:
        return std::make_shared<Tolerance>(sp);
    case Entity::TRACE:
        return std::make_shared<Trace>(sp);
    case Entity::UNDERLAY:
        return std::make_shared<Underlay>(sp);
    case Entity::VERTEX:
        return std::make_shared<Vertex>(sp);
    case Entity::VIEWPORT:
        return std::make_shared<Viewport>(sp);
    case Entity::WIPEOUT:
        return std::make_shared<Wipeout>(sp);
    case Entity::XLINE:
        return std::make_shared<XLine>(sp);
    default:

        throw std::logic_error(__FUNCTION__);
        //        throw DxfObj::tr("Unknown Entity: %1, %2").arg(key).arg(code.operator QString());
    }
    //    throw DxfObj::tr("Not implemented: %1, %2").arg(key).arg(code.operator QString());
}

QDataStream& operator<<(QDataStream& stream, const std::shared_ptr<Entity>& entity) {
    stream << static_cast<int>(entity->type());
    entity->Entity::write(stream);
    entity->write(stream);
    return stream;
}

QDataStream& operator>>(QDataStream& stream, std::shared_ptr<Entity>& entity) {
    static Blocks blocks;
    Entity::Type type;
    stream >> type;
    entity = createEntity(type, blocks, nullptr);
    entity->Entity::read(stream);
    entity->read(stream);
    return stream;
}

Entity::Entity(SectionParser* sp)
    : sp(sp) { }

Entity::~Entity() { }

void Entity::draw(const InsertEntity* const i) const {
    if (i) {
        for (int r {}; r < i->rowCount; ++r) {
            for (int c {}; c < i->colCount; ++c) {
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

void Entity::parse(CodeData& code) {
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

        break;
    case EntityType: // 0

        break;
    case Handle: // 5
        handle = code.string();

        break;
    case SoftPointerID: // 330
        softPointerID = code.string();

        break;
    case HardOwnerID: // 360

        break;
    case SubclassMarker: // 100

        break;
    case E67: // 67

        break;
    case E410: // 410

        break;
    case LayerName: // 8
        layerName = code.string();

        break;
    case LineType: // 6

        break;
    case E347: // 347

        break;
    case ColorNumber: // 62
        colorNumber = code;

        break;
    case LineWeight: // 370

        break;
    case LineTypeScale: // 48

        break;
    case Visibility: // 60

        break;
    case NumberOfBytes: // 92

        break;
    case BinaryChunk: // 310

        break;
    case A24bitColor: // 420

        break;
    case ColorName: // 430

        break;
    case TransparencyValue: // 440

        break;
    case PlotStyleID: // 390

        break;
    case ShadowMode: // 284

        break;
    default:
        qDebug() << __FUNCTION__ << "default" << code;
        break;
    }
}

void Entity::write(QDataStream& stream) const {
    stream << layerName;
    stream << handle;
    stream << softPointerID;
    stream << colorNumber;
    stream << id;
}

void Entity::read(QDataStream& stream) {
    stream >> layerName;
    stream >> handle;
    stream >> softPointerID;
    stream >> colorNumber;
    stream >> id;
}

Entity::Type Entity::toType(const QString& key) {
    return Type(staticMetaObject.enumerator(0).keyToValue(key.toUtf8().toUpper().data()));
}

QString Entity::typeName(int key) {
    return staticMetaObject.enumerator(0).valueToKey(key);
}

QString Entity::name() const {
    return staticMetaObject.enumerator(0).valueToKey(type());
}

QColor Entity::color() const {
    if (auto layer = sp->file->layer(layerName); layer != nullptr) {
        QColor c(dxfColors[layer->colorNumber()]);
        c.setAlpha(200);
        return c;
    }

    return QColor(255, 0, 255, 100);
}

void Entity::attachToLayer(GraphicObject&& go) const {
    if (sp == nullptr)
        throw DxfObj::tr("SectionParser is null!");
    else if (sp->file == nullptr)
        throw DxfObj::tr("File in SectionParser is null!");
    else if (sp->file->layer(layerName) == nullptr)
        throw DxfObj::tr("Layer '%1' not found in file!").arg(layerName);

    sp->file->layer(layerName)->addGraphicObject(std::move(go));
}

Entity::DataEnum Entity::toDataEnum(const QString& key) {
    return DataEnum(staticMetaObject.enumerator(1).keyToValue(key.toUtf8().toUpper().data()));
}

QPointF polar(QPointF p, float angle, float distance) {
    // Returns the point at a specified `angle` and `distance` from point `p`.
    return p + QPointF(cos(angle) * distance, sin(angle) * distance);
}

double angle(QPointF p1, QPointF p2) {
    // Returns angle a line defined by two endpoints and x-axis in radians.
    p2 -= p1;
    return atan2(p2.y(), p2.x());
}

double signedBulgeRadius(QPointF start_point, QPointF end_point, double bulge) {
    return QLineF(start_point, end_point).length() * (1.0 + (bulge * bulge)) / 4.0 / bulge;
}

std::tuple<QPointF, double, double, double> bulgeToArc(QPointF start_point, QPointF end_point, float bulge) {
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
    double a = angle(start_point, end_point) + (pi / 2.0 - atan(bulge) * 2.0);
    QPointF c = polar(start_point, a, r);
    if (bulge < 0.0)
        return {c, angle(c, end_point), angle(c, start_point), abs(r)};
    else
        return {c, angle(c, start_point), angle(c, end_point), abs(r)};
}

} // namespace Dxf

#include "moc_dxf_entity.cpp"
