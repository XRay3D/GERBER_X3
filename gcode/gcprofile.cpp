#include "gcprofile.h"
#include "gcfile.h"

#include <gi/bridgeitem.h>

#include <scene.h>

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
    App::mInstance->m_creator = this;

    m_toolDiameter = tool.getDiameter(depth);
    // execute offset
    if (m_gcp.side() == On) {
        m_returnPs = m_workingPs;

        for (Path& path : m_returnPs)
            path.append(path.first());

        // fix direction
        if (m_gcp.convent())
            ReversePaths(m_returnPs);

        if (m_workingRawPs.size())
            m_returnPs.append(m_workingRawPs);

    } else {
        // calc offset
        const double dOffset = (m_gcp.side() == Outer) ? +m_toolDiameter * uScale * 0.5 : -m_toolDiameter * uScale * 0.5;

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
        }

        if (!m_workingRawPs.isEmpty())
            m_returnPs.append(m_workingRawPs);

        // fix direction
        if (m_gcp.side() == Outer && !m_gcp.convent())
            ReversePaths(m_returnPs);
        else if (m_gcp.side() == Inner && m_gcp.convent())
            ReversePaths(m_returnPs);

        for (Path& path : m_returnPs)
            path.append(path.first());
    }

    if (m_returnPs.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }

    // find Bridges
    QVector<BridgeItem*> bridgeItems;
    for (QGraphicsItem* item : App::scene()->items()) {
        if (item->type() == GiBridge)
            bridgeItems.append(static_cast<BridgeItem*>(item));
    }
    // create Bridges
    if (bridgeItems.size()) {
        for (int index = 0; index < m_returnPs.size(); ++index) {
            const Path& path = m_returnPs.at(index);
            QList<QPair<BridgeItem*, IntPoint>> biStack;
            for (BridgeItem* bi : bridgeItems) {
                IntPoint pt;
                if (pointOnPolygon(bi->getPath(), path, &pt))
                    biStack.append({ bi, pt });
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
                    for (const QPair<BridgeItem*, IntPoint>& bip : biStack) {
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
                m_returnPss.append(sortBE(paths));
            }
        }
    }

    sortB(m_returnPs);
    if (m_returnPs.size() != 0)
        m_returnPss.append(m_returnPs);
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
}
