// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "pocketoffset.h"
#include "gc_file.h"

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
            offset.AddPaths(paths, JoinType::Round, EndType::Polygon);
            paths = offset.Execute(dOffset);
            //            if (App::settings().gbrCleanPolygons())
            //                CleanPolygons(paths, uScale * 0.0005);
            Paths tmpPaths;
            int counter = steps;
            if (counter > 1) {
                do {
                    tmpPaths.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, JoinType::Miter, EndType::Polygon);
                    paths = offset.Execute(dOffset);
                } while (paths.size() && --counter);
            } else {
                tmpPaths.append(paths);
            }
            returnPs.append(tmpPaths);
        }
    } else {
        ClipperOffset offset(uScale);
        for (Paths paths : groupedPss) {
            offset.AddPaths(paths, JoinType::Round, EndType::Polygon);
        }
        Paths paths;
        paths = offset.Execute(dOffset);
        //        if (App::settings().gbrCleanPolygons())
        //            CleanPolygons(paths, uScale * 0.0005);
        int counter = steps;
        if (counter > 1) {
            do {
                returnPs.append(paths);
                offset.Clear();
                offset.AddPaths(paths, JoinType::Miter, EndType::Polygon);
                paths = offset.Execute(stepOver);
            } while (paths.size() && --counter);
        } else {
            returnPs.append(paths);
        }
    }

    if (returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    Paths fillPaths {returnPs};
    stacking(returnPs);
    if (returnPss.empty()) {
        emit fileReady(nullptr);
        return;
    }

    gcp_.gcType = Pocket;
    {
        ClipperOffset offset(uScale);
        offset.AddPaths(fillPaths, JoinType::Round, EndType::Round);
        fillPaths = offset.Execute(dOffset + 10);
    }
    file_ = new GCode::File(returnPss, std::move(gcp_), fillPaths);
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
        groupedPaths(CutoffPaths, static_cast<Point::Type>(toolDiameter * 1.005));
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    }

    setCurrent(0);
    for (Paths paths : groupedPss) {
        Clipper clipper;
        clipper.AddClip(paths); // FIXME Open???
        auto rect = Bounds(paths);
        setMax(max() + std::min(rect.right - rect.left, rect.bottom - rect.top));
    }

    setMax(max() / (stepOver * 2)); /////////////////////////

    for (Paths paths : groupedPss) {
        ClipperOffset offset(uScale);
        offset.AddPaths(paths, JoinType::Round, EndType::Polygon);
        paths = offset.Execute(-dOffset);
        //        if (App::settings().gbrCleanPolygons())
        //            CleanPolygons(paths, uScale * 0.0005);
        fillPaths.append(paths);
        Paths offsetPaths;
        do {
            incCurrent(); ////////////////////
            isCancel();   ///////////
            offsetPaths.append(paths);
            offset.Clear();
            offset.AddPaths(paths, JoinType::Miter, EndType::Polygon);
            paths = offset.Execute(-stepOver);
        } while (paths.size());
        returnPs.append(offsetPaths);
    }

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
        offset.AddPaths(fillPaths, JoinType::Round, EndType::Polygon);
        fillPaths = offset.Execute(dOffset);
    }
    file_ = new GCode::File(returnPss, std::move(gcp_), fillPaths);
    file_->setFileName(tool.nameEnc());
    emit fileReady(file_);
}

void PocketCreator::createMultiTool(const mvector<Tool>& tools, double depth) {

    switch (gcp_.side()) {
    case On:
        return;
    case Outer:
        groupedPaths(CutoffPaths, tools.front().getDiameter(depth) * 1.005 * uScale);
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    }

    auto removeSmall = [](Paths& paths, double dOffset) {
        const auto ta = dOffset * dOffset * pi;
        const auto tp = dOffset * 4;
        std::erase_if(paths, [ta, tp](auto& path) {
            const auto a = abs(Area(path));
            const auto p = Perimeter(path);
            return a < ta && p < tp;
        });
    };

    int steps = 0;

    Pathss fillPaths;
    fillPaths.resize(tools.size());

    for (int tIdx {}, size = tools.size(); tIdx < size; ++tIdx) {

        const Tool& tool = tools[tIdx];

        returnPs.clear();
        toolDiameter = tool.getDiameter(depth) * uScale;
        dOffset = toolDiameter / 2;
        stepOver = tool.stepover() * uScale;

        Paths clipFrame; // "обтравочная" рамка
        for (size_t i {}; tIdx && i <= tIdx; ++i) {
            Paths tmp;
            { // "обтравочная" рамка для текущего инструмента и предыдущих УП
                ClipperOffset offset(uScale);
                offset.AddPaths(fillPaths[i], JoinType::Round, EndType::Polygon);
                tmp = offset.Execute(-dOffset + uScale * 0.001);
                //                if (App::settings().gbrCleanPolygons())
                //                    CleanPolygons(tmp, uScale * 0.0005);
            }
            { // объединение рамок
                Clipper cliper;
                cliper.AddSubject(clipFrame);
                cliper.AddClip(tmp);
                cliper.Execute(ClipType::Union, FillRule::EvenOdd, clipFrame);
            }
        }

        for (size_t pIdx = 0; pIdx < groupedPss.size(); ++pIdx) {

            if (gcp_.params[GCodeParams::Steps].toInt() > 0)
                steps = gcp_.params[GCodeParams::Steps].toInt();

            const Paths& paths = groupedPss[pIdx];

            ClipperOffset offset(uScale);
            offset.AddPaths(paths, JoinType::Round, EndType::Polygon);
            Paths wp = offset.Execute(-dOffset + 1); // + 1 <- поправка при расчёте впритык.
            //            if (App::settings().gbrCleanPolygons())
            //                CleanPolygons(wp, uScale * 0.0005);

            if (tIdx) { // обрезка текущего пути предыдущим
                Clipper cliper;
                cliper.AddSubject(wp);
                cliper.AddClip(clipFrame);
                cliper.Execute(ClipType::Difference, FillRule::EvenOdd, wp);
            }

            if (tIdx + 1 != size)
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
                offset.AddPaths(wp, JoinType::Miter, EndType::Polygon);
                wp = offset.Execute(-stepOver);
            } while (wp.size());
            returnPs.append(offsetPaths);
        } // for (const Paths& paths : groupedPss_) {

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
        gcp_.params[GCodeParams::PocketIndex] = tIdx;

        { // make a fill box for the toolpath and create a file
            ClipperOffset offset(uScale);
            for (auto& paths : returnPss)
                offset.AddPaths(paths, JoinType::Round, EndType::Round);
            Paths fillToolpath;
            fillToolpath = offset.Execute(dOffset);
            file_ = new GCode::File(returnPss, GCodeParams {gcp_}, fillToolpath);
            file_->setFileName(tool.nameEnc());
            // App::project()->addFile(file_);
            emit fileReady(file_);
        }

        { // make a bounding box for the next tool
            ClipperOffset offset(uScale);
            offset.AddPaths(fillPaths[tIdx], JoinType::Round, EndType::Polygon);
            fillPaths[tIdx] = offset.Execute(dOffset);
        }

    } // for (int tIdx = 0; tIdx < tools.size(); ++tIdx) {
}

} // namespace GCode
