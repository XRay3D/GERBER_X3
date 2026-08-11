/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
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

// Соответствие Entity::Type -> конкретный класс и аргументы его конструктора.
// Список ОДИН на обе стороны -- фабрику createEntity и диспетчер записи в
// Serial::Adapter: два независимых switch'а на 44 ветки разъезжались бы молча,
// и сущность нового типа сохранялась бы как чужая.
#define DXF_ENTITIES(X)                   \
    X(ACAD_PROXY_ENTITY, Dummy, (sp))     \
    X(ARC, Arc, (sp))                     \
    X(ATTDEF, AttDef, (sp))               \
    X(ATTRIB, Attrib, (sp))               \
    X(BODY, Body, (sp))                   \
    X(CIRCLE, Circle, (sp))               \
    X(DIMENSION, Dimension, (sp))         \
    X(ELLIPSE, Ellipse, (sp))             \
    X(FACE3D, Face3D, (sp))               \
    X(HATCH, Hatch, (sp))                 \
    X(HELIX, Helix, (sp))                 \
    X(IMAGE, Image, (sp))                 \
    X(INSERT, InsertEntity, (blocks, sp)) \
    X(LEADER, Leader, (sp))               \
    X(LIGHT, Light, (sp))                 \
    X(LINE, Line, (sp))                   \
    X(LWPOLYLINE, LwPolyline, (sp))       \
    X(MESH, Mesh, (sp))                   \
    X(MLEADER, MLeader, (sp))             \
    X(MLEADERSTYLE, MLeaderStyle, (sp))   \
    X(MLINE, MLine, (sp))                 \
    X(MTEXT, MText, (sp))                 \
    X(OLE2FRAME, Ole2Frame, (sp))         \
    X(OLEFRAME, OleFrame, (sp))           \
    X(POINT, Point, (sp))                 \
    X(POLYLINE, PolyLine, (sp))           \
    X(RAY, Ray, (sp))                     \
    X(REGION, Region, (sp))               \
    X(SECTION, Section, (sp))             \
    X(SEQEND, SeqEnd, (sp))               \
    X(SHAPE, AbstractShape, (sp))         \
    X(SOLID, Solid, (sp))                 \
    X(SPLINE, Spline, (sp))               \
    X(SUN, Sun, (sp))                     \
    X(SURFACE, Surface, (sp))             \
    X(TABLE, Table, (sp))                 \
    X(TEXT, Text, (sp))                   \
    X(TOLERANCE, Tolerance, (sp))         \
    X(TRACE, Trace, (sp))                 \
    X(UNDERLAY, Underlay, (sp))           \
    X(VERTEX, Vertex, (sp))               \
    X(VIEWPORT, Viewport, (sp))           \
    X(WIPEOUT, Wipeout, (sp))             \
    X(XLINE, XLine, (sp))

std::shared_ptr<Entity> createEntity(Entity::Type key, Blocks& blocks [[maybe_unused]], SectionParser* sp) {
    switch(key) {
#define X(TYPE, CLASS, ARGS) \
    case Entity::TYPE: return std::make_shared<CLASS> ARGS;
        DXF_ENTITIES(X)
#undef X
    default: return std::make_shared<Dummy>(sp);
    }
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

Entity::Type Entity::toType(const QString& key) {
    if(key.compare(u"3DFACE"_s, Qt::CaseInsensitive) == 0) // "3DFACE" не может быть ключом Q_ENUM, т.к. начинается с цифры
        return Type::FACE3D;
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

void Serial::Adapter<std::shared_ptr<Dxf::Entity>>::write(
    Writer& sb, const std::shared_ptr<Dxf::Entity>& entity) {
    using namespace Dxf;
    sb.start_object();
    sb.append_raw("\"type\":");
    sb.append(std::to_underlying(entity->type()));
    auto dispatch = [&sb]<typename T>(const T& e) { Serial::writeInto(sb, e); };
    switch(entity->type()) {
#define X(TYPE, CLASS, ARGS) \
    case Entity::TYPE: dispatch(static_cast<const CLASS&>(*entity)); break;
        DXF_ENTITIES(X)
#undef X
    default: dispatch(static_cast<const Dummy&>(*entity)); break;
    }
    sb.end_object();
}

simdjson::error_code Serial::Adapter<std::shared_ptr<Dxf::Entity>>::read(
    simdjson::ondemand::value& val, std::shared_ptr<Dxf::Entity>& entity) {
    using namespace Dxf;
    std::string_view slice; // сырой текст элемента: тип подсмотреть + поля прочесть
    if(auto err = simdjson::to_json_string(val).get(slice); err) return err;
    Serial::Parsed peek{slice};
    simdjson::ondemand::object obj;
    if(peek.error || peek.doc.get_object().get(obj)) return simdjson::INCORRECT_TYPE;
    int64_t type{};
    if(auto err = obj["type"].get_int64().get(type); err) return err;
    // Блоки нужны конструктору INSERT, но после загрузки проекта повторного
    // разбора нет -- сущности живут только как подписи к готовой геометрии.
    static Blocks blocks;
    entity = createEntity(Entity::Type(type), blocks, nullptr);
    auto dispatch = [&slice]<typename T>(T& e) { Serial::loadInto(slice, e); };
    switch(entity->type()) {
#define X(TYPE, CLASS, ARGS) \
    case Entity::TYPE: dispatch(static_cast<CLASS&>(*entity)); break;
        DXF_ENTITIES(X)
#undef X
    default: dispatch(static_cast<Dummy&>(*entity)); break;
    }
    return simdjson::SUCCESS;
}

#include "moc_dxf_entity.cpp"
