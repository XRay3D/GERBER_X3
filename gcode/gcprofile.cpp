#include "gcprofile.h"
#include "gcfile.h"

#include <gi/bridgeitem.h>

#include <scene.h>

namespace GCode {
ProfileCreator::ProfileCreator()
{
}

void ProfileCreator::create(const GCodeParams& gcp)
{
    m_gcp = gcp;
    createProfile(gcp.tool.first(), gcp.dParam[Depth]);
}

void ProfileCreator::createProfile(const Tool& tool, const double depth)
{
    try {
        self = this;

        m_toolDiameter = tool.getDiameter(depth);
        // execute offset
        if (m_gcp.side == On) {
            m_returnPaths = m_workingPaths;

            for (Path& path : m_returnPaths)
                path.append(path.first());

            // fix direction
            if (m_gcp.convent)
                ReversePaths(m_returnPaths);

            if (m_workingRawPaths.size())
                m_returnPaths.append(m_workingRawPaths);

        } else {
            // calc offset
            const double dOffset = (m_gcp.side == Outer) ? +m_toolDiameter * uScale * 0.5 : -m_toolDiameter * uScale * 0.5;

            // execute offset
            if (!m_workingPaths.isEmpty()) {
                ClipperOffset offset;
                for (Paths& paths : groupedPaths(CopperPaths))
                    offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(m_returnPaths, dOffset);
            }
            if (!m_workingRawPaths.isEmpty()) {
                ClipperOffset offset;
                offset.AddPaths(m_workingRawPaths, jtRound, etOpenRound);
                offset.Execute(m_workingRawPaths, dOffset);
            }

            if (!m_workingRawPaths.isEmpty())
                m_returnPaths.append(m_workingRawPaths);

            // fix direction
            if (m_gcp.side == Outer && !m_gcp.convent)
                ReversePaths(m_returnPaths);
            else if (m_gcp.side == Inner && m_gcp.convent)
                ReversePaths(m_returnPaths);

            for (Path& path : m_returnPaths)
                path.append(path.first());
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            return;
        }

        // find Bridges
        QVector<BridgeItem*> bridgeItems;
        for (QGraphicsItem* item : Scene::items()) {
            if (item->type() == BridgeType)
                bridgeItems.append(static_cast<BridgeItem*>(item));
        }
        // create Bridges
        if (bridgeItems.size()) {
            for (int index = 0, size = m_returnPaths.size(); index < size; ++index) {
                Path& path = m_returnPaths[index];
                QList<QPair<BridgeItem*, IntPoint>> biStack;
                for (BridgeItem* bi : bridgeItems) {
                    IntPoint pt;
                    if (PointOnPolygon(bi->getPath(), path, &pt))
                        biStack.append({ bi, pt });
                }
                if (!biStack.isEmpty()) {
                    Paths tmpPaths;
                    // create frame
                    {
                        ClipperOffset offset;
                        offset.AddPath(path, jtMiter, etClosedLine);
                        offset.Execute(tmpPaths, +m_toolDiameter * uScale * 0.1);

                        Clipper clipper;
                        clipper.AddPaths(tmpPaths, ptSubject, true);
                        for (const QPair<BridgeItem*, IntPoint>& bip : biStack) {
                            clipper.AddPath(CirclePath((bip.first->lenght() + m_toolDiameter) * uScale, bip.second), ptClip, true);
                        }
                        clipper.Execute(ctIntersection, tmpPaths, pftPositive);
                    }
                    // cut toolPath
                    {
                        Clipper clipper;
                        clipper.AddPath(path, ptSubject, false);
                        clipper.AddPaths(tmpPaths, ptClip, true);
                        PolyTree polytree;
                        clipper.Execute(ctDifference, polytree, pftPositive);
                        PolyTreeToPaths(polytree, tmpPaths);
                    }
                    // merge result toolPaths
                    for (int i = 0; i < tmpPaths.size(); ++i) {
                        for (int j = 0; j < tmpPaths.size(); ++j) {
                            if (i == j)
                                continue;
                            if (tmpPaths[i].last() == tmpPaths[j].first()) {
                                tmpPaths[j].removeFirst();
                                tmpPaths[i].append(tmpPaths[j]);
                                tmpPaths.remove(j--);
                                continue;
                            }
                            if (tmpPaths[i].first() == tmpPaths[j].last()) {
                                tmpPaths[i].removeFirst();
                                tmpPaths[j].append(tmpPaths[i]);
                                tmpPaths.remove(i--);
                                break;
                            }
                            if (tmpPaths[i].last() == tmpPaths[j].last()) {
                                ReversePath(tmpPaths[j]);
                                tmpPaths[j].removeFirst();
                                tmpPaths[i].append(tmpPaths[j]);
                                tmpPaths.remove(j--);
                                continue;
                            }
                        }
                    }

                    --size;
                    m_returnPaths.remove(index--);
                    m_returnPaths.append(tmpPaths);
                }
            }
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_file = new File(sortByStratDistance(m_returnPaths), tool, depth, Profile);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        }
    } catch (...) {
        //qDebug() << "catch";
    }
}
}
