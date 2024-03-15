// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

namespace Gerber {

QDebug operator<<(QDebug debug, const State& state) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "State("
                    << "D0" << state.dCode() << ", "
                    << "G0" << state.gCode() << ", "
                    << u"Positive|Negative"_qs.split('|').at(state.imgPolarity()) << ", "
                    << u"Linear|ClockwiseCircular|CounterclockwiseCircular"_qs.split('|').at(state.interpolation() - 1) << ", "
                    << u"Aperture|Line|Region"_qs.split('|').at(state.type()) << ", "
                    << u"Undef|Single|Multi"_qs.split('|').at(state.quadrant()) << ", "
                    << u"Off|On"_qs.split('|').at(state.region()) << ", "
                    << u"NoMirroring|X_Mirroring|Y_Mirroring|XY_Mirroring"_qs.split('|').at(state.mirroring()) << ", "
                    << u"aperture"_qs << state.aperture() << ", "
                    << state.curPos() << ", "
                    << u"scaling"_qs << state.scaling() << ", "
                    << u"rotating"_qs << state.rotating() << ", "
                    << ')';
    return debug;
}

File::File()
    : AbstractFile() {
    itemGroups_.append({new Gi::Group, new Gi::Group});
    layerTypes_ = {
        {    Normal,         GbrObj::tr("Normal"),                                                                GbrObj::tr("Normal view")},
        {   ApPaths, GbrObj::tr("Aperture paths"), GbrObj::tr("Displays only aperture paths of copper\nwithout width and without contacts")},
        {Components,     GbrObj::tr("Components"),                                                            GbrObj::tr("Show components")}
    };
}

File::~File() { }

const ApertureMap* File::apertures() const { return &apertures_; }

mvector<GraphicObject> File::getDataForGC(std::span<Criteria> criterias, GCType gcType, bool test) const {
    mvector<GraphicObject> retData;
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
                default:
                    break;
                }
            }
        }
    }

    return retData;
}

Paths File::merge() const {
    QElapsedTimer t;
    t.start();
    mergedPaths_.clear();
    size_t i = 0;

    if constexpr(0) { // FIXME fill closed line
        std::list<Paths> pathList;
        {
            std::list<std::map<int, Paths>> pathListMap;
            int exp = -1;
            for(auto& go: graphicObjects_) {
                if(exp != go.state.imgPolarity()) {
                    exp = go.state.imgPolarity();
                    pathListMap.resize(pathListMap.size() + 1);
                }
                if(go.state.type() == Line) {
                    auto& paths = pathListMap.back();
                    paths[go.state.aperture()].push_back(go.path);
                }
            }
            for(auto& map: pathListMap) {
                pathList.resize(pathList.size() + 1);
                for(auto& [aperture, paths]: map) {
                    mergePaths(paths);
                    Clipper2Lib::ClipperOffset offset;
                    for(int i{}; i < paths.size(); ++i) {
                        auto& path = paths[i];
                        if(path.back() == path.front()) {
                            offset.AddPath(paths[i], JoinType::Round, EndType::Polygon);
                            paths.erase(paths.begin() + i--);
                        }
                    }
                    offset.AddPaths(paths, JoinType::Round, EndType::Round);
                    offset.Execute(apertures_.at(aperture)->apSize() * uScale * 0.5, paths);
                    // pathList.back().append(std::move(paths));
                    pathList.back() += paths; // NOTE maybe move
                }
            }
        }

        while(i < graphicObjects_.size()) {
            Clipper clipper;
            clipper.AddSubject(mergedPaths_);
            const auto exp = graphicObjects_.at(i).state.imgPolarity();
            do {
                if(graphicObjects_[i].state.type() == Line) {
                    ++i;
                } else {
                    const GrObject& go = graphicObjects_.at(i++);
                    clipper.AddClip(go.fill);
                }
            } while(i < graphicObjects_.size() && exp == graphicObjects_.at(i).state.imgPolarity());

            if(exp)
                ReversePaths(pathList.front());
            clipper.AddClip(pathList.front());
            pathList.pop_front();

            if(graphicObjects_.at(i - 1).state.imgPolarity() == Positive)
                clipper.Execute(ClipType::Union, FillRule::Positive, mergedPaths_);
            else
                clipper.Execute(ClipType::Difference, FillRule::NonZero, mergedPaths_);
        }
    } else {
        while(i < graphicObjects_.size()) {
            Clipper clipper;

            clipper.AddSubject(mergedPaths_);
            const auto exp = graphicObjects_.at(i).state.imgPolarity();
            do {
                const GrObject& go = graphicObjects_[i++];
                clipper.AddClip(go.fill);
            } while(i < graphicObjects_.size() && exp == graphicObjects_[i].state.imgPolarity());
            if(graphicObjects_.at(i - 1).state.imgPolarity() == Positive)
                clipper.Execute(ClipType::Union, FillRule::Positive, mergedPaths_);
            else
                clipper.Execute(ClipType::Difference, FillRule::NonZero, mergedPaths_);
        }
    }

    if(Settings::cleanPolygons())
        CleanPaths(mergedPaths_, Settings::cleanPolygonsDist() * uScale);
    return mergedPaths_;
}

