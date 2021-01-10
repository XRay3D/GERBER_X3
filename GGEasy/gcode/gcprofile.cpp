// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
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
        for (int index = 0; index < m_returnPs.size(); ++index) {
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

    sortB(m_returnPs);
    if (m_returnPs.size() != 0)
        m_returnPss.push_back(m_returnPs);
    sortBE(m_returnPss);
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
    } else {
        m_gcp.gcType = Profile;
        m_file = new File(m_returnPss, m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

void ProfileCreator::strip()
{
    const double dOffset = m_toolDiameter * uScale * 0.5;
    for (int i = 0; i < m_workingRawPs.size(); ++i) {
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
}
