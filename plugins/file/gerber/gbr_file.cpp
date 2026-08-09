/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbr_file.h"
#include "gbr_node.h"
#include "gbrcomp_item.h"
#include "gbrcomp_onent.h"
#include "gi_datapath.h"
#include "gi_datasolid.h"
#include "graphicsview.h"
#include <QElapsedTimer>
#include <algorithm>
#include <cassert>
#include <forward_list>
#include <gi_dbg.h>
#include <qglobal.h>
#include <utility>

namespace Gerber {

template <typename T>
struct EnumToStr {
    T e;
    template <auto>
    static constexpr std::string_view name() {
        return {__PRETTY_FUNCTION__};
    }

    static constexpr auto text = []<T... Ts> {
        return std::array{name<Ts>()...};
    }(std::integer_sequence<T, T{100}>{});
    constexpr operator std::string_view() const {
        return std::to_underlying(e) < text.size() ? text[std::to_underlying(e)] : std::string_view{};
    }
};

QDebug operator<<(QDebug debug, const State& state) {
    QDebugStateSaver saver{debug};
    debug.nospace() << u"State("_s
                    << u"D0"_s << state.dCode() << u", "_s
                    << u"G0"_s << state.gCode() << u", "_s
                    << u"Positive|Negative"_s.split(u'|').at(state.imgPolarity()) << u", "_s
                    << u"Linear|ClockwiseCircular|CounterClockwiseCircular"_s.split(u'|').at(state.interpolation() - 1) << u", "_s
                    << u"Aperture|Line|Region"_s.split(u'|').at(state.type()) << u", "_s
                    << u"Undef|Single|Multi"_s.split(u'|').at(state.quadrant()) << u", "_s
                    << u"Off|On"_s.split(u'|').at(state.region()) << u", "_s
                    << u"NoMirroring|X_Mirroring|Y_Mirroring|XY_Mirroring"_s.split(u'|').at(state.mirroring()) << u", "_s
                    << u"aperture"_s << state.aperture() << u", "_s
                    << state.curPos() << u", "_s
                    << u"scaling"_s << state.scaling() << u", "_s
                    << u"rotating"_s << state.rotating() << u", "_s
                    << ')';
    return debug;
}

File::File()
    : AbstractFile() {
    itemGroups_.append_range(std::array{new Gi::Group, new Gi::Group});
    layerTypes_ = {
        {Normal,     GbrObj::tr("Normal"),         GbrObj::tr("Normal view")                                                               },
        {ApPaths,    GbrObj::tr("Aperture paths"), GbrObj::tr("Displays only aperture paths of copper\nwithout width and without contacts")},
        {Components, GbrObj::tr("Components"),     GbrObj::tr("Show components")                                                           }
    };
}

File::~File() { }

const ApertureMap* File::apertures() const { return &apertures_; }

std::vector<GraphicObject> File::getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test) const {
    std::vector<GraphicObject> retData;
    auto t = transform_.toQTransform(); // cached  QTransform
    for(auto&& criterion: criterias) {
        for(const GrObject& go: graphicObjects_) {
            auto transformedGo = go * t; // return copy
            if(criterion.test(transformedGo)) {
                auto& g = retData.emplace_back(transformedGo);
                if(test)
                    return retData;

                switch(gcType) {
                case GCType::Drill: {
                    double drillDiameter{};
                    auto& ap = *apertures_.at(go.state.aperture());

                    auto name{ap.name()};
                    if(ap.withHole())
                        drillDiameter = ap.drillDiameter();
                    else
                        drillDiameter = ap.minSize();
                    drillDiameter *= std::min(transform_.scale.x(), transform_.scale.y());
                    name += QObject::tr(", drill Ø%1mm").arg(drillDiameter);
                    g.raw = drillDiameter;
                } break;
                default: break;
                }
            }
        }
    }

    return retData;
}

