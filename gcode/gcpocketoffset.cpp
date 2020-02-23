#include "gcpocketoffset.h"
#include "gcfile.h"

#include <QElapsedTimer>
namespace GCode {
PocketCreator::PocketCreator()
{
}

void PocketCreator::create()
{
    if (m_gcp.params[GCodeParams::TwoTools].toBool()) {
        createPocket2({ m_gcp.tools.first(), m_gcp.tools.last() }, m_gcp.params[GCodeParams::Depth].toDouble(), m_gcp.params[GCodeParams::MinArea].toDouble());
    } else {
        createPocket(m_gcp.tools.first(), m_gcp.params[GCodeParams::Depth].toDouble(), m_gcp.params[GCodeParams::Steps].toInt());
    }
}

void PocketCreator::createPocket(const Tool& tool, const double depth, const int steps)
{
    m_instance = this;
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
            Paths tmpPaths;
            do {
                tmpPaths.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, -m_stepOver);
            } while (paths.size());
            m_returnPs.append(tmpPaths);
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
        m_file->setFileName(tool.name());
        emit fileReady(m_file);
    }
}

void PocketCreator::createPocket2(const QPair<Tool, Tool>& tool, double depth, double minArea)
{
    m_instance = this;

    if (m_gcp.side() == On)
        return;

    do {
        m_toolDiameter = tool.second.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.second.stepover() * uScale;

        Paths fillPaths;

        switch (m_gcp.side()) {
        case On:
            break;
        case Outer:
            groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter + 5.0));
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
            break;
        }
        //            for (int i = 0; i < m_returnPaths.size(); ++i) {
        //                if (Perimeter(m_returnPaths[i]) < m_dOffset)
        //                    m_returnPaths.remove(i--);
        //            }
        //            for (int i = 0; i < fillPaths.size(); ++i) {
        //                if (Perimeter(fillPaths[i]) < m_dOffset)
        //                    fillPaths.remove(i--);
        //            }
        stacking(m_returnPs);
        if (m_returnPss.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_gcp.gcType = Pocket;
            m_gcp.params[GCodeParams::PocketIndex] = 1;
            m_file = new GCode::File(m_returnPss, m_gcp, fillPaths);
            m_file->setFileName(tool.second.name());
            emit fileReady(m_file);
        }
    } while (0);
    {
        m_returnPs.clear();

        m_toolDiameter = tool.first.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.first.stepover() * uScale;

        Paths fillPaths;

        for (Paths paths : m_groupedPss) {
            {
                double toolDiameter = tool.second.getDiameter(depth) * uScale;
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
                    offset.Execute(tmpPaths, dOffset - m_toolDiameter * 0.95);
                }
                if (m_gcp.side() != Inner)
                    ReversePaths(tmpPaths);
                paths.append(tmpPaths);
            }
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, -m_dOffset);
            for (int i = 0; i < paths.size(); ++i) {
                if (abs(Area(paths[i])) < minArea * uScale * uScale)
                    paths.remove(i--);
            }
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
            return;
        }

        for (int i = 0; i < m_returnPs.size(); ++i) {
            if (Perimeter(m_returnPs[i]) < m_dOffset)
                m_returnPs.remove(i--);
        }
        for (int i = 0; i < fillPaths.size(); ++i) {
            if (Perimeter(fillPaths[i]) < m_dOffset)
                fillPaths.remove(i--);
        }
        stacking(m_returnPs);
        if (m_returnPss.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_gcp.gcType = Pocket;
            m_gcp.params[GCodeParams::PocketIndex] = 0;
            m_file = new GCode::File(m_returnPss, m_gcp, fillPaths);
            m_file->setFileName(tool.first.name());
            emit fileReady(m_file);
        }
    }
}
}
