// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "profile.h"
#include "gc_file.h"
#include "profile_form.h"
#include "gi_bridge.h"
#include "mainwindow.h"
#include "scene.h"
#include <numbers>

namespace GCode {
ProfileCreator::ProfileCreator() {
}

void ProfileCreator::create() {
    createProfile(gcp_.tools.front(), gcp_.params[GCodeParams::Depth].toDouble());
}

GCodeType ProfileCreator::type() { return Profile; }

void ProfileCreator::createProfile(const Tool& tool, const double depth) {
    do {

        toolDiameter = tool.getDiameter(depth);

        const double dOffset = (gcp_.side() == Outer) ? +toolDiameter * uScale * 0.5 : -toolDiameter * uScale * 0.5;

        if (gcp_.side() == On) {
            if (gcp_.params[GCodeParams::Trimming].toBool())
                trimmingOpenPaths(workingRawPs);
            returnPs = std::move(workingPs);
        } else {
            if (workingPs.size()) {
                ClipperOffset offset;
                for (Paths& paths : groupedPaths(CopperPaths))
                    offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(returnPs, dOffset);
            }
            if (workingRawPs.size()) {
                ClipperOffset offset;
                offset.AddPaths(workingRawPs, jtRound, etOpenRound);
                offset.Execute(workingRawPs, dOffset);
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
            sortBE(workingRawPs);
            for (auto&& path : workingRawPs)
                returnPss.push_back({ std::move(path) });
        }

        makeBridges();

        if (gcp_.params.contains(GCodeParams::CornerTrimming) && gcp_.params[GCodeParams::CornerTrimming].toInt())
            cornerTrimming();

        if (returnPss.empty())
            break;

        gcp_.gcType = Profile;
        file_ = new File(returnPss,  std::move(gcp_));
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
        return;
    } while (0);
    emit fileReady(nullptr);
}

void ProfileCreator::trimmingOpenPaths(Paths& paths) {
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
            p = { (b.p2()), (e.p2()) };
        } else if (double l = Perimeter(p); l <= toolDiameter * uScale) {
            paths.remove(i--);
            continue;
        } else {
            Paths ps;
            {
                ClipperOffset offset;
                offset.AddPath(p, jtMiter, etOpenButt);
                offset.Execute(ps, dOffset + 100);
                // dbgPaths(ps, {}, "offset+");
                offset.Clear();
                offset.AddPath(ps.front(), jtMiter, etClosedPolygon);
                offset.Execute(ps, -dOffset);
                // dbgPaths(ps, {}, "offset-");
                if (ps.empty()) {
                    paths.remove(i--);
                    continue;
                }
            }
            {
                Clipper clipper;
                clipper.AddPath(p, ptSubject, false);
                clipper.AddPaths(ps, ptClip, true);
                clipper.Execute(ctIntersection, ps, pftPositive);
                // dbgPaths(ps, {}, "clip-");
                p = ps.front();
            }
        }
    }
}

