#include "gcpocketoffset.h"
#include "gcfile.h"
#include <QElapsedTimer>
#include <project.h>

namespace GCode {
PocketCreator::PocketCreator()
{
}

void PocketCreator::create()
{
    if (m_gcp.tools.count() > 1) {
        createPocket2(m_gcp.tools, m_gcp.params[GCodeParams::Depth].toDouble());
    } else {
        createPocket(m_gcp.tools.first(), m_gcp.params[GCodeParams::Depth].toDouble(), m_gcp.params[GCodeParams::Steps].toInt());
    }
}

void PocketCreator::createPocket(const Tool& tool, const double depth, const int steps)
{
    App::mInstance->m_creator = this;
    if (m_gcp.side() == On)
        return;
    progress(0, 0);
    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;
    Paths fillPaths;
    if (steps) {
        groupedPaths(CopperPaths);
        if (m_gcp.side() == Inner) {
            m_dOffset *= -1;
            for (Paths paths : m_groupedPss) {
                ClipperOffset offset(uScale);
                offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(paths, m_dOffset);
                fillPaths.append(paths);
                Paths tmpPaths;
                int counter = steps;
                if (counter > 1) {
                    do {
                        if (counter == 1)
                            fillPaths.append(paths);
                        tmpPaths.append(paths);
                        offset.Clear();
                        offset.AddPaths(paths, jtMiter, etClosedPolygon);
                        offset.Execute(paths, m_dOffset);
                    } while (paths.size() && --counter);
                } else {
                    tmpPaths.append(paths);
                    fillPaths.append(paths);
                }
                m_returnPs.append(tmpPaths);
            }
        } else {
            ClipperOffset offset(uScale);
            for (Paths paths : m_groupedPss) {
                offset.AddPaths(paths, jtRound, etClosedPolygon);
            }
            Paths paths;
            offset.Execute(paths, m_dOffset);
            fillPaths.append(paths);
            int counter = steps;

            if (counter > 1) {
                do {
                    if (counter == 1)
                        fillPaths.append(paths);
                    m_returnPs.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, m_stepOver);
                } while (paths.size() && --counter);
            } else {
                m_returnPs.append(paths);
                fillPaths.append(paths);
            }
        }
    } else {
        switch (m_gcp.side()) {
        case On:
            break;
        case Outer:
            groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5));
            if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
                m_groupedPss.removeFirst();
            break;
        case Inner:
            groupedPaths(CopperPaths);
            break;
        }
        for (Paths paths : m_groupedPss) {
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, -m_dOffset);
            fillPaths.append(paths);
            Paths offsetPaths;
            do {
                offsetPaths.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, -m_stepOver);
            } while (paths.size());
            m_returnPs.append(offsetPaths);
        }
    }

    if (m_returnPs.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }

    stacking(m_returnPs);

    //        ReversePaths(m_returnPaths);
    //        sortByStratDistance(m_returnPaths);
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
    } else {
        m_gcp.gcType = Pocket;
        m_file = new GCode::File(m_returnPss, m_gcp, fillPaths);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

void PocketCreator::createPocket2(QVector<Tool>& tools, double depth)
{
    App::mInstance->m_creator = this;

    switch (m_gcp.side()) {
    case On:
        return;
    case Outer:
        groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5.0));
        if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
            m_groupedPss.removeFirst();
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    }

    qDebug() << tools;

    for (int i = 0; i < tools.size(); ++i) {
        const Tool& tool = tools[i];
        Paths fillPaths;
        m_returnPs.clear();
        if (i) {
            m_toolDiameter = tool.getDiameter(depth) * uScale;
            m_dOffset = m_toolDiameter / 2;
            m_stepOver = tool.stepover() * uScale;

            for (Paths paths : m_groupedPss) {
                { //create frame
                    Paths framePaths;
                    for (int ii = 0; ii < i; ++ii) {
                        double toolDiameter = tools[ii].getDiameter(depth) * uScale;
                        double dOffset = toolDiameter / 2;
                        Paths tmpPaths;
                        {
                            ClipperOffset offset(uScale);
                            offset.AddPaths(paths, jtRound, etClosedPolygon);
                            offset.Execute(tmpPaths, -dOffset);
                        }
                        {
                            ClipperOffset offset(uScale);
                            offset.AddPaths(tmpPaths, jtRound, etClosedPolygon);
                            offset.Execute(tmpPaths, dOffset - m_toolDiameter * 0.999);
                        }
                        {
                            Clipper cliper;
                            cliper.AddPaths(framePaths, ptSubject, true);
                            cliper.AddPaths(tmpPaths, ptClip, true);
                            cliper.Execute(ctUnion, framePaths, pftPositive);
                        }
                    }
                    if (m_gcp.side() != Inner)
                        ReversePaths(framePaths);
                    paths.append(framePaths);
                }
                ClipperOffset offset(uScale);
                offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(paths, -m_dOffset);
                fillPaths.append(paths);
                Paths offsetPaths;
                const auto area = M_PI * (m_dOffset * 0.5) * (m_dOffset * 0.5);
                do {
                    if (offsetPaths.size() && paths.size()) {
                        for (const auto& p : paths) {
                            const auto a = abs(Area(p));
                            if (!qFuzzyIsNull(a) && a >= area)
                                offsetPaths.append(p); //                                    paths.remove(i--);
                        }
                    } else {
                        offsetPaths.append(paths);
                    }
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, -m_stepOver);
                } while (paths.size());
                m_returnPs.append(offsetPaths);
            }

            if (m_returnPs.isEmpty()) {
                emit fileReady(nullptr);
                continue;
            }

            //                for (int i = 0; i < m_returnPs.size(); ++i) {
            //                    if (Perimeter(m_returnPs[i]) < m_dOffset)
            //                        m_returnPs.remove(i--);
            //                }
            //                for (int i = 0; i < fillPaths.size(); ++i) {
            //                    if (Perimeter(fillPaths[i]) < m_dOffset)
            //                        fillPaths.remove(i--);
            //                }

        } else {
            m_toolDiameter = tool.getDiameter(depth) * uScale;
            m_dOffset = m_toolDiameter / 2;
            m_stepOver = tool.stepover() * uScale;

            for (Paths paths : m_groupedPss) {
                ClipperOffset offset(uScale);
                offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(paths, -m_dOffset);
                fillPaths.append(paths);

                Paths tmpPaths;
                do {
                    tmpPaths.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, -m_stepOver);
                } while (paths.size());
                m_returnPs.append(tmpPaths);
            }

            if (m_returnPs.isEmpty()) {
                emit fileReady(nullptr);
                continue;
            }
        }

        stacking(m_returnPs);

        if (m_returnPss.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_gcp.gcType = Pocket;
            m_gcp.params[GCodeParams::PocketIndex] = i;
            m_file = new GCode::File(m_returnPss, m_gcp, fillPaths);
            m_file->setFileName(tool.nameEnc());
            emit fileReady(m_file);
        }
    }
}
}
