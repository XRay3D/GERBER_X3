// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "profile.h"
#include "app.h"
#include "gi_bridge.h"
#include "graphicsview.h"

#include <execution>

namespace GCode {
ProfileCtr::ProfileCtr() {
}

void ProfileCtr::create() {
    // WARNING App::fileTreeView()->closeFiles();
    createProfile(gcp_.tools.front(), gcp_.params[GCodeParams::Depth].toDouble());
}

GCodeType ProfileCtr::type() { return Profile; }

void ProfileCtr::createProfile(const Tool& tool, const double depth) {
    do {

        toolDiameter = tool.getDiameter(depth);

        const double dOffset = ((gcp_.side() == Outer) ? +toolDiameter : -toolDiameter) * 0.5 * uScale;

        if (gcp_.side() == On) {
            if (gcp_.params[TrimmingOpenPaths].toBool())
                trimmingOpenPaths(workingRawPs);
            returnPs = std::move(workingPs);
        } else {
            if (workingPs.size()) {
                ClipperOffset offset;
                for (Paths& paths : groupedPaths(Grouping::Copper))
                    offset.AddPaths(paths, JoinType::Round, EndType::Polygon);
                returnPs = offset.Execute(dOffset);
            }
            if (workingRawPs.size()) {
                ClipperOffset offset;
                offset.AddPaths(workingRawPs, JoinType::Round, EndType::Round);
                workingRawPs = offset.Execute(dOffset);
                if (!workingRawPs.empty())
                    returnPs.append(workingRawPs);
            }
        }

        if (returnPs.empty() && workingRawPs.empty())
            break;

        reorder();

        if (gcp_.side() == On && workingRawPs.size()) {
            returnPss.reserve(returnPss.size() + workingRawPs.size());
            mergePaths(workingRawPs);
            sortBeginEnd(workingRawPs);
            for (auto&& path : workingRawPs)
                returnPss.push_back({std::move(path)});
        }

        if (gcp_.params.contains(TrimmingCorners) && gcp_.params[TrimmingCorners].toInt())
            cornerTrimming();

        makeBridges();

        if (returnPss.empty())
            break;

        file_ = new ProfileFile(std::move(gcp_), std::move(returnPss));
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
        return;
    } while (0);
    emit fileReady(nullptr);
}

void ProfileCtr::trimmingOpenPaths(Paths& paths) {
    const double dOffset = toolDiameter * uScale * 0.5;
    for (size_t i = 0; i < paths.size(); ++i) {
        auto& p = paths[i];
        if (p.size() == 2) {
            double l = p.front().distTo(p.back());
            if (l <= toolDiameter * uScale) {
                paths.remove(i--);
                continue;
            }
            QLineF b(p.front(), p.back());
            QLineF e(p.back(), p.front());
            b.setLength(b.length() - toolDiameter * 0.5);
            e.setLength(e.length() - toolDiameter * 0.5);
            p = Path {(b.p2()), (e.p2())};
        } else if (double l = Perimeter(p); l <= toolDiameter * uScale) {
            paths.remove(i--);
            continue;
        } else {
            Paths ps;
            {
                ClipperOffset offset;
                offset.AddPath(p, JoinType::Miter, EndType::Butt);
                ps = offset.Execute(dOffset + 100);

                offset.Clear();
                offset.AddPath(ps.front(), JoinType::Miter, EndType::Polygon);
                ps = offset.Execute(-dOffset);

                if (ps.empty()) {
                    paths.remove(i--);
                    continue;
                }
            }
            {
                Clipper clipper;
                clipper.AddOpenSubject({p});
                clipper.AddClip(ps);
                clipper.Execute(ClipType::Intersection, FillRule::Positive, ps); // FIXME open paths ???

                p = ps.front();
            }
        }
    }
}

void ProfileCtr::cornerTrimming() {
    const double trimDepth = (toolDiameter - toolDiameter * sqrt1_2) * sqrt1_2;
    const double sqareSide = toolDiameter * sqrt1_2 * 0.5;
    const double testAngle = gcp_.convent() ? 90.0 : 270.0;
    const double trimAngle = gcp_.convent() ? -45.0 : +45;

    auto test = [&](QLineF l1, QLineF l2) {
        const bool fl = abs(l1.angleTo(l2) - testAngle) < 1.e-4;
        return fl && sqareSide <= l1.length() && sqareSide <= l2.length();
    };

    for (auto& paths : returnPss) {
        for (auto& path : paths) {
            path.reserve(path.size() * 3);
            for (size_t i = 1, size = path.size() - 1; i < size; size = path.size() - 1, ++i) {
                const auto curCorner = path[i];
                const QLineF l1(path[i - 1], curCorner);
                const QLineF l2(curCorner, path[i + 1]);
                if (test(l1, l2)) {
                    path.insert(path.begin() + i, QLineF::fromPolar(trimDepth, l1.angle() + trimAngle).translated(curCorner).p2());
                    path.insert(path.begin() + i, curCorner);
                    i += 2;
                }
            }
            if (path.front() == path.back()) { // for trimming between the beginning and the end of the path
                const auto curCorner = path.front();
                const QLineF l1(*(path.end() - 2), curCorner);
                const QLineF l2(curCorner, path[1]);
                if (test(l1, l2)) {
                    path.insert(path.end(), QLineF::fromPolar(trimDepth, l1.angle() + trimAngle).translated(curCorner).p2());
                    path.insert(path.end(), curCorner);
                }
            }
            path.shrink_to_fit();
        }
    }
}

void ProfileCtr::makeBridges() {
    auto bridgeItems {App::graphicsView()->items<GiBridge>(GiType::Bridge)};
    if (bridgeItems.empty())
        return;

    std::for_each(std::execution::par_unseq, returnPss.begin(), returnPss.end(), [&bridgeItems, this](Paths& rPaths) -> void {
        // find Bridges
        auto biStack = bridgeItems | rviews::filter([&rPaths](GiBridge* bi) { return bi->test(rPaths.front()); });
        if (ranges::empty(biStack))
            return;
        auto isPositive1 = C2::IsPositive(rPaths.front());

        // create frame
        Paths frame = C2::InflatePaths(rPaths, toolDiameter * uScale * 0.1, JT::Miter, ET::Butt, uScale);
        Paths clip;
        for (GiBridge* bip : biStack)
            clip.append(bip->paths());

        frame = C2::Intersect(frame, clip, FR::Positive);

        // cut toolPath
        Clipper clipper;
        clipper.AddOpenSubject(rPaths);
        clipper.AddClip(frame);
        PolyTree polytree;
        clipper.Execute(CT::Difference, FR::Positive, frame, rPaths);

        if (rPaths.empty())
            return;

        mergeSegments(rPaths);
        sortBeginEnd(rPaths);

        auto IsPositive = [](Paths paths) {
            for (auto&& path : paths.midRef(1))
                paths.front().append(path);
            return C2::IsPositive(paths.front());
        };

        if (isPositive1 ^ IsPositive(rPaths)) // Вернуть исходное направление пути
            ReversePaths(rPaths), ranges::reverse(rPaths);
    });

    std::erase_if(returnPss, [](auto&& paths) { return paths.empty(); });
}

void ProfileCtr::reorder() {
    //    returnPss = {returnPs};
    //    return;
    PolyTree polyTree;
    {
        Clipper clipper;
        clipper.AddSubject(returnPs);
        Rect r(Bounds(returnPs));
        int k = uScale;
        Path outer = {
            { r.left - k, r.bottom + k},
            {r.right + k, r.bottom + k},
            {r.right + k,    r.top - k},
            { r.left - k,    r.top - k}
        };
        clipper.AddSubject({outer});
        clipper.Execute(ClipType::Union, FillRule::EvenOdd, polyTree);
        returnPs.clear();
    }

    polyTreeToPaths(polyTree, returnPs);

    std::reverse(returnPs.begin(), returnPs.end());

    if ((gcp_.side() == Inner) ^ gcp_.convent())
        ReversePaths(returnPs);

    returnPss.reserve(returnPs.size());

    for (auto&& path : returnPs) {
        path.push_back(path.front());
        returnPss.push_back({path});
    }
}

void ProfileCtr::reduceDistance(Point& from, Path& to) {
    double d = std::numeric_limits<double>::max();
    int ctr2 = 0, idx = 0;
    for (auto pt2 : to) {
        if (auto tmp = from.distToSq(pt2); d > tmp) {
            d = tmp;
            idx = ctr2;
        }
        ++ctr2;
    }
    std::rotate(to.begin(), to.begin() + idx, to.end());
    from = to.back();
}

void ProfileCtr::polyTreeToPaths(PolyTree& polytree, Paths& rpaths) {
    rpaths.clear();

    //    auto Total = [i = 0](this auto&& total, PolyTree& polytree) mutable {
    //        return i;
    //    };
    //    rpaths.reserve(Total(polytree));

    std::function<void(PolyTree&, ProfileCtr::NodeType)> addPolyNodeToPaths;

    if (!Settings::profileSort()) { // Grouping by nesting

        markPolyTreeDByNesting(polytree);

        std::map<int, Paths> pathsMap;
        addPolyNodeToPaths = [&addPolyNodeToPaths, &pathsMap, this](PolyTree& polynode, ProfileCtr::NodeType nodetype) {
            bool match = true;
            if (nodetype == ntClosed)
                match = true; //! polynode.IsOpen();// FIXME
            else if (nodetype == ntOpen)
                return;

            if (!polynode.Polygon().empty() && match)
                pathsMap[nesting[&polynode]].emplace_back(std::move(polynode.Polygon()));

            for (auto&& node : polynode)
                addPolyNodeToPaths(*node, nodetype);
        };
        addPolyNodeToPaths(polytree, ntClosed /*ntAny*/);

        pathsMap.extract(pathsMap.begin());

        for (auto& [nest, paths] : pathsMap) {
            qDebug() << "nest" << nest << paths.size();
            if (paths.size() > 1)
                sortB(paths);
            rpaths.append(paths);
        }
    } else { // Grouping by nesting depth
        sortPolyTreeByNesting(polytree);
        Point from = App::settings().mkrZeroOffset();
        std::function<void(PolyTree&, ProfileCtr::NodeType)> addPolyNodeToPaths =
            [&addPolyNodeToPaths, &rpaths, &from, this](PolyTree& polynode, ProfileCtr::NodeType nodetype) {
                bool match = true;
                if (nodetype == ntClosed)
                    match = true; //! polynode.IsOpen(); FIXME
                else if (nodetype == ntOpen)
                    return;

                if (!polynode.Polygon().empty() && match && nesting[std::addressof(polynode)] > 2) {
                    auto path {polynode.Polygon()};
                    reduceDistance(from, path);
                    rpaths.emplace_back(std::move(path));
                }

                //                std::map<int, std::vector<PolyTree*>, std::greater<>> map;
                //                for (auto node : polynode.Childs)
                //                    map[node->Nesting].emplace_back(node);
                //                size_t i = polynode.Count();
                //                for (auto& [nest, nodes] : map) {
                //                    for (auto node : nodes)
                //                        polynode.Childs[--i] = node;
                //                }

                for (auto&& node : polynode)
                    addPolyNodeToPaths(*node, nodetype);
            };

        addPolyNodeToPaths(polytree, ntClosed /*ntAny*/);
    }
}

// void ProfileCreator::addPolyNodeToPaths(PolyTree& polynode, ProfileCreator::NodeType nodetype, Paths& paths)
//{
//     bool match = true;
//     if (nodetype == ntClosed)
//         match = !polynode.IsOpen();
//     else if (nodetype == ntOpen)
//         return;
//     if (!polynode.Polygon().empty() && match) {
//         reduceDistance(from, polynode.Polygon());
//         polynode.Polygon().push_back(polynode.Polygon().front());
//         paths.push_back(std::move(polynode.Polygon()));
//     }
//     for (size_t i = 0; i < polynode.Count(); ++i)
//         addPolyNodeToPaths(*polynode.Childs[i], nodetype, paths);
// }

// void ProfileCreator::closedPathsFromPolyTree(PolyTree& polytree, Paths& paths)
//{
//     paths.resize(0);
//     paths.reserve(polytree.Total());
//     addPolyNodeToPaths(polytree, ntClosed, paths);
// }

// void ProfileCreator::openPathsFromPolyTree(const PolyTree& polytree, Paths& paths)
//{
//     paths.resize(0);
//     paths.reserve(polytree.Total());
//     //Open paths are top level only, so ...
//     for (size_t i = 0; i < polytree.Count(); ++i)
//         if (polytree.Childs[i]->IsOpen())
//             paths.push_back(polytree.Childs[i]->Polygon());
// }

} // namespace GCode
