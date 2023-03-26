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
#include "gbrcomp_item.h"
#include "gbrcomp_onent.h"

#include "ft_node.h"
#include "gi_datapath.h"
#include "gi_datasolid.h"
#include "myclipper.h"
#include "project.h"
#include "settings.h"

#include <QElapsedTimer>
#include <QSemaphore>
#include <QThread>

#include "gbr_node.h"

namespace Gerber {

Path GraphicObject::elipse() const { return (state_.dCode() == D03
                                                && gFile_->apertures()->at(state_.aperture())->type() == ApertureType::Circle) ?
        path_ :
        Path(); } // circle
Paths GraphicObject::elipseW() const { return (state_.dCode() == D03
                                                  && gFile_->apertures()->at(state_.aperture())->type() == ApertureType::Circle
                                                  && gFile_->apertures()->at(state_.aperture())->withHole()) ?
        paths_ :
        Paths(); }

QDebug operator<<(QDebug debug, const State& state) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "State("
                    << "D0" << state.dCode() << ", "
                    << "G0" << state.gCode() << ", "
                    << QStringLiteral("Positive|Negative").split('|').at(state.imgPolarity()) << ", "
                    << QStringLiteral("Linear|ClockwiseCircular|CounterclockwiseCircular").split('|').at(state.interpolation() - 1) << ", "
                    << QStringLiteral("Aperture|Line|Region").split('|').at(state.type()) << ", "
                    << QStringLiteral("Undef|Single|Multi").split('|').at(state.quadrant()) << ", "
                    << QStringLiteral("Off|On").split('|').at(state.region()) << ", "
                    << QStringLiteral("NoMirroring|X_Mirroring|Y_Mirroring|XY_Mirroring").split('|').at(state.mirroring()) << ", "
                    << QStringLiteral("aperture") << state.aperture() << ", "
                    << state.curPos() << ", "
                    << QStringLiteral("scaling") << state.scaling() << ", "
                    << QStringLiteral("rotating") << state.rotating() << ", "
                    << ')';
    return debug;
}

File::File(const QString& fileName)
    : FileInterface() {
    itemGroups_.append({new GiGroup, new GiGroup});
    name_ = fileName;
    layerTypes_ = {
        {    Normal,         GbrObj::tr("Normal"),
         GbrObj::tr("Normal view")                                                               },
        {   ApPaths, GbrObj::tr("Aperture paths"),
         GbrObj::tr("Displays only aperture paths of copper\nwithout width and without contacts")},
        {Components,     GbrObj::tr("Components"),
         GbrObj::tr("Show components")                                                           }
    };
}

File::~File() { }

const ApertureMap* File::apertures() const { return &apertures_; }

Paths File::merge() const {
    QElapsedTimer t;
    t.start();
    mergedPaths_.clear();
    size_t i = 0;

    if constexpr (0) { // FIXME fill closed line
        std::list<Paths> pathList;
        {
            std::list<std::map<int, Paths>> pathListMap; // FIXME V826. Consider replacing standard container with a different one. to vector
            int exp = -1;
            for (auto& go : graphicObjects_) {
                if (exp != go.state().imgPolarity()) {
                    exp = go.state().imgPolarity();
                    pathListMap.resize(pathListMap.size() + 1);
                }
                if (go.state().type() == Line) {
                    auto& paths = pathListMap.back();
                    paths[go.state().aperture()].push_back(go.path());
                }
            }
            for (auto& map : pathListMap) {
                pathList.resize(pathList.size() + 1);
                for (auto& [aperture, paths] : map) {
                    mergePaths(paths);
                    ClipperOffset offset;
                    for (int i {}; i < paths.size(); ++i) {
                        auto& path = paths[i];
                        if (path.back() == path.front()) {
                            offset.AddPath(paths[i], JoinType::Round, EndType::Polygon);
                            paths.erase(paths.begin() + i--);
                        }
                    }
                    offset.AddPaths(paths, JoinType::Round, EndType::Round);
                    paths = offset.Execute(apertures_.at(aperture)->apSize() * uScale * 0.5);
                    // pathList.back().append(std::move(paths));
                    pathList.back().append(paths);
                }
            }
        }

        while (i < graphicObjects_.size()) {
            Clipper clipper;
            clipper.AddSubject(mergedPaths_);
            const auto exp = graphicObjects_.at(i).state().imgPolarity(); // FIXME V831 Decreased performance. Consider replacing the call to the 'at()' method with the 'operator[]'. gbr_file.cpp 122
            do {
                if (graphicObjects_[i].state().type() == Line) {
                    ++i;
                } else {
                    const GraphicObject& go = graphicObjects_.at(i++);
                    clipper.AddClip(go.paths());
                }
            } while (i < graphicObjects_.size() && exp == graphicObjects_.at(i).state().imgPolarity()); // FIXME V831 Decreased performance. Consider replacing the call to the 'at()' method with the 'operator[]'. gbr_file.cpp 122

            if (exp)
                ReversePaths(pathList.front());
            clipper.AddClip(pathList.front());
            pathList.pop_front();

            if (graphicObjects_.at(i - 1).state().imgPolarity() == Positive)
                clipper.Execute(ClipType::Union, FillRule::Positive, mergedPaths_);
            else
                clipper.Execute(ClipType::Difference, FillRule::NonZero, mergedPaths_);
        }
    } else {
        while (i < graphicObjects_.size()) {
            Clipper clipper;
            clipper.AddSubject(mergedPaths_);
            const auto exp = graphicObjects_.at(i).state().imgPolarity();
            do {
                const GraphicObject& go = graphicObjects_.at(i++);
                clipper.AddClip(go.paths());
            } while (i < graphicObjects_.size() && exp == graphicObjects_.at(i).state().imgPolarity()); // FIXME V831 Decreased performance. Consider replacing the call to the 'at()' method with the 'operator[]'. gbr_file.cpp 122
            if (graphicObjects_.at(i - 1).state().imgPolarity() == Positive)
                clipper.Execute(ClipType::Union, FillRule::Positive, mergedPaths_);
            else
                clipper.Execute(ClipType::Difference, FillRule::NonZero, mergedPaths_);
        }
    }

    if (Settings::cleanPolygons())
        CleanPaths(mergedPaths_, Settings::cleanPolygonsDist() * uScale);
    return mergedPaths_;
}