const QList<Comp::Component>& File::components() const { return components_; }

void File::grouping(PolyTree& node, Pathss* pathss) {
    Path path;
    Paths paths;
    switch(group_) {
    case CutoffGroup:
        if(!node.IsHole()) {
            path = node.Polygon();
            paths.push_back(path);
            for(size_t i = 0; i < node.Count(); ++i) {
                path = node[i]->Polygon();
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for(size_t i = 0; i < node.Count(); ++i)
            grouping(*node[i], pathss);

        break;
    case CopperGroup:
        if(node.IsHole()) {
            path = node.Polygon();
            paths.push_back(path);
            for(size_t i = 0; i < node.Count(); ++i) {
                path = node[i]->Polygon();
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for(size_t i = 0; i < node.Count(); ++i)
            grouping(*node[i], pathss);

        break;
    }
}

Pathss& File::groupedPaths(File::Group group, bool fl) {
    if(groupedPaths_.empty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddSubject(mergedPaths());
        auto r{GetBounds(mergedPaths())};
        int k = uScale;
        Path outer = {
            Point(r.left - k, r.bottom + k),
            Point(r.right + k, r.bottom + k),
            Point(r.right + k, r.top - k),
            Point(r.left - k, r.top - k)};
        if(fl)
            ReversePath(outer);
        clipper.AddSubject({outer});
        clipper.Execute(ClipType::Union, FillRule::NonZero, polyTree);
        group_ = group;
        grouping(polyTree, &groupedPaths_);
    }
    return groupedPaths_;
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

mvector<const ::GraphicObject*> File::graphicObjects() const {
    mvector<const ::GraphicObject*> go(graphicObjects_.size());
    size_t i = 0;
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
    case File::ApPaths:
        return decoration(color_, 'A');
    case File::Components:
        return decoration(color_, 'C');
    default:
        return decoration(color_);
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
    ::Block(stream).write(
        graphicObjects_,
        apertures_,
        format_,
        rawIndex,
        itemsType_,
        components_);
}

void File::read(QDataStream& stream) {
    crutch = this; // NOTE
    ::Block(stream).read(
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
        for(Paths& paths: groupedPaths()) {
            Gi::Item* item = new Gi::DataFill{paths, this};
            itemGroups_[Normal]->push_back(item);
        }
        itemGroups_[Normal]->shrink_to_fit();
    }
    if constexpr(1) { // add components
        for(const Comp::Component& component: qAsConst(components_))
            if(!component.referencePoint().isNull())
                itemGroups_[Components]->push_back(new Comp::Item{component, this});
        itemGroups_[Components]->shrink_to_fit();
    }
    if constexpr(1) { // add aperture paths
        auto contains = [&](const Path& path) -> bool {
            constexpr double k = 0.001 * uScale;
            for(const Path& chPath: checkList) { // find copy
                size_t counter = 0;
                if(chPath.size() == path.size()) {
                    for(const Point& p1: chPath) {
                        for(const Point& p2: path) {
                            if((abs(p1.x - p2.x) < k) && (abs(p1.y - p2.y) < k)) {
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
                    Paths paths;
                    SimplifyPolygon(go.path, paths);
                    for(Path& path: paths) {
                        path.push_back(path.front());
                        if(!Settings::skipDuplicates()) {
                            checkList.push_front(path);
                            itemGroups_[ApPaths]->push_back(new Gi::DataPath{checkList.front(), this});
                        } else if(!contains(path)) {
                            checkList.push_front(path);
                            itemGroups_[ApPaths]->push_back(new Gi::DataPath{checkList.front(), this});
                        }
                    }
                } else if(!Settings::skipDuplicates()) {
                    itemGroups_[ApPaths]->push_back(new Gi::DataPath{go.path, this});
                } else if(!contains(go.path)) {
                    itemGroups_[ApPaths]->push_back(new Gi::DataPath{go.path, this});
                    checkList.push_front(go.path);
                }
            }
        }
        itemGroups_[ApPaths]->shrink_to_fit();
    }

    bool zeroLine = false;
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