Geo::Polygons File::merge() const {
    Timer t;

    // Слой -- последовательность вспышек: положительные добавляют медь,
    // отрицательные её вычитают. Идущие подряд с одинаковой полярностью
    // собираются в одну пачку -- по булевой операции на пачку, а не на вспышку.
    constexpr auto samePolarity = +[](const GrObject& l, const GrObject& r) {
        return l.state.imgPolarity() == r.state.imgPolarity();
    };

#if 0 // FIXME
  auto gbgImage = [](QPainterPath&& path, QString&& out, bool hole) {
        QRectF box = path.boundingRect();
        if(box.isEmpty()) {
            qWarning() << "empty path";
            return;
        }
        const double margin = std::max(box.width(), box.height()) * 0.05;
        box.adjust(-margin, -margin, margin, margin);

        const int side     = qEnvironmentVariableIntValue("DUMP_SIDE") ?: 1200;
        const double scale = side / std::max(box.width(), box.height());

        QImage image{QSize(int(box.width() * scale), int(box.height() * scale)), QImage::Format_RGB32};
        image.fill(Qt::white);
        QPainter painter{&image};
        painter.setRenderHint(QPainter::Antialiasing);
        // Gerber Y goes up, raster Y goes down.
        painter.translate(0, image.height());
        painter.scale(scale, -scale);
        painter.translate(-box.left(), -box.top());
        painter.setPen(Qt::NoPen);
        painter.setBrush(hole ? QColor(0, 0, 120) : QColor(0, 120, 0));
        painter.drawPath(path);
        painter.end();

        out = out + u".png"_s;
        image.save(out);
    };
#endif

    mergedCurves_ = {};
    for(int i{}; auto&& gObjects: v::chunk_by(graphicObjects_, samePolarity)) {
        // Пачка собирается ГОТОВЫМИ ПОЛИГОНАМИ, а не плоским списком контуров:
        // union всё так же идёт разом, в несколько потоков и с пространственной
        // сортировкой (вспышек в слое десятки тысяч, и цепочка из стольких же
        // точных операций была бы неподъёмной), но вложенность каждого тела
        // остаётся при нём.
        //
        // Плоский список её терял: там вложенность выражена одной ориентацией
        // контура, и дырка одного объекта вычитала чужой объект, лежащий внутри
        // неё. Кольцевой штрих с окружностями внутри (репер на плате) так и
        // пропадал целиком -- дырка внешнего кольца съедала оба вложенных.

#if 0 // FIXME
        std::vector<Geo::Polygon> parts;
        for(int j{}; const GrObject& go: gObjects) {
            gbgImage(go.fill.toPath(), u"%1_3_go%2"_s.arg(i).arg(j), gObjects.front().state.imgPolarity());
            if(qEnvironmentVariableIsSet("GBR_DEBUG_MERGE")) {
                std::size_t holes{};
                for(const Geo::Polygon& p: go.fill.all()) holes += p.holes().size();
                qInfo().noquote() << u"%1_3_go%2"_s.arg(i).arg(j) << go.name
                                  << u"id"_s << go.id
                                  << u"ap"_s << go.state.aperture()
                                  << u"pol"_s << go.state.imgPolarity()
                                  << u"pos"_s << go.state.curPos()
                                  << u"n"_s << go.fill.size()
                                  << u"holes"_s << holes
                                  << u"bbox"_s << go.fill.boundingRect()
                                  << u"path.size"_s << go.path.size()
                                  << u"path.closed"_s << go.path.closed
                                  << u"path.width"_s << go.path.width;
                if(qEnvironmentVariableIsSet("GBR_DEBUG_MERGE_PATH")) {
                    QStringList sel = QString::fromLocal8Bit(qgetenv("GBR_DEBUG_MERGE_PATH")).split(u',');
                    if(sel.contains(u"%1_3_go%2"_s.arg(i).arg(j)))
                        for(const Geo::Vertex& v: go.path)
                            qInfo().noquote() << u"   "_s << v.x() << v.y() << u"b"_s << v.bulge;
                }
            }
            ++j;
            // if((i < 2) | go.fill.all().front().holes().size())
            parts.append_range(go.fill.all());
        }

        const Geo::Polygons part{std::span<const Geo::Polygon>{parts}};
#else
        // std::vector<Geo::Polygon> parts;
        // for(const GrObject& go: gObjects)
        //     parts.append_range(go.fill.all());

        std::vector parts{std::from_range,
            gObjects
                | v::transform(&GrObject::fill)
                | v::transform(&Geo::Polygons::all)
                | v::join};

#endif
        QElapsedTimer chunkTimer;
        if(qEnvironmentVariableIsSet("GBR_DEBUG_MERGE_TIME")) chunkTimer.start();

        const Geo::Polygons part{std::span<const Geo::Polygon>{parts}};

        if(qEnvironmentVariableIsSet("GBR_DEBUG_MERGE_TIME"))
            qInfo() << "chunk" << i << "objs" << gObjects.size() << "parts" << parts.size()
                    << "buildPart(ms)" << chunkTimer.restart();

        // gbgImage(mergedCurves_.toPath(), u"%1_0_merged"_s.arg(i), gObjects.front().state.imgPolarity());

        // gbgImage(part.toPath(), u"%1_1_part"_s.arg(i), gObjects.front().state.imgPolarity());

        if(gObjects.front().state.imgPolarity() == Positive)
            mergedCurves_ |= part;
        else
            mergedCurves_ -= part;

        if(qEnvironmentVariableIsSet("GBR_DEBUG_MERGE_TIME"))
            qInfo() << "chunk" << i << "union(ms)" << chunkTimer.elapsed();

        // gbgImage(mergedCurves_.toPath(), u"%1_2_merged"_s.arg(i), gObjects.front().state.imgPolarity());

        ++i;
    }

    return mergedCurves_;
}

const QList<Comp::Component>& File::components() const { return components_; }

