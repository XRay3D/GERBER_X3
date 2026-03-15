/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
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
    switch(key) {
    case Entity::ACAD_PROXY_ENTITY: return std::make_shared<Dummy /*Dimension*/>(sp);
    case Entity::ARC              : return std::make_shared<Arc>(sp);
    case Entity::ATTDEF           : return std::make_shared<AttDef>(sp);
    case Entity::ATTRIB           : return std::make_shared<Attrib>(sp);
    case Entity::BODY             : return std::make_shared<Body>(sp);
    case Entity::CIRCLE           : return std::make_shared<Circle>(sp);
    case Entity::DIMENSION        : return std::make_shared<Dimension>(sp);
    case Entity::ELLIPSE          : return std::make_shared<Ellipse>(sp);
    case Entity::HATCH            : return std::make_shared<Hatch>(sp);
    case Entity::HELIX            : return std::make_shared<Helix>(sp);
    case Entity::IMAGE            : return std::make_shared<Image>(sp);
    case Entity::INSERT           : return std::make_shared<InsertEntity>(blocks, sp);
    case Entity::LEADER           : return std::make_shared<Leader>(sp);
    case Entity::LIGHT            : return std::make_shared<Light>(sp);
    case Entity::LINE             : return std::make_shared<Line>(sp);
    case Entity::LWPOLYLINE       : return std::make_shared<LwPolyline>(sp);
    case Entity::MESH             : return std::make_shared<Mesh>(sp);
    case Entity::MLEADER          : return std::make_shared<MLeader>(sp);
    case Entity::MLEADERSTYLE     : return std::make_shared<MLeaderStyle>(sp);
    case Entity::MLINE            : return std::make_shared<MLine>(sp);
    case Entity::MTEXT            : return std::make_shared<MText>(sp);
    case Entity::OLE2FRAME        : return std::make_shared<Ole2Frame>(sp);
    case Entity::OLEFRAME         : return std::make_shared<OleFrame>(sp);
    case Entity::POINT            : return std::make_shared<Point>(sp);
    case Entity::POLYLINE         : return std::make_shared<PolyLine>(sp);
    case Entity::RAY              : return std::make_shared<Ray>(sp);
    case Entity::REGION           : return std::make_shared<Region>(sp);
    case Entity::SECTION          : return std::make_shared<Section>(sp);
    case Entity::SEQEND           : return std::make_shared<SeqEnd>(sp);
    case Entity::SHAPE            : return std::make_shared<AbstractShape>(sp);
    case Entity::SOLID            : return std::make_shared<Solid>(sp);
    case Entity::SPLINE           : return std::make_shared<Spline>(sp);
    case Entity::SUN              : return std::make_shared<Sun>(sp);
    case Entity::SURFACE          : return std::make_shared<Surface>(sp);
    case Entity::TABLE            : return std::make_shared<Table>(sp);
    case Entity::TEXT             : return std::make_shared<Text>(sp);
    case Entity::TOLERANCE        : return std::make_shared<Tolerance>(sp);
    case Entity::TRACE            : return std::make_shared<Trace>(sp);
    case Entity::UNDERLAY         : return std::make_shared<Underlay>(sp);
    case Entity::VERTEX           : return std::make_shared<Vertex>(sp);
    case Entity::VIEWPORT         : return std::make_shared<Viewport>(sp);
    case Entity::WIPEOUT          : return std::make_shared<Wipeout>(sp);
    case Entity::XLINE            : return std::make_shared<XLine>(sp);
    default                       : throw std::logic_error(__FUNCTION__); // throw DxfObj::tr("Unknown Entity: %1, %2").arg(key).arg(code.operator QString());
    }
    // throw DxfObj::tr("Not implemented: %1, %2").arg(key).arg(code.operator QString());
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
    : sp{sp} { }

Entity::~Entity() { }

void Entity::draw(const InsertEntity* const i) const {
    if(i) {
        for(int r{}; r < i->rowCount; ++r) {
            for(int c{}; c < i->colCount; ++c) {
                QPointF tr{r * i->rowSpacing, c * i->colSpacing};
                DxfGo go(toGo());
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        attachToLayer(toGo());
    }
}

void Entity::parse(CodeData& code) {
    switch(code.code()) {
        // case LayerName:
        // layerName = code.string();
        // break;
        // case Handle:
        // handle = code.string();
        // break;
        // case ColorNumber:
        // colorNumber = code;
        // break;
        // case SoftPointerID:
        // softPointerID = code.string();
        // break;
        // case NumberOfBytes:
        // break;

    case EntityName       : break;                                // -1 //
    case EntityType       : break;                                // 0 //
    case Handle           : handle = code.string(); break;        // 5 //
    case SoftPointerID    : softPointerID = code.string(); break; // 330 //
    case HardOwnerID      : break;                                // 360 //
    case SubclassMarker   : break;                                // 100 //
    case E67              : break;                                // 67 //
    case E410             : break;                                // 410 //
    case LayerName        : layerName = code.string(); break;     // 8 //
    case LineType         : break;                                // 6 //
    case E347             : break;                                // 347 //
    case ColorNumber      : colorNumber = code; break;            // 62 //
    case LineWeight       : lineWeight = code; break;             // 370 //
    case LineTypeScale    : break;                                // 48 //
    case Visibility       : break;                                // 60 //
    case NumberOfBytes    : break;                                // 92 //
    case BinaryChunk      : break;                                // 310 //
    case A24bitColor      : break;                                // 420 //
    case ColorName        : break;                                // 430 //
    case TransparencyValue: break;                                // 440 //
    case PlotStyleID      : break;                                // 390 //
    case ShadowMode       : break;                                       // 284 //
    default:
        qDebug() << __FUNCTION__ << u"default"_s << code;
        break;
    }
}

void Entity::write(QDataStream& stream) const {
    stream << layerName;
    stream << lineWeight;
    stream << handle;
    stream << softPointerID;
    stream << colorNumber;
    stream << id;
}

void Entity::read(QDataStream& stream) {
    stream >> layerName;
    stream >> lineWeight;
    stream >> handle;
    stream >> softPointerID;
    stream >> colorNumber;
    stream >> id;
}

Entity::Type Entity::toType(const QString& key) {
    return Type(staticMetaObject.enumerator(0).keyToValue(key.toUtf8().toUpper().data()));
}

QString Entity::typeName(int key) { return QString::fromUtf8(staticMetaObject.enumerator(0).valueToKey(key)); }

QString Entity::name() const { return QString::fromUtf8(staticMetaObject.enumerator(0).valueToKey(type())); }

QColor Entity::color() const {
    if(auto layer = sp->file->layer(layerName); layer != nullptr) { //-V2006
        QColor c(dxfColors[layer->colorNumber()]);
        c.setAlpha(200);
        return c;
    }

    return QColor(255, 0, 255, 100);
}

void Entity::attachToLayer(DxfGo&& go) const {
    if(sp == nullptr)
        throw DxfObj::tr("SectionParser is null!");
    else if(sp->file == nullptr)
        throw DxfObj::tr("File in SectionParser is null!");
    else if(sp->file->layer(layerName) == nullptr)
        throw DxfObj::tr("Layer '%1' not found in file!").arg(layerName);

    sp->file->layer(layerName)->addGraphicObject(std::move(go));
}

Entity::DataEnum Entity::toDataEnum(const QString& key) {
    return DataEnum(staticMetaObject.enumerator(1).keyToValue(key.toUtf8().toUpper().data()));
}

} // namespace Dxf

#include "moc_dxf_entity.cpp"