const QList<Comp::Component>& File::components() const { return components_; }

void File::grouping(PolyTree& node, Pathss* pathss) {
    Path path;
    Paths paths;
    switch (group_) {
    case CutoffGroup:
        if (!node.IsHole()) {
            path = node.Polygon();
            paths.push_back(path);
            for (size_t i = 0; i < node.Count(); ++i) {
                path = node[i]->Polygon();
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (size_t i = 0; i < node.Count(); ++i)
            grouping(*node[i], pathss);

        break;
    case CopperGroup:
        if (node.IsHole()) {
            path = node.Polygon();
            paths.push_back(path);
            for (size_t i = 0; i < node.Count(); ++i) {
                path = node[i]->Polygon();
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (size_t i = 0; i < node.Count(); ++i)
            grouping(*node[i], pathss);

        break;
    }
}

Pathss& File::groupedPaths(File::Group group, bool fl) {
    if (groupedPaths_.empty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddSubject(mergedPaths());
        auto r {Bounds(mergedPaths())};
        int k = uScale;
        Path outer = {
            Point(r.left - k, r.bottom + k),
            Point(r.right + k, r.bottom + k),
            Point(r.right + k, r.top - k),
            Point(r.left - k, r.top - k)};
        if (fl)
            ReversePath(outer);
        clipper.AddSubject({outer});
        clipper.Execute(ClipType::Union, FillRule::NonZero, polyTree);
        group_ = group;
        grouping(polyTree, &groupedPaths_);
    }
    return groupedPaths_;
}

bool File::flashedApertures() const {
    for (const auto& [_, aperture] : apertures_)
        if (aperture->flashed())
            return true;
    return false;
}

void File::setColor(const QColor& color) {
    color_ = color;
    itemGroups_[Normal]->setBrushColor(color);
    itemGroups_[ApPaths]->setPen(QPen(color, 0.0));
}

mvector<const AbstrGraphicObject*> File::graphicObjects() const {
    mvector<const AbstrGraphicObject*> go(graphicObjects_.size());
    size_t i = 0;
    for (auto& refGo : graphicObjects_)
        go[i++] = &refGo;
    return go;
}

void File::initFrom(FileInterface* file) {
    FileInterface::initFrom(file);
    static_cast<Node*>(node_)->file = this;

    setColor(file->color());
    // Normal
    itemGroup(Gerber::File::Normal)->setBrushColor(file->itemGroup(Gerber::File::Normal)->brushColor());
    itemGroup(Gerber::File::Normal)->addToScene();
    itemGroup(Gerber::File::Normal)->setZValue(-id());
    // ApPaths
    itemGroup(Gerber::File::ApPaths)->setPen(file->itemGroup(Gerber::File::ApPaths)->pen());
    itemGroup(Gerber::File::ApPaths)->addToScene();
    itemGroup(Gerber::File::ApPaths)->setZValue(-id());
    // Components
    itemGroup(Gerber::File::Components)->addToScene();
    itemGroup(Gerber::File::Components)->setZValue(-id());
    setItemType(file->itemsType());
}

FileTree_::Node* File::node() {
    return node_ ? node_ : node_ = new Node(this);
}

void File::setItemType(int type) {
    if (itemsType_ == type)
        return;

    itemsType_ = type;

    itemGroups_[Normal]->setVisible(false);
    itemGroups_[ApPaths]->setVisible(false);
    itemGroups_[Components]->setVisible(false);

    itemGroups_[itemsType_]->setVisible(true /*visible_*/);
}

int File::itemsType() const { return itemsType_; }

void File::write(QDataStream& stream) const {
    stream << graphicObjects_; // write  QList<GraphicObject>
    stream << apertures_;
    stream << format_;
    // stream << layer;
    // stream << miror;
    stream << rawIndex;
    stream << itemsType_;
    stream << components_;
}

void File::read(QDataStream& stream) {
    crutch = this;             ///////////////////
    stream >> graphicObjects_; // read  QList<GraphicObject>
    stream >> apertures_;
    stream >> format_;
    // stream >> layer;
    // stream >> miror;
    stream >> rawIndex;
    stream >> itemsType_;
    stream >> components_;

    for (GraphicObject& go : graphicObjects_) {
        go.gFile_ = this;
        go.state_.file_ = this;
    }
}

void File::createGi() {
    if constexpr (1) { // fill copper
        for (Paths& paths : groupedPaths()) {
            GraphicsItem* item = new GiDataSolid(paths, this);
            itemGroups_[Normal]->push_back(item);
        }
        itemGroups_[Normal]->shrink_to_fit();
    }
    if constexpr (1) { // add components
        for (const Comp::Component& component : qAsConst(components_)) {
            if (!component.referencePoint().isNull())
                itemGroups_[Components]->push_back(new Comp::Item(component, this));
        }
        itemGroups_[Components]->shrink_to_fit();
    }
    if constexpr (1) { // add aperture paths
        auto contains = [&](const Path& path) -> bool {
            constexpr double k = 0.001 * uScale;
            for (const Path& chPath : checkList) { // find copy
                size_t counter = 0;
                if (chPath.size() == path.size()) {
                    for (const Point& p1 : chPath) {
                        for (const Point& p2 : path) {
                            if ((abs(p1.x - p2.x) < k) && (abs(p1.y - p2.y) < k)) {
                                ++counter;
                                break;
                            }
                        }
                    }
                }
                if (counter == path.size())
                    return true;
            }
            return false;
        };

        for (const GraphicObject& go : graphicObjects_) {
            if (!go.path().empty()) {
                if (Settings::simplifyRegions() && go.path().front() == go.path().back()) {
                    Paths paths;
                    SimplifyPolygon(go.path(), paths);
                    for (Path& path : paths) {
                        path.push_back(path.front());
                        if (!Settings::skipDuplicates()) {
                            checkList.push_front(path);
                            itemGroups_[ApPaths]->push_back(new GiDataPath(checkList.front(), this));
                        } else if (!contains(path)) {
                            checkList.push_front(path);
                            itemGroups_[ApPaths]->push_back(new GiDataPath(checkList.front(), this));
                        }
                    }
                } else {
                    if (!Settings::skipDuplicates()) {
                        itemGroups_[ApPaths]->push_back(new GiDataPath(go.path(), this));
                    } else if (!contains(go.path())) {
                        itemGroups_[ApPaths]->push_back(new GiDataPath(go.path(), this));
                        checkList.push_front(go.path());
                    }
                }
            }
        }
        itemGroups_[ApPaths]->shrink_to_fit();
    }

    bool zeroLine = false;
    for (auto& [dCode, ap] : apertures_)
        if (zeroLine = (qFuzzyIsNull(ap->minSize()) && ap->used()); zeroLine)
            break;

    if (itemsType_ == NullType) {
        if /**/ (itemGroups_[Components]->size())
            itemsType_ = Components;
        else if (itemGroups_[Normal]->size()) // && !zeroLine)
            itemsType_ = Normal;
        else
            itemsType_ = ApPaths;
    }

    layerTypes_[Normal].id = itemGroups_[Normal]->size() ? Normal : NullType;
    layerTypes_[ApPaths].id = itemGroups_[ApPaths]->size() ? ApPaths : NullType;
    layerTypes_[Components].id = itemGroups_[Components]->size() ? Components : NullType;

    itemGroups_[ApPaths]->setVisible(false);
    itemGroups_[Components]->setVisible(false);
    itemGroups_[Normal]->setVisible(false);

    itemGroups_[itemsType_]->setVisible(visible_);
}

} // namespace Gerber
