// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "pocketoffset.h"
#include <QStringBuilder>

namespace GCode {

// dbgPaths(clipFrame, QString("clipFrame %1").arg(tIdx), Qt::red);

void PocketCtr::create() {
    setMax(10000);

    assert(gcp_.side() != On);

    if (gcp_.tools.size() > 1) {
        createMultiTool(gcp_.tools, gcp_.params[GCode::Params::Depth].toDouble());
    } else if (gcp_.params.contains(OffsetSteps) && gcp_.params[OffsetSteps].toInt() > 0) {
        createFixedSteps(gcp_.tools.front(), gcp_.params[GCode::Params::Depth].toDouble(), gcp_.params[OffsetSteps].toInt());
    } else {
        createStdFull(gcp_.tools.front(), gcp_.params[GCode::Params::Depth].toDouble());
    }
}

void PocketCtr::createFixedSteps(const Tool& tool, const double depth, int steps) {
    Timer t {__FUNCTION__};
    if (gcp_.side() == On)
        return;

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;

    Paths cutAreaPaths;

    auto calculate = [&cutAreaPaths, steps, this](Paths&& paths) {
        cutAreaPaths.append(InflatePaths(paths, -dOffset, JT::Round, ET::Polygon, uScale)); // inner
        int counter = steps;
        do {
            if (counter == 1)
                cutAreaPaths.append(InflatePaths(paths, dOffset, JT::Round, ET::Polygon, uScale)); // outer
            returnPs.append(paths);
            CleanPaths(paths, uScale * 0.001);
            paths = InflatePaths(paths, stepOver, JT::Miter, ET::Polygon, uScale);
        } while (paths.size() && --counter);
    };

    if (gcp_.side() == Inner) {
        dOffset = -dOffset, stepOver = -stepOver;
        for (Paths paths : groupedPaths(Grouping::Copper)) {
            paths = InflatePaths(paths, dOffset, JT::Round, ET::Polygon, uScale);
            if (paths.empty())
                continue;
            // if (App::settings().gbrCleanPolygons()) CleanPaths(paths, uScale * 0.0005);
            calculate(std::move(paths));
        }
    } else { // Outer
        Paths paths {InflatePaths(workingPs, +dOffset, JT::Round, ET::Polygon, uScale)};
        if (paths.empty()) {
            emit fileReady(nullptr);
            return;
        }
        calculate(std::move(paths));
    }

    if (returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }

    stacking(returnPs);
    assert(returnPss.size());

    file_ = new PocketOffsetFile(std::move(gcp_), std::move(returnPss), std::move(cutAreaPaths));
    file_->setFileName(tool.nameEnc());
    emit fileReady(file_);
}

void PocketCtr::createStdFull(const Tool& tool, const double depth) {
    Timer t {__FUNCTION__};

    if (gcp_.side() == On)
        return;

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;
    Paths cutAreaPaths;

    if (gcp_.side() == Outer)
        groupedPaths(Grouping::Cutoff, static_cast<Point::Type>(toolDiameter * 1.005));
    else // Inner:
        groupedPaths(Grouping::Copper);

    dbgPaths(groupedPss, "groupedPss", Qt::red);

    setCurrent(0);

    for (Paths paths : groupedPss) {
        paths = InflatePaths(paths, -dOffset, JT::Round, ET::Polygon, uScale);
        // if (App::settings().gbrCleanPolygons()) CleanPaths(paths, uScale * 0.0005);
        cutAreaPaths.append(paths);
        do {
            CleanPaths(paths, uScale * 0.001);
            returnPs.append(paths);
            paths = InflatePaths(paths, -stepOver, JT::Miter, ET::Polygon, uScale);
        } while (paths.size());
    }

    if (returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }

    stacking(returnPs);
    assert(returnPss.size());

    cutAreaPaths = InflatePaths(cutAreaPaths, dOffset, JT::Round, ET::Polygon, uScale);

    file_ = new PocketOffsetFile(std::move(gcp_), std::move(returnPss), std::move(cutAreaPaths));
    file_->setFileName(tool.nameEnc());
    emit fileReady(file_);
}

void PocketCtr::createMultiTool(const mvector<Tool>& tools, double depth) {

    if (gcp_.side() == Outer)
        groupedPaths(Grouping::Cutoff, tools.front().getDiameter(depth) * 1.005 * uScale);
    else // Inner:
        groupedPaths(Grouping::Copper);

    auto removeSmall = [](Paths& paths, double dOffset) {
        const auto ta = dOffset * dOffset * pi;
        const auto tp = dOffset * 4;
        std::erase_if(paths, [ta, tp](auto& path) {
            const auto a = abs(Area(path));
            const auto p = Perimeter(path);
            return a < ta && p < tp;
        });
    };

    Pathss fillPaths;
    fillPaths.resize(tools.size());

    for (int tIdx {}, size = tools.size(); const auto& tool : tools) {
        returnPs.clear();
        toolDiameter = tool.getDiameter(depth) * uScale;
        dOffset = toolDiameter / 2;
        stepOver = tool.stepover() * uScale;

        Paths clipFrame; // "обтравочная" рамка
        for (size_t i {}; tIdx && i <= tIdx; ++i) {
            // "обтравочная" рамка для текущего инструмента и предыдущих УП
            Paths tmp = InflatePaths(fillPaths[i], -dOffset + uScale * 0.001, JT::Round, ET::Polygon, uScale);
            // объединение рамок
            clipFrame = C2::Union(clipFrame, tmp, FR::EvenOdd);
        }

        Paths cutAreaPaths;

        {
            Timer t {"groupedPss"};
            for (size_t pIdx {}; const Paths& paths : groupedPss) {
                Paths wp = InflatePaths(paths, -dOffset + 2, JT::Round, ET::Polygon, uScale); // + 2 <- поправка при расчёте впритык.

                if (tIdx) // обрезка текущего пути предыдущим
                    wp = C2::Difference(wp, clipFrame, FR::EvenOdd);

                if (tIdx == size - 1)
                    removeSmall(wp, dOffset * 0.5); // последний
                else if (tIdx /*+ 1 != size*/)
                    removeSmall(wp, dOffset * 2.0); // остальные

                // dbgPaths(wp, QString("wp %1").arg(pIdx), Qt::red);

                fillPaths[tIdx].append(wp);

                cutAreaPaths.append(wp);

                do {
                    returnPs.append(wp);
                    CleanPaths(wp, uScale * 0.0005);
                    wp = InflatePaths(wp, -stepOver, JT::Miter, ET::Polygon, uScale);
                } while (wp.size());
                ++pIdx;
            } // for (const Paths& paths : groupedPss_) {
        }

        if (returnPs.empty()) {
            emit fileReady(nullptr);
            continue;
        }

        // make a fill box for the toolpath and create a file
        Timer t {"cutAreaPaths"};
        // dbgPaths(cutAreaPaths, "cutAreaPaths", Qt::green);
        cutAreaPaths = InflatePaths(cutAreaPaths, dOffset, JT::Round, ET::Polygon, uScale);

        stacking(returnPs);
        assert(returnPss.size());

        gcp_.params[GCode::Params::MultiToolIndex] = tIdx;

        file_ = new PocketOffsetFile(GCode::Params {gcp_}, std::move(returnPss), std::move(cutAreaPaths));
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);

        // make a bounding box for the next tool
        fillPaths[tIdx] = InflatePaths(fillPaths[tIdx], dOffset, JT::Round, ET::Polygon, uScale);
        ++tIdx;
    } // for (int tIdx = 0; tIdx < tools.size(); ++tIdx) {
}

} // namespace GCode
