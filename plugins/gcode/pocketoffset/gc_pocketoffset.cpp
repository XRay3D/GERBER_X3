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
#include "gc_pocketoffset.h"
#include "gc_file.h"
#include "project.h"
#include "scene.h"
#include "settings.h"
#include <QDialog>
#include <QElapsedTimer>

namespace GCode {

void PocketCreator::create() {
    if (gcp_.tools.size() > 1) {
        createMultiTool(gcp_.tools, gcp_.params[GCodeParams::Depth].toDouble());
    } else if (gcp_.params.contains(GCodeParams::Steps) && gcp_.params[GCodeParams::Steps].toInt() > 0) {
        createFixedSteps(gcp_.tools.front(), gcp_.params[GCodeParams::Depth].toDouble(), gcp_.params[GCodeParams::Steps].toInt());
    } else {
        createStdFull(gcp_.tools.front(), gcp_.params[GCodeParams::Depth].toDouble());
    }
}

void PocketCreator::createFixedSteps(const Tool& tool, const double depth, const int steps) {

    if (gcp_.side() == On)
        return;
    // PROG //PROG .3setProgMaxAndVal(0, 0);
    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;

    groupedPaths(CopperPaths);

    if (gcp_.side() == Inner) {
        dOffset *= -1;
        for (Paths paths : groupedPss) {
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, dOffset);
            //            if (App::settings().gbrCleanPolygons())
            //                CleanPolygons(paths, uScale * 0.0005);
            Paths tmpPaths;
            int counter = steps;
            if (counter > 1) {
                do {
                    tmpPaths.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, dOffset);
                } while (paths.size() && --counter);
            } else {
                tmpPaths.append(paths);
            }
            returnPs.append(tmpPaths);
        }
    } else {
        ClipperOffset offset(uScale);
        for (Paths paths : groupedPss) {
            offset.AddPaths(paths, jtRound, etClosedPolygon);
        }
        Paths paths;
        offset.Execute(paths, dOffset);
        //        if (App::settings().gbrCleanPolygons())
        //            CleanPolygons(paths, uScale * 0.0005);
        int counter = steps;
        if (counter > 1) {
            do {
                returnPs.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, stepOver);
            } while (paths.size() && --counter);
        } else {
            returnPs.append(paths);
        }
    }

    if (returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    Paths fillPaths { returnPs };
    stacking(returnPs);
    if (returnPss.empty()) {
        emit fileReady(nullptr);
        return;
    }

    gcp_.gcType = Pocket;
    {
        ClipperOffset offset(uScale);
        offset.AddPaths(fillPaths, jtRound, etClosedLine);
        offset.Execute(fillPaths, dOffset + 10);
    }
    file_ = new GCode::File(returnPss, gcp_, fillPaths);
    file_->setFileName(tool.nameEnc());
    emit fileReady(file_);
}