void ProfileCreator::cornerTrimming() {
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

void ProfileCreator::makeBridges() {
    // find Bridges
    mvector<GiBridge*> bridgeItems;
    for (QGraphicsItem* item : App::scene()->items()) {
        if (static_cast<GiType>(item->type()) == GiType::Bridge)
            bridgeItems.push_back(static_cast<GiBridge*>(item));
    }
    // create Bridges
    if (bridgeItems.size()) {
        for (auto& returnPs_ : returnPss) {
            const Path& path = returnPs_.front();
            std::vector<std::pair<GiBridge*, IntPoint>> biStack;
            biStack.reserve(bridgeItems.size());
            IntPoint pt;
            for (GiBridge* bi : bridgeItems) {
                if (pointOnPolygon(bi->getPath(), path, &pt))
                    biStack.emplace_back(bi, pt);
            }
            if (!biStack.empty()) {
                Paths paths;
                // create frame
                {
                    ClipperOffset offset;
                    offset.AddPath(path, jtMiter, etClosedLine);
                    offset.Execute(paths, +toolDiameter * uScale * 0.1);

                    Clipper clipper;
                    clipper.AddPaths(paths, ptSubject, true);
                    for (const auto& bip : biStack) {
                        clipper.AddPath(CirclePath((bip.first->lenght() + toolDiameter) * uScale, bip.second), ptClip, true);
                    }
                    clipper.Execute(ctIntersection, paths, pftPositive);
                }
                // cut toolPath
                {
                    Clipper clipper;
                    clipper.AddPath(path, ptSubject, false);
                    clipper.AddPaths(paths, ptClip, true);
                    PolyTree polytree;
                    clipper.Execute(ctDifference, polytree, pftPositive);
                    PolyTreeToPaths(polytree, paths);
                }
                // merge result toolPaths
                mergeSegments(paths);
                sortBE(paths);

                auto check = [&paths, &path] {
                    for (size_t i = 0, end = path.size() - 1; i < end; ++i) {
                        auto srcPt(path[i]);
                        for (const auto& rPath : paths) {
                            for (size_t j = 0, rEnd = rPath.size() - 1; j < rEnd; ++j) {
                                if (srcPt == rPath[j]) {
                                    if (path[i + 1] == rPath[j + 1]) {
                                        return false;
                                    } else if (j && path[i + 1] == rPath[j - 1]) {
                                        return true;
                                    } else if (j && i && path[i - 1] == rPath[j - 1]) {
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                    return false; // ???
                };

                if (check())
                    ReversePaths(paths);

                std::swap(returnPs_, paths);
            }
        }
    }
}

void ProfileCreator::reorder() {
    //    returnPss = { returnPs };
    //    return;
    PolyTree polyTree;
    {
        Clipper clipper;
        clipper.AddPaths(returnPs, ptSubject);
        IntRect r(clipper.GetBounds());
        int k = uScale;
        Path outer = {
            { r.left - k, r.bottom + k },
            { r.right + k, r.bottom + k },
            { r.right + k, r.top - k },
            { r.left - k, r.top - k }
        };
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftEvenOdd);
        returnPs.clear();
    }

    polyTreeToPaths(polyTree, returnPs);

    std::reverse(returnPs.begin(), returnPs.end());

    if ((gcp_.side() == Inner) ^ gcp_.convent())
        ReversePaths(returnPs);

    returnPss.reserve(returnPs.size());

    for (auto&& path : returnPs) {
        path.push_back(path.front());
        returnPss.push_back({ path });
    }
}

void ProfileCreator::reduceDistance(IntPoint& from, Path& to) {
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

void ProfileCreator::polyTreeToPaths(PolyTree& polytree, Paths& rpaths) {
    rpaths.clear();
    rpaths.reserve(polytree.Total());

    std::function<void(PolyNode&, ProfileCreator::NodeType)> addPolyNodeToPaths;

    if (!Settings::profileSort()) { // Grouping by nesting

        markPolyNodeByNesting(polytree);

        std::map<int, Paths> pathsMap;
        addPolyNodeToPaths = [&addPolyNodeToPaths, &pathsMap](PolyNode& polynode, ProfileCreator::NodeType nodetype) {
            bool match = true;
            if (nodetype == ntClosed)
                match = !polynode.IsOpen();
            else if (nodetype == ntOpen)
                return;

            if (!polynode.Contour.empty() && match)
                pathsMap[polynode.Nesting].emplace_back(std::move(polynode.Contour));

            for (auto node : polynode.Childs)
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
        sortPolyNodeByNesting(polytree);
        IntPoint from = App::settings().mkrZeroOffset();
        std::function<void(PolyNode&, ProfileCreator::NodeType)> addPolyNodeToPaths =
            [&addPolyNodeToPaths, &rpaths, &from, this](PolyNode& polynode, ProfileCreator::NodeType nodetype) {
                bool match = true;
                if (nodetype == ntClosed)
                    match = !polynode.IsOpen();
                else if (nodetype == ntOpen)
                    return;

                if (!polynode.Contour.empty() && match && polynode.Nesting > 2) {
                    reduceDistance(from, polynode.Contour);
                    rpaths.emplace_back(std::move(polynode.Contour));
                }

                //                std::map<int, std::vector<PolyNode*>, std::greater<>> map;
                //                for (auto node : polynode.Childs)
                //                    map[node->Nesting].emplace_back(node);
                //                size_t i = polynode.ChildCount();
                //                for (auto& [nest, nodes] : map) {
                //                    for (auto node : nodes)
                //                        polynode.Childs[--i] = node;
                //                }

                for (auto node : polynode.Childs)
                    addPolyNodeToPaths(*node, nodetype);
            };

        addPolyNodeToPaths(polytree, ntClosed /*ntAny*/);
    }
}

// void ProfileCreator::addPolyNodeToPaths(PolyNode& polynode, ProfileCreator::NodeType nodetype, Paths& paths)
//{
//     bool match = true;
//     if (nodetype == ntClosed)
//         match = !polynode.IsOpen();
//     else if (nodetype == ntOpen)
//         return;
//     if (!polynode.Contour.empty() && match) {
//         reduceDistance(from, polynode.Contour);
//         polynode.Contour.push_back(polynode.Contour.front());
//         paths.push_back(std::move(polynode.Contour));
//     }
//     for (size_t i = 0; i < polynode.ChildCount(); ++i)
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
//     for (size_t i = 0; i < polytree.ChildCount(); ++i)
//         if (polytree.Childs[i]->IsOpen())
//             paths.push_back(polytree.Childs[i]->Contour);
// }
} // namespace GCode
