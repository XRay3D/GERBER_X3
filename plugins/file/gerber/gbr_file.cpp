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

Geo::Polygon File::merge() const {
    Timer t;
    Paths64 mergedPaths;

    constexpr auto samePolarity = +[](const GrObject& l, const GrObject& r) {
        return l.state.imgPolarity() == r.state.imgPolarity();
    };
    constexpr std::array CT{cl::ClipType::Union, cl::ClipType::Difference};
    constexpr std::array FR{cl::FillRule::Positive, cl::FillRule::NonZero};
#if DEBUG && 0
    for(auto&& gObjects: v::chunk_by(graphicObjects_, samePolarity) | v::reverse) {
        Paths64 clip{
            std::from_range,
            v::join(v::transform(gObjects, &GrObject::fill)
                // | v::counted(208 - 99 - 5, 5)
                // | v::take(5)),
                | v::drop(208 - 99 - 5) | v::take(5)),
        };
        bool fl = gObjects.front().state.imgPolarity();
        mergedPaths = CL2::BooleanOp(CT[fl], FR[fl], mergedPaths, clip);
        break;
    }
#else
    for(auto&& gObjects: v::chunk_by(graphicObjects_, samePolarity)) {
        Paths64 clip{
            std::from_range,
            gObjects
                | v::transform(&GrObject::fill)
                | v::transform(qOverload<const Geo::Polygon&>(toPaths))
                | v::join,
        };
        bool fl = gObjects.front().state.imgPolarity();
        mergedPaths = cl::BooleanOp(CT[fl], FR[fl], mergedPaths, clip);
    }
    // Gi::Debug(mergedPaths, {255, 255, 255, 128}); //->arrows = {};
#endif
    if(Settings::cleanPolygons())
        CleanPaths(mergedPaths, Settings::cleanPolygonsDist() * uScale);

    for(Path64& path: mergedPaths) // close paths
        path.emplace_back(path.front());

    return mergedCurves_ = toCurves(mergedPaths);
}

const QList<Comp::Component>& File::components() const { return components_; }

void File::grouping(PolyTree& node, Geo::Polygons* curvess) {
    // Узел вместе со своими прямыми потомками образует одну группу
    // «контур + его дырки»; какой из уровней считать контуром, задаёт group_.
    auto collect = [curvess](PolyTree& node) {
        Paths64 paths{node.Polygon()};
        for(size_t i{}; i < node.Count(); ++i)
            paths.push_back(node[i]->Polygon());
        for(Path64& path: paths) // close paths
            path.emplace_back(path.front());
        curvess->push_back(toCurves(paths));
    };

    switch(group_) {
    case CutoffGroup:
        if(!node.IsHole()) collect(node);
        break;
    case CopperGroup:
        if(node.IsHole()) collect(node);
        break;
    }
    for(size_t i{}; i < node.Count(); ++i)
        grouping(*node[i], curvess);
}

Geo::Polygons& File::groupedPaths(File::Group group, bool fl) {
    if(groupedCurves_.empty()) {
        PolyTree polyTree;
        cl::Clipper64 clipper;
        auto paths = toPaths(mergedCurves());
        clipper.AddSubject(paths);
        auto r{GetBounds(paths)};
        int k = uScale;
        Path64 outer = {
            Point64(r.left - k, r.bottom + k),
            Point64(r.right + k, r.bottom + k),
            Point64(r.right + k, r.top - k),
            Point64(r.left - k, r.top - k)};
        if(fl)
            ReversePath(outer);
        clipper.AddSubject({outer});
        clipper.Execute(cl::ClipType::Union, cl::FillRule::NonZero, polyTree);
        group_ = group;
        grouping(polyTree, &groupedCurves_);
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

void File::write(QDataStream& stream) const {
    ::Block{stream}.write(
        graphicObjects_,
        apertures_,
        format_,
        rawIndex,
        itemsType_,
        components_);
}

void File::read(QDataStream& stream) {
    crutch = this; // NOTE
    ::Block{stream}.read(
        graphicObjects_,
        apertures_,
        format_,
        rawIndex,
        itemsType_,
        components_);

    for(GrObject& go: graphicObjects_) {
        go.gFile = this;
        go.state.file_ = this;
    }
}

void File::createGi() {
    if constexpr(1) { // fill copper
        for(Geo::Polygon& paths: groupedPaths()) {
            // Gi::Debug(paths);
            Gi::Item* item = new Gi::DataFill{paths, this};
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
            constexpr double k = 0.001 * uScale;
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

        for(const GrObject& go: graphicObjects_) {
            if(!go.path.empty()) {
                if(Settings::simplifyRegions() && go.path.front() == go.path.back()) {
                    Geo::Polygon paths;
                    // FIXME SimplifyPolygon(toPath(go.path), paths);
                    for(auto&& path: paths) {
                        path.push_back(path.front());
                        if(!Settings::skipDuplicates()) {
                            checkList.push_front(path);
                            itemGroups_[ApPaths]->push_back(new Gi::DataPath{{checkList.front()}, this});
                        } else if(!contains(path)) {
                            checkList.push_front(path);
                            itemGroups_[ApPaths]->push_back(new Gi::DataPath{{checkList.front()}, this});
                        }
                    }
                } else if(!Settings::skipDuplicates()) {
                    itemGroups_[ApPaths]->push_back(new Gi::DataPath{{go.path}, this});
                } else if(!contains(go.path)) {
                    itemGroups_[ApPaths]->push_back(new Gi::DataPath{{go.path}, this});
                    checkList.push_front(go.path);
                }
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