// Разбор вложенности вручную больше не нужен: Geo::Polygons и ЕСТЬ разобранный
// регион -- его собственные полигоны это медь, а полигоны того, что осталось от
// габаритной рамки после вычитания меди, -- вырезы. Прежде то же самое
// получалось объединением с рамкой по NonZero и обходом PolyTree.
Geo::Polygons& File::groupedPaths(File::Group group, bool /*fl*/) {
    if(!groupedCurves_.empty() && group_ == group)
        return groupedCurves_;

    group_ = group;
    const Geo::Polygons region = mergedCurves();

    if(group == CopperGroup) {
        groupedCurves_ = region;
    } else {
        // Поле рамки вокруг детали: миллиметр с каждой стороны, лишь бы вырезы
        // оказались внутри неё и отделились от бесконечности.
        constexpr double margin = 1.0;
        QRectF box = region.boundingRect();
        box.adjust(-margin, -margin, margin, margin);
        const Geo::Polygons frame{Geo::Polylines{Geo::rectangle(box.width(), box.height(), box.center())}};
        groupedCurves_ = frame - region;
    }

    return groupedCurves_;
}

bool File::flashedApertures() const {
    for(const auto& [_, aperture]: apertures_)
        if(aperture->flashed())
            return true;
    return false;
}

void File::setColor(const QColor& color) {
    color_ = color;
    itemGroups_[Normal]->setBrushColor(color_);
    itemGroups_[ApPaths]->setPen(QPen(color_, 0.0));
}

std::vector<const ::GraphicObject*> File::graphicObjects() const {
    std::vector<const ::GraphicObject*> go(graphicObjects_.size());
    size_t i{};
    for(auto& refGo: graphicObjects_)
        go[i++] = &refGo;
    return go;
}

void File::initFrom(AbstractFile* file_) {
    AbstractFile::initFrom(file_);
    static_cast<Node*>(node_)->file = this;
}

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Node{this};
}

QIcon File::icon() const {
    switch(itemsType_) {
    case File::ApPaths   : return decoration(color_, u'A');
    case File::Components: return decoration(color_, u'C');
    default              : return decoration(color_);
    }
}

void File::setItemType(int type) {
    if(itemsType_ == type)
        return;

    itemsType_ = type;

    itemGroups_[Normal]->setVisible(false);
    itemGroups_[ApPaths]->setVisible(false);
    itemGroups_[Components]->setVisible(false);

    itemGroups_[itemsType_]->setVisible(true /*visible_*/);
}

int File::itemsType() const { return itemsType_; }

void File::createGi() {

    if constexpr(1) { // fill copper
        for(const Geo::Polygon& paths: groupedPaths()) {
            // Gi::Debug(paths);
            Gi::Item* item = new Gi::DataFill{Geo::Polygons{paths}, this};
            itemGroups_[Normal]->push_back(item);
        }
        itemGroups_[Normal]->shrink_to_fit();
    }
    if constexpr(1) { // add components
        for(const Comp::Component& component: std::as_const(components_))
            if(!component.referencePoint().isNull())
                itemGroups_[Components]->push_back(new Comp::Item{component, this});
        itemGroups_[Components]->shrink_to_fit();
    }
    if constexpr(1) { // add aperture paths
        auto contains = [&](const Geo::Polyline& path) -> bool {
            constexpr double k = 0.001;
            for(const auto& chPath: checkList) { // find copy
                size_t counter{};
                if(chPath.size() == path.size()) {
                    for(const auto& p1: chPath) {
                        for(const auto& p2: path) {
                            if((abs(p1.x() - p2.x()) < k) && (abs(p1.y() - p2.y()) < k)) {
                                ++counter;
                                break;
                            }
                        }
                    }
                }
                if(counter == path.size())
                    return true;
            }
            return false;
        };

        // Ветка упрощения регионов ушла: SimplifyPolygon был заглушкой и
        // наполнял пустой список, то есть контуры замкнутых регионов просто
        // терялись. Теперь они добавляются наравне с прочими.
        for(const GrObject& go: graphicObjects_) {
            if(go.path.empty()) continue;
            if(!Settings::skipDuplicates()) {
                itemGroups_[ApPaths]->push_back(new Gi::DataPath{{go.path}, this});
            } else if(!contains(go.path)) {
                itemGroups_[ApPaths]->push_back(new Gi::DataPath{{go.path}, this});
                checkList.push_front(go.path);
            }
        }

        itemGroups_[ApPaths]->shrink_to_fit();
    }

    bool zeroLine{};
    for(auto& [dCode, ap]: apertures_)
        if(zeroLine = (qFuzzyIsNull(ap->minSize()) && ap->used()); zeroLine)
            break;

    if(itemsType_ == NullType) {
        if /**/ (itemGroups_[Components]->size())
            itemsType_ = Components;
        else if(itemGroups_[Normal]->size()) // && !zeroLine)
            itemsType_ = Normal;
        else
            itemsType_ = ApPaths;
    }

    setColor(color_);

    layerTypes_[Normal].id = itemGroups_[Normal]->size() ? Normal : NullType;
    layerTypes_[ApPaths].id = itemGroups_[ApPaths]->size() ? ApPaths : NullType;
    layerTypes_[Components].id = itemGroups_[Components]->size() ? Components : NullType;

    itemGroups_[ApPaths]->setVisible(false);
    itemGroups_[Components]->setVisible(false);
    itemGroups_[Normal]->setVisible(false);

    itemGroups_[itemsType_]->setVisible(visible_);
}

} // namespace Gerber
