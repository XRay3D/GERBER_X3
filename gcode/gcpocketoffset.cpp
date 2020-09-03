// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "gcpocketoffset.h"
#include "gcfile.h"
#include "project.h"
#include "scene.h"
#include <QDialog>
#include <QElapsedTimer>
#include <settings.h>

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
    App::m_creator = this;
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
            if (GlobalSettings::gbrCleanPolygons())
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
        if (GlobalSettings::gbrCleanPolygons())
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
    App::m_creator = this;
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
        if (GlobalSettings::gbrCleanPolygons())
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
    App::m_creator = this;

    switch (m_gcp.side()) {
    case On:
        return;
    case Outer:
        groupedPaths(CutoffPaths, tools.first().getDiameter(depth) * 1.005 * uScale); // offset = 1mm
        if (m_groupedPss.size() > 1 && m_groupedPss.first().size() == 2)
            m_groupedPss.removeFirst();
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

    {
        Paths wpe;
        const double d = tools.last().getDiameter(depth) * uScale;
        const double r = d * 0.5;
        const double ta = d * d - M_PI * r * r;
        for (int pIdx = 0; pIdx < m_groupedPss.size(); ++pIdx) {
            const Paths& paths = m_groupedPss[pIdx];
            Paths wp;
            {
                ClipperOffset offset(uScale);
                offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(wp, -r);
            }
            if (GlobalSettings::gbrCleanPolygons())
                CleanPolygons(wp, uScale * 0.0005);
            {
                ClipperOffset offset(uScale);
                offset.AddPaths(wp, jtRound, etClosedPolygon);
                offset.Execute(wp, r + 100);
            }
            {
                Clipper clipper;
                clipper.AddPaths(wp, ptClip);
                clipper.AddPaths(paths, ptSubject);
                clipper.Execute(ctDifference, wp, pftPositive);
            }
            for (int i = 0; i < wp.size(); ++i) {
                if (ta > abs(Area(wp[i])))
                    wp.remove(i--);
                //App::scene()->addPolygon(toQPolygon(wp[i]), Qt::NoPen, Qt::red);
            }
            //            dbgPaths(wp, "wp2");
            wpe.append(wp);
        }
        if (!wpe.isEmpty()) {
            GCode::GCodeParams gcp { {}, 0.0, GCode::Pocket };
            //                m_returnPss.resize(wp.size());
            //                for (int i = 0; i < wp.size(); ++i) {
            //                    m_returnPss[i] = { std::move(wp[i]) };
            //                }
            auto file = new GCode::File({} /*m_returnPss*/, gcp, wpe);
            file->setFileName("Errors");
            file->itemGroup()->setBrushColor(new QColor(Qt::red));
            file->itemGroup()->setPen(QPen(QColor(Qt::red), 0.0));
            //                file->itemGroup()->setBrush(QColor(Qt::red));
            //                file->itemGroup()->setPenColor(new QColor(Qt::red));
            //                QDialog d((QWidget*)parent());
            //                d.setWindowFlag(Qt::WindowStaysOnTopHint);
            //                d.setWindowModality(Qt::NonModal);
            //                if (1 || !d.exec()) {
            for (int i = 0; i < tools.size() - 1; ++i) {
                emit fileReady(nullptr);
            }
            emit fileReady(file);
            return;
            //                }
        }
    }

    qDebug() << tools;

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
                if (GlobalSettings::gbrCleanPolygons())
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
            const Paths& paths = m_groupedPss[pIdx];
            Paths wp;
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(wp, -m_dOffset);
            if (GlobalSettings::gbrCleanPolygons())
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

        {
            ClipperOffset offset(uScale);
            offset.AddPaths(fillPaths[tIdx], jtRound, etClosedPolygon);
            offset.Execute(fillPaths[tIdx], m_dOffset);
        }

        m_file = new GCode::File(m_returnPss, m_gcp, fillPaths[tIdx]);
        m_file->setFileName(tool.nameEnc());
        //App::project()->addFile(m_file);
        emit fileReady(m_file);

    } // for (int tIdx = 0; tIdx < tools.size(); ++tIdx) {
    //    {
    //        ClipperOffset offset(uScale);
    //        offset.AddPaths(testFrame, jtRound, etClosedPolygon);
    //        offset.Execute(testFrame, tools.last().getDiameter(depth) * uScale * 0.6);
    //    }
}
}