void PocketCreator::createStdFull(const Tool& tool, const double depth) {

    if (gcp_.side() == On)
        return;
    // PROG //PROG .3setProgMaxAndVal(0, 0);
    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;
    Paths fillPaths;

    switch (gcp_.side()) {
    case On:
        break;
    case Outer:
        groupedPaths(CutoffPaths, static_cast<cInt>(toolDiameter * 1.005));
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    }

    setCurrent(0);
    for (Paths paths : groupedPss) {
        Clipper clipper;
        clipper.AddPaths(paths, ptClip);
        auto rect = clipper.GetBounds();
        setMax(max() + std::min(rect.right - rect.left, rect.bottom - rect.top));
    }

    setMax(max() / (stepOver * 2)); /////////////////////////

    qDebug() << "getMax" << max();
    for (Paths paths : groupedPss) {
        ClipperOffset offset(uScale);
        offset.AddPaths(paths, jtRound, etClosedPolygon);
        offset.Execute(paths, -dOffset);
        //        if (App::settings().gbrCleanPolygons())
        //            CleanPolygons(paths, uScale * 0.0005);
        fillPaths.append(paths);
        Paths offsetPaths;
        do {
            incCurrent(); ////////////////////
            getCancel();  ///////////
            offsetPaths.append(paths);
            offset.Clear();
            offset.AddPaths(paths, jtMiter, etClosedPolygon);
            offset.Execute(paths, -stepOver);
        } while (paths.size());
        returnPs.append(offsetPaths);
    }
    qDebug() << "getCurrent" << current();
    if (returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    stacking(returnPs);
    if (returnPss.empty()) {
        emit fileReady(nullptr);
        return;
    }

    gcp_.gcType = Pocket;
    {
        ClipperOffset offset(uScale);
        offset.AddPaths(fillPaths, jtRound, etClosedPolygon);
        offset.Execute(fillPaths, dOffset);
    }
    file_ = new GCode::File(returnPss, gcp_, fillPaths);
    file_->setFileName(tool.nameEnc());
    emit fileReady(file_);
}

void PocketCreator::createMultiTool(mvector<Tool>& tools, double depth) {

    switch (gcp_.side()) {
    case On:
        return;
    case Outer:
        groupedPaths(CutoffPaths, tools.front().getDiameter(depth) * 1.005 * uScale); // offset = 1mm
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    }

    auto removeSmall = [](Paths& paths, double dOffset) {
        // return;
        const auto ta = dOffset * dOffset * pi;
        const auto tp = dOffset * 4;
        for (size_t i = 0; i < paths.size(); ++i) {
            const auto a = abs(Area(paths[i]));
            const auto p = Perimeter(paths[i]);
            if (a < ta && p < tp)
                paths.remove(i--);
        }
    };

    int steps = 0;

    Pathss fillPaths;
    fillPaths.resize(tools.size());

    for (size_t tIdx = 0; tIdx < tools.size(); ++tIdx) {
        const Tool& tool = tools[tIdx];

        returnPs.clear();
        toolDiameter = tool.getDiameter(depth) * uScale;
        dOffset = toolDiameter / 2;
        stepOver = tool.stepover() * uScale;

        Paths clipFrame; // "обтравочная" рамка
        for (size_t i = 0; tIdx && i <= tIdx; ++i) {
            Paths tmp;
            { // "обтравочная" рамка для текущего инструмента и предыдущих УП
                ClipperOffset offset(uScale);
                offset.AddPaths(fillPaths[i], jtRound, etClosedPolygon);
                offset.Execute(tmp, -dOffset + uScale * 0.001);
                //                if (App::settings().gbrCleanPolygons())
                //                    CleanPolygons(tmp, uScale * 0.0005);
            }
            { // объединение рамок
                Clipper cliper;
                cliper.AddPaths(clipFrame, ptSubject, true);
                cliper.AddPaths(tmp, ptClip, true);
                cliper.Execute(ctUnion, clipFrame, pftEvenOdd);
            }
        }

        for (size_t pIdx = 0; pIdx < groupedPss.size(); ++pIdx) {

            if (gcp_.params[GCodeParams::Steps].toInt() > 0)
                steps = gcp_.params[GCodeParams::Steps].toInt();

            const Paths& paths = groupedPss[pIdx];
            Paths wp;
            ClipperOffset offset(uScale);
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(wp, -dOffset + 1); // + 1 <- поправка при расчёте впритык.
            //            if (App::settings().gbrCleanPolygons())
            //                CleanPolygons(wp, uScale * 0.0005);

            if (tIdx) { // обрезка текущего пути предыдущим
                Clipper cliper;
                cliper.AddPaths(wp, ptSubject, true);
                cliper.AddPaths(clipFrame, ptClip, true);
                cliper.Execute(ctDifference, wp, pftEvenOdd);
            }

            if (tIdx + 1 != tools.size())
                removeSmall(wp, dOffset * 2.0); // остальные
            else
                removeSmall(wp, dOffset * 0.5); // последний

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
                offset.Execute(wp, -stepOver);
            } while (wp.size());
            returnPs.append(offsetPaths);
        } // for (const Paths& paths : m_groupedPss) {

        if (returnPs.empty()) {
            emit fileReady(nullptr);
            continue;
        }
        stacking(returnPs);
        if (returnPss.empty()) {
            emit fileReady(nullptr);
            continue;
        }

        gcp_.gcType = Pocket;
        gcp_.params[GCodeParams::PocketIndex] = static_cast<int>(tIdx);

        { // make a fill box for the toolpath and create a file
            ClipperOffset offset(uScale);
            for (auto& paths : returnPss)
                offset.AddPaths(paths, jtRound, etClosedLine);
            Paths fillToolpath;
            offset.Execute(fillToolpath, dOffset);
            file_ = new GCode::File(returnPss, gcp_, fillToolpath);
            file_->setFileName(tool.nameEnc());
            // App::project()->addFile(m_file);
            emit fileReady(file_);
        }

        { // make a bounding box for the next tool
            ClipperOffset offset(uScale);
            offset.AddPaths(fillPaths[tIdx], jtRound, etClosedPolygon);
            offset.Execute(fillPaths[tIdx], dOffset);
        }

    } // for (int tIdx = 0; tIdx < tools.size(); ++tIdx) {
}
} // namespace GCode
