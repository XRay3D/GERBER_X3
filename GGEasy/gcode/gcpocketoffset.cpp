// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcpocketoffset.h"
#include "gcfile.h"
#include "project.h"
#include "scene.h"
#include "settings.h"
#include <QDialog>
#include <QElapsedTimer>

#include "leakdetector.h"

namespace GCode {
PocketCreator::PocketCreator()
{
}

void PocketCreator::create()
{
    if (m_gcp.tools.count() > 1) {
        createMultiTool(m_gcp.tools, m_gcp.params[GCodeParams::Depth].toDouble());
    } else if (m_gcp.params.contains(GCodeParams::Steps) && m_gcp.params[GCodeParams::Steps].toInt() > 0) {
        createFixedSteps(m_gcp.tools.first(), m_gcp.params[GCodeParams::Depth].toDouble(), m_gcp.params[GCodeParams::Steps].toInt());
    } else {
        createStdFull(m_gcp.tools.first(), m_gcp.params[GCodeParams::Depth].toDouble());
    }
}

void PocketCreator::createFixedSteps(const Tool& tool, const double depth, const int steps)
{
    App::m_app->m_creator = this;
    if (m_gcp.side() == On)
        return;
    progress(0, 0);
    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;

    groupedPaths(CopperPaths);

    if (m_gcp.side() == Inner) {
        m_dOffset *= -1;
        for (Paths paths : m_groupedPss) {
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, m_dOffset);
            if (AppSettings::gbrCleanPolygons())
                CleanPolygons(paths, uScale * 0.0005);
            Paths tmpPaths;
            int counter = steps;
            if (counter > 1) {
                do {
                    tmpPaths.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, m_dOffset);
                } while (paths.size() && --counter);
            } else {
                tmpPaths.append(paths);
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
        if (AppSettings::gbrCleanPolygons())
            CleanPolygons(paths, uScale * 0.0005);
        int counter = steps;
        if (counter > 1) {
            do {
                m_returnPs.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, m_stepOver);
            } while (paths.size() && --counter);
        } else {
            m_returnPs.append(paths);
        }
    }

    if (m_returnPs.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }
    Paths fillPaths { m_returnPs };
    stacking(m_returnPs);
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }

    m_gcp.gcType = Pocket;
    {
        ClipperOffset offset(uScale);
        offset.AddPaths(fillPaths, jtRound, etClosedLine);
        offset.Execute(fillPaths, m_dOffset + 10);
    }
    m_file = new GCode::File(m_returnPss, m_gcp, fillPaths);
    m_file->setFileName(tool.nameEnc());
    emit fileReady(m_file);
}

void PocketCreator::createStdFull(const Tool& tool, const double depth)
{
    App::m_app->m_creator = this;
    if (m_gcp.side() == On)
        return;
    progress(0, 0);
    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;
    Paths fillPaths;

    switch (m_gcp.side()) {
    case On:
        break;
    case Outer:
        groupedPaths(CutoffPaths, static_cast<cInt>(m_toolDiameter * 1.005));
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    }

    for (Paths paths : m_groupedPss) {
        ClipperOffset offset(uScale);
        offset.AddPaths(paths, jtRound, etClosedPolygon);
        offset.Execute(paths, -m_dOffset);
        if (AppSettings::gbrCleanPolygons())
            CleanPolygons(paths, uScale * 0.0005);
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

    if (m_returnPs.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }
    stacking(m_returnPs);
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }

    m_gcp.gcType = Pocket;
    {
        ClipperOffset offset(uScale);
        offset.AddPaths(fillPaths, jtRound, etClosedPolygon);
        offset.Execute(fillPaths, m_dOffset);
    }
    m_file = new GCode::File(m_returnPss, m_gcp, fillPaths);
    m_file->setFileName(tool.nameEnc());
    emit fileReady(m_file);
}

