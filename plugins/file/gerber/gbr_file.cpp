// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbr_file.h"
#include "gbrcomp_item.h"
#include "gbrcomp_onent.h"

#include "clipper.hpp"
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

Path GraphicObject::elipse() const { return (m_state.dCode() == D03
                                                && m_gFile->apertures()->at(m_state.aperture())->type() == ApertureType::Circle) ?
        m_path :
        Path(); } // circle
Paths GraphicObject::elipseW() const { return (m_state.dCode() == D03
                                                  && m_gFile->apertures()->at(m_state.aperture())->type() == ApertureType::Circle
                                                  && m_gFile->apertures()->at(m_state.aperture())->withHole()) ?
        m_paths :
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
    itemGroups_.append({ new GiGroup, new GiGroup });
    name_ = fileName;
    layerTypes_ = {
        { Normal, GbrObj::tr("Normal"), GbrObj::tr("Normal view") },
        { ApPaths, GbrObj::tr("Aperture paths"), GbrObj::tr("Displays only aperture paths of copper\n"
                                                            "without width and without contacts") },
        { Components, GbrObj::tr("Components"), GbrObj::tr("Show components") }
    };
}

File::~File() { }

const ApertureMap* File::apertures() const { return &apertures_; }

Paths File::merge() const {
    QElapsedTimer t;
    t.start();
    mergedPaths_.clear();
    size_t i = 0;

    if constexpr (1) {
        std::list<Paths> pathList;
        {
            std::list<std::map<int, Paths>> pathListMap;
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
                            offset.AddPath(paths[i], ClipperLib::jtRound, ClipperLib::etClosedLine);
                            paths.erase(paths.begin() + i--);
                        }
                    }
                    offset.AddPaths(paths, ClipperLib::jtRound, ClipperLib::etOpenRound);
                    offset.Execute(paths, apertures_.at(aperture)->apertureSize() * uScale * 0.5);
                    // pathList.back().append(std::move(paths));
                    pathList.back().append(paths);
                }
            }
        }

        while (i < graphicObjects_.size()) {
            Clipper clipper;
            clipper.AddPaths(mergedPaths_, ptSubject, true);
            const auto exp = graphicObjects_.at(i).state().imgPolarity();
            do {
                if (graphicObjects_[i].state().type() == Line) {
                    ++i;
                } else {
                    const GraphicObject& go = graphicObjects_.at(i++);
                    clipper.AddPaths(go.paths(), ptClip, true);
                }
            } while (i < graphicObjects_.size() && exp == graphicObjects_.at(i).state().imgPolarity());

            if (exp)
                ReversePaths(pathList.front());
            clipper.AddPaths(pathList.front(), ptClip, true);
            pathList.pop_front();

            if (graphicObjects_.at(i - 1).state().imgPolarity() == Positive)
                clipper.Execute(ctUnion, mergedPaths_, pftPositive);
            else
                clipper.Execute(ctDifference, mergedPaths_, pftNonZero);
        }
    } else {
        while (i < graphicObjects_.size()) {
            Clipper clipper;
            clipper.AddPaths(mergedPaths_, ptSubject, true);
            const auto exp = graphicObjects_.at(i).state().imgPolarity();
            do {
                const GraphicObject& go = graphicObjects_.at(i++);
                clipper.AddPaths(go.paths(), ptClip, true);
            } while (i < graphicObjects_.size() && exp == graphicObjects_.at(i).state().imgPolarity());
            if (graphicObjects_.at(i - 1).state().imgPolarity() == Positive)
                clipper.Execute(ctUnion, mergedPaths_, pftPositive);
            else
                clipper.Execute(ctDifference, mergedPaths_, pftNonZero);
        }
    }

    if (Settings::cleanPolygons())
        CleanPolygons(mergedPaths_, Settings::cleanPolygonsDist() * uScale);
    return mergedPaths_;
}

const QList<Component>& File::components() const { return components_; }

void File::grouping(PolyNode* node, Pathss* pathss, File::Group group) {
    Path path;
    Paths paths;
    switch (group) {
    case CutoffGroup:
        if (!node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (size_t i = 0; i < node->ChildCount(); ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (size_t i = 0; i < node->ChildCount(); ++i) {
            grouping(node->Childs[i], pathss, group);
        }
        break;
    case CopperGroup:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (size_t i = 0; i < node->ChildCount(); ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (size_t i = 0; i < node->ChildCount(); ++i) {
            grouping(node->Childs[i], pathss, group);
        }
        break;
    }
}

Pathss& File::groupedPaths(File::Group group, bool fl) {
    if (groupedPaths_.empty()) {
        PolyTree polyTree;
        Clipper clipper;
        clipper.AddPaths(mergedPaths(), ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = /*uScale*/ 1;
        Path outer = {
            IntPoint(r.left - k, r.bottom + k),
            IntPoint(r.right + k, r.bottom + k),
            IntPoint(r.right + k, r.top - k),
            IntPoint(r.left - k, r.top - k)
        };
        if (fl)
            ReversePath(outer);
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftNonZero);
        grouping(polyTree.GetFirst(), &groupedPaths_, group);
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

FileTree::Node* File::node() {
    return node_ ? node_ : node_ = new Node(this, &id_);
}

void File::setItemType(int type) {
    if (itemsType_ == type)
        return;

    itemsType_ = type;

    itemGroups_[Normal]->setVisible(false);
    itemGroups_[ApPaths]->setVisible(false);
    itemGroups_[Components]->setVisible(false);

    itemGroups_[itemsType_]->setVisible(true /*m_visible*/);
}

int File::itemsType() const { return itemsType_; }

void File::write(QDataStream& stream) const {
    stream << graphicObjects_; // write  QList<GraphicObject>
    stream << apertures_;
    stream << m_format;
    // stream << layer;
    // stream << miror;
    stream << rawIndex;
    stream << itemsType_;
    stream << components_;
}

void File::read(QDataStream& stream) {
    crutch = &m_format;        ///////////////////
    stream >> graphicObjects_; // read  QList<GraphicObject>
    stream >> apertures_;
    stream >> m_format;
    // stream >> layer;
    // stream >> miror;
    stream >> rawIndex;
    stream >> itemsType_;
    stream >> components_;
    QPointF offset_;
    if (App::project()->ver() >= ProVer_6)
        stream >> offset_;

    for (GraphicObject& go : graphicObjects_) {
        go.m_gFile = this;
        go.m_state.m_format = format();
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
        for (const Component& component : qAsConst(components_)) {
            if (!component.referencePoint().isNull())
                itemGroups_[Components]->push_back(new ComponentItem(component, this));
        }
        itemGroups_[Components]->shrink_to_fit();
    }
    if constexpr (1) { // add aperture paths
        auto contains = [&](const Path& path) -> bool {
            constexpr double k = 0.001 * uScale;
            for (const Path& chPath : checkList) { // find copy
                size_t counter = 0;
                if (chPath.size() == path.size()) {
                    for (const IntPoint& p1 : chPath) {
                        for (const IntPoint& p2 : path) {
                            if ((abs(p1.X - p2.X) < k) && (abs(p1.Y - p2.Y) < k)) {
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
