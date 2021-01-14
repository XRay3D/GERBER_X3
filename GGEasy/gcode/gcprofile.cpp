// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcprofile.h"
#include "gcfile.h"

#include "bridgeitem.h"

#include "scene.h"

#include "leakdetector.h"

namespace GCode {
ProfileCreator::ProfileCreator()
{
}

void ProfileCreator::create()
{
    createProfile(m_gcp.tools.first(), m_gcp.params[GCodeParams::Depth].toDouble());
}

GCodeType ProfileCreator::type() { return Profile; }

void ProfileCreator::createProfile(const Tool& tool, const double depth)
{

    m_toolDiameter = tool.getDiameter(depth);

    const double dOffset = (m_gcp.side() == Outer) ? +m_toolDiameter * uScale * 0.5 : -m_toolDiameter * uScale * 0.5;

    // execute offset
    if (m_gcp.side() == On) {

        if (m_gcp.params[GCodeParams::Strip].toBool())
            strip();

        m_returnPs = m_workingPs;

        for (Path& path : m_returnPs)
            path.push_back(path.first());

        // fix direction
        if (m_gcp.convent())
            ReversePaths(m_returnPs);

        if (m_workingRawPs.size())
            m_returnPs.push_back(m_workingRawPs);

    } else {
        // calc offset

        // execute offset
        if (!m_workingPs.isEmpty()) {
            ClipperOffset offset;
            for (Paths& paths : groupedPaths(CopperPaths))
                offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(m_returnPs, dOffset);
        }

        if (!m_workingRawPs.isEmpty()) {
            ClipperOffset offset;
            offset.AddPaths(m_workingRawPs, jtRound, etOpenRound);
            offset.Execute(m_workingRawPs, dOffset);
            if (!m_workingRawPs.isEmpty())
                m_returnPs.push_back(m_workingRawPs);
        }

        // fix direction
        if (m_gcp.side() == Outer && !m_gcp.convent())
            ReversePaths(m_returnPs);
        else if (m_gcp.side() == Inner && m_gcp.convent())
            ReversePaths(m_returnPs);

        for (Path& path : m_returnPs)
            path.push_back(path.first());
    }

    if (m_returnPs.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }

    // find Bridges
    mvector<BridgeItem*> bridgeItems;
    for (QGraphicsItem* item : App::scene()->items()) {
        if (static_cast<GiType>(item->type()) == GiType::Bridge)
            bridgeItems.push_back(static_cast<BridgeItem*>(item));
    }
    // create Bridges
    if (bridgeItems.size()) {
        for (size_t index = 0; index < m_returnPs.size(); ++index) {
            const Path& path = m_returnPs.at(index);
            QList<QPair<BridgeItem*, Point64>> biStack;
            for (BridgeItem* bi : bridgeItems) {
                Point64 pt;
                if (pointOnPolygon(bi->getPath(), path, &pt))
                    biStack.push_back({ bi, pt });
            }
            if (!biStack.isEmpty()) {
                Paths paths;
                // create frame
                {
                    ClipperOffset offset;
                    offset.AddPath(path, jtMiter, etClosedLine);
                    offset.Execute(paths, +m_toolDiameter * uScale * 0.1);

                    Clipper clipper;
                    clipper.AddPaths(paths, ptSubject, true);
                    for (const QPair<BridgeItem*, Point64>& bip : biStack) {
                        clipper.AddPath(CirclePath((bip.first->lenght() + m_toolDiameter) * uScale, bip.second), ptClip, true);
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
                m_returnPs.remove(index--);
                m_returnPss.push_back(sortBE(paths));
            }
        }
    }

    PolyTree polyTree;
    {
        Clipper clipper;
        clipper.AddPaths(m_returnPs, ptSubject, true);
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
        m_returnPs.clear();
    }

    m_returnPss.resize(1);
    polyTreeToPaths(polyTree, m_returnPss.front());

    m_returnPss.front().takeFirst();

    reorder();

    for (auto& path : m_returnPss.front())
        path.append(path.first());

    //    sortB(m_returnPs);
    //    if (m_returnPs.size() != 0)
    //        m_returnPss.push_back(m_returnPs);
    //    sortBE(m_returnPss);

    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
    } else {
        //std::reverse(m_returnPss.begin(), m_returnPss.end());
        m_gcp.gcType = Profile;
        m_file = new File(m_returnPss, m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

void ProfileCreator::strip()
{
    const double dOffset = m_toolDiameter * uScale * 0.5;
    for (size_t i = 0; i < m_workingRawPs.size(); ++i) {
        auto& p = m_workingRawPs[i];
        if (p.size() == 2) {
            double l = Length(p.first(), p.last());
            if (l <= m_toolDiameter * uScale) {
                m_workingRawPs.remove(i--);
                continue;
            }
            QLineF b(p.first(), p.last());
            QLineF e(p.last(), p.first());
            b.setLength(b.length() - m_toolDiameter * 0.5);
            e.setLength(e.length() - m_toolDiameter * 0.5);
            p = { (b.p2()), (e.p2()) };
        } else if (double l = Perimeter(p); l <= m_toolDiameter * uScale) {
            m_workingRawPs.remove(i--);
            continue;
        } else {
            Paths ps;
            {
                ClipperOffset offset;
                offset.AddPath(p, jtMiter, etOpenButt);
                offset.Execute(ps, dOffset + 100);
                //dbgPaths(ps, {}, "offset+");
                offset.Clear();
                offset.AddPath(ps.first(), jtMiter, etClosedPolygon);
                offset.Execute(ps, -dOffset);
                //dbgPaths(ps, {}, "offset-");
                if (ps.isEmpty()) {
                    m_workingRawPs.remove(i--);
                    continue;
                }
            }
            {
                Clipper clipper;
                clipper.AddPath(p, ptSubject, false);
                clipper.AddPaths(ps, ptClip, true);
                clipper.Execute(ctIntersection, ps, pftPositive);
                //dbgPaths(ps, {}, "clip-");
                p = ps.first();
            }
        }
    }
}

void ProfileCreator::reorder()
{
    Paths& paths = m_returnPss.front();
    std::reverse(paths.begin(), paths.end());
    int ctr1 = 0, ctr2 = 0, j = 0, k = 0;
    for (size_t i = 1; i < paths.size(); ++i) {
        Path& path1 = paths[i - 1];
        Path& path2 = paths[i];
        if (i == 1) {
            double d = std::numeric_limits<double>::max();
            ctr1 = 0;
            for (auto pt1 : path1) {
                ctr2 = 0;
                for (auto pt2 : path2) {
                    if (auto tmp = pt1.distTo(pt2); d > tmp) {
                        d = tmp;
                        j = ctr1;
                        k = ctr2;
                    }
                    ++ctr2;
                }
                ++ctr1;
            }
            std::rotate(path1.begin(), path1.begin() + j, path1.end());
            std::rotate(path2.begin(), path2.begin() + k, path2.end());
        } else {
            double d = std::numeric_limits<double>::max();
            auto pt1 = path1.front();
            ctr2 = 0;
            for (auto pt2 : path2) {
                if (auto tmp = pt1.distTo(pt2); d > tmp) {
                    d = tmp;
                    k = ctr2;
                }
                ++ctr2;
            }
            std::rotate(path2.begin(), path2.begin() + k, path2.end());
        }
    }
}

void ProfileCreator::addPolyNodeToPaths(const PolyNode& polynode, ProfileCreator::NodeType nodetype, Paths& paths)
{
    bool match = true;
    if (nodetype == ntClosed)
        match = !polynode.IsOpen();
    else if (nodetype == ntOpen)
        return;

    if (!polynode.Contour.empty() && match)
        paths.push_back(polynode.Contour);
    for (size_t i = 0; i < polynode.ChildCount(); ++i)
        addPolyNodeToPaths(*polynode.Childs[i], nodetype, paths);
}

void ProfileCreator::polyTreeToPaths(const PolyTree& polytree, Paths& paths)
{
    paths.resize(0);
    paths.reserve(polytree.Total());
    addPolyNodeToPaths(polytree, ntAny, paths);
}

void ProfileCreator::closedPathsFromPolyTree(const PolyTree& polytree, Paths& paths)
{
    paths.resize(0);
    paths.reserve(polytree.Total());
    addPolyNodeToPaths(polytree, ntClosed, paths);
}

void ProfileCreator::openPathsFromPolyTree(const PolyTree& polytree, Paths& paths)
{
    paths.resize(0);
    paths.reserve(polytree.Total());
    //Open paths are top level only, so ...
    for (size_t i = 0; i < polytree.ChildCount(); ++i)
        if (polytree.Childs[i]->IsOpen())
            paths.push_back(polytree.Childs[i]->Contour);
}
}