void PocketCreator::createMultiTool(QVector<Tool>& tools, double depth)
{
    App::m_app->m_creator = this;

    switch (m_gcp.side()) {
    case On:
        return;
    case Outer:
        groupedPaths(CutoffPaths, tools.first().getDiameter(depth) * 1.005 * uScale); // offset = 1mm
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    }

    auto removeSmall = [](Paths& paths, double dOffset) {
        //return;
        const auto ta = dOffset * dOffset * M_PI;
        const auto tp = dOffset * 4;
        for (int i = 0; i < paths.size(); ++i) {
            const auto a = abs(Area(paths[i]));
            const auto p = Perimeter(paths[i]);
            if (a < ta && p < tp)
                paths.remove(i--);
        }
    };

    int steps = 0;

    Pathss fillPaths;
    fillPaths.resize(tools.size());

    for (int tIdx = 0; tIdx < tools.size(); ++tIdx) {
        const Tool& tool = tools[tIdx];

        m_returnPs.clear();
        m_toolDiameter = tool.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.stepover() * uScale;

        Paths clipFrame; // "обтравочная" рамка
        for (int i = 0; tIdx && i <= tIdx; ++i) {
            Paths tmp;
            { // "обтравочная" рамка для текущего инструмента и предыдущих УП
                ClipperOffset offset(uScale);
                offset.AddPaths(fillPaths[i], jtRound, etClosedPolygon);
                offset.Execute(tmp, -m_dOffset + uScale * 0.001);
                if (AppSettings::gbrCleanPolygons())
                    CleanPolygons(tmp, uScale * 0.0005);
            }
            { // объединение рамок
                Clipper cliper;
                cliper.AddPaths(clipFrame, ptSubject, true);
                cliper.AddPaths(tmp, ptClip, true);
                cliper.Execute(ctUnion, clipFrame, pftEvenOdd);
            }
        }

        for (int pIdx = 0; pIdx < m_groupedPss.size(); ++pIdx) {

            if (m_gcp.params[GCodeParams::Steps].toInt() > 0)
                steps = m_gcp.params[GCodeParams::Steps].toInt();

            const Paths& paths = m_groupedPss[pIdx];
            Paths wp;
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(wp, -m_dOffset + 1); // + 1 <- поправка при расчёте впритык.
            if (AppSettings::gbrCleanPolygons())
                CleanPolygons(wp, uScale * 0.0005);

            if (tIdx) { // обрезка текущего пути предыдущим
                Clipper cliper;
                cliper.AddPaths(wp, ptSubject, true);
                cliper.AddPaths(clipFrame, ptClip, true);
                cliper.Execute(ctDifference, wp, pftEvenOdd);
            }

            if (tIdx + 1 != tools.size())
                removeSmall(wp, m_dOffset * 2.0); // остальные
            else
                removeSmall(wp, m_dOffset * 0.5); // последний

            fillPaths[tIdx].append(wp);

            Paths offsetPaths;
            do {
                offsetPaths.append(wp);
                if (steps && !tIdx) {
                    if (--steps == 0)
                        break;
                }

                offset.Clear();
                offset.AddPaths(wp, jtMiter, etClosedPolygon);
                offset.Execute(wp, -m_stepOver);
            } while (wp.size());
            m_returnPs.append(offsetPaths);
        } // for (const Paths& paths : m_groupedPss) {

        if (m_returnPs.isEmpty()) {
            emit fileReady(nullptr);
            continue;
        }
        stacking(m_returnPs);
        if (m_returnPss.isEmpty()) {
            emit fileReady(nullptr);
            continue;
        }

        m_gcp.gcType = Pocket;
        m_gcp.params[GCodeParams::PocketIndex] = tIdx;

        { // make a fill box for the toolpath and create a file
            ClipperOffset offset(uScale);
            for (auto& paths : m_returnPss)
                offset.AddPaths(paths, jtRound, etClosedLine);
            Paths fillToolpath;
            offset.Execute(fillToolpath, m_dOffset);
            m_file = new GCode::File(m_returnPss, m_gcp, fillToolpath);
            m_file->setFileName(tool.nameEnc());
            //App::project()->addFile(m_file);
            emit fileReady(m_file);
        }

        { // make a bounding box for the next tool
            ClipperOffset offset(uScale);
            offset.AddPaths(fillPaths[tIdx], jtRound, etClosedPolygon);
            offset.Execute(fillPaths[tIdx], m_dOffset);
        }

    } // for (int tIdx = 0; tIdx < tools.size(); ++tIdx) {
}
}
