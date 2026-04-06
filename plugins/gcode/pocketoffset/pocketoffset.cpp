/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "pocketoffset.h"
#include "project.h"

#include <gi_dbg.h>
// #include <QStringBuilder>

namespace PocketOffset {

// //dbgPaths(clipFrame, u"clipFrame %1"_s.arg(tIdx), Qt::red);

void Creator::create() {
    setMax(10000);

    assert(gcp.side() != GCode::On);

    if(this->gcp.tools.size() > 1)
        createMultiTool(gcp.tools, gcp.params[GCode::Params::Depth].toDouble());
    else if(gcp.params.contains(OffsetSteps) && gcp.params[OffsetSteps].toInt() > 0) // FIXME inside steps
        createFixedSteps(gcp.tools.front(), gcp.params[GCode::Params::Depth].toDouble(), gcp.params[OffsetSteps].toInt());
    else
        createStdFull(gcp.tools.front(), gcp.params[GCode::Params::Depth].toDouble());
}

void Creator::createFixedSteps(const Tool& tool, const double depth, int steps) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On)
        return;

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;

    Paths cutAreaPaths;

    auto calculate = [&cutAreaPaths, steps, this](Paths&& paths) {
        cutAreaPaths.append_range(Inflate(paths, -dOffset * 2, JoinType::Round, EndType::Polygon, uScale)); // inner
        int counter = steps;
        do {
            if(counter == 1)
                cutAreaPaths.append_range(Inflate(paths, dOffset * 2, JoinType::Round, EndType::Polygon, uScale)); // outer
            returnPs.append_range(paths);
            CleanPaths(paths, uScale * 0.001);
            paths = Inflate(paths, stepOver, JoinType::Miter, EndType::Polygon, uScale);
        } while(paths.size() && --counter);
    };

    if(gcp.side() == GCode::Inner) {
        dOffset = -dOffset, stepOver = -stepOver;
        for(Paths paths: groupedPaths(GCode::Grouping::Copper)) {
            paths = Inflate(paths, dOffset * 2, JoinType::Round, EndType::Polygon, uScale);
            if(paths.empty())
                continue;
            // if (App::settings().gbrCleanPolygons()) CleanPaths(paths, uScale * 0.0005);
            calculate(std::move(paths));
        }
    } else { // Outer
        Paths paths{Inflate(closedSrcPaths, +dOffset * 2, JoinType::Round, EndType::Polygon, uScale)};
        if(paths.empty()) {
            emit fileReady(nullptr);
            return;
        }
        calculate(std::move(paths));
    }

    if(returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }

    stacking(returnPs);
    assert(returnPss.size());

    gcp.setToolPathss(toCurvess(returnPss));
    gcp.setPocketAreaCurves(toCurves(cutAreaPaths));
    file_ = new File{std::move(gcp)};

    file_->setFileName(tool.nameEnc());
}

void Creator::createStdFull(const Tool& tool, const double depth) {
    Timer t{__FUNCTION__};

    if(gcp.side() == GCode::On)
        return;

    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;
    Paths cutAreaPaths;

    if(gcp.side() == GCode::Outer)
        groupedPaths(GCode::Grouping::Cutoff, static_cast</*PType*/ int32_t>(toolDiameter * 1.005));
    else // Inner:
        groupedPaths(GCode::Grouping::Copper);

    // dbgPaths(groupedPss, u"groupedPss"_s, Qt::red);

    setCurrent(0);

    for(Paths paths: groupedPss) {
        paths = InflateRoundPolygon(paths, -dOffset * 2, uScale);
        // if (App::settings().gbrCleanPolygons()) CleanPaths(paths, uScale * 0.0005);
        CleanPaths(paths, uScale * 0.001);
        cutAreaPaths.append_range(paths);

        returnPs.append_range(paths);
        double step{};
        Paths tmp;
        do {
            tmp = InflateMiterPolygon(paths, step += -stepOver * 2, uScale);
            returnPs.append_range(tmp);
        } while(tmp.size());
    }

    if(returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }

    stacking(returnPs);

    assert(returnPss.size());

    cutAreaPaths = InflateRoundPolygon(cutAreaPaths, dOffset * 2, uScale);

    gcp.setToolPathss(toCurvess(returnPss));
    gcp.setPocketAreaCurves(toCurves(cutAreaPaths));
    file_ = new File{std::move(gcp)};

    file_->setFileName(tool.nameEnc());
}

void Creator::createMultiTool(const mvector<Tool>& tools, double depth) {

    if(gcp.side() == GCode::Outer)
        groupedPaths(GCode::Grouping::Cutoff, tools.front().getDiameter(depth) * 1.005 * uScale);
    else // Inner:
        groupedPaths(GCode::Grouping::Copper);

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

    for(size_t tIdx{}, size = tools.size(); const auto& tool: tools) {
        returnPs.clear();
        toolDiameter = tool.getDiameter(depth) * uScale;
        dOffset = toolDiameter / 2;
        stepOver = tool.stepover() * uScale;

        Paths clipFrame; // u"обтравочная"_s рамка
        for(size_t i{}; tIdx && i <= tIdx; ++i) {
            // u"обтравочная"_s рамка для текущего инструмента и предыдущих УП
            Paths tmp = Inflate(fillPaths[i], -dOffset + uScale * 0.001, JoinType::Round, EndType::Polygon, uScale);
            // объединение рамок
            clipFrame = CL2::Union(clipFrame, tmp, FillRule::EvenOdd);
        }

        Paths cutAreaPaths;

        {
            Timer t{"groupedPss"};
            for(size_t pIdx{}; const Paths& paths: groupedPss) {
                Paths wp = Inflate(paths, -dOffset + 2, JoinType::Round, EndType::Polygon, uScale); // + 2 <- поправка при расчёте впритык.

                if(tIdx) // обрезка текущего пути предыдущим
                    wp = CL2::Difference(wp, clipFrame, FillRule::EvenOdd);

                if(tIdx == size - 1)
                    removeSmall(wp, dOffset * 0.5); // последний
                else if(tIdx /*+ 1 != size*/)
                    removeSmall(wp, dOffset * 2.0); // остальные

                // //dbgPaths(wp, u"wp %1"_s.arg(pIdx), Qt::red);

                fillPaths[tIdx].append_range(wp);

                cutAreaPaths.append_range(wp);

                do {
                    returnPs.append_range(std::move(wp));
                    CleanPaths(wp, uScale * 0.0005); //-V1030
                    wp = Inflate(wp, -stepOver * 2, JoinType::Miter, EndType::Polygon, uScale);
                } while(wp.size());
                ++pIdx;
            } // for (const Paths& paths : groupedPss_) {
        }

        if(returnPs.empty()) {
            emit fileReady(nullptr);
            continue;
        }

        // make a fill box for the toolpath and create a file
        Timer t{"cutAreaPaths"};
        // //dbgPaths(cutAreaPaths, u"cutAreaPaths"_s, Qt::green);
        cutAreaPaths = Inflate(cutAreaPaths, dOffset * 2, JoinType::Round, EndType::Polygon, uScale);

        stacking(returnPs);
        assert(returnPss.size());

        gcp.params[GCode::Params::MultiToolIndex] = static_cast<ssize_t>(tIdx);

        gcp.setToolPathss(toCurvess(returnPss));
        gcp.setPocketAreaCurves(toCurves(cutAreaPaths));
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());

        // make a bounding box for the next tool
        fillPaths[tIdx] = Inflate(fillPaths[tIdx], dOffset * 2, JoinType::Round, EndType::Polygon, uScale);

        if(++tIdx < tools.size()) emit fileReady(file_); // NOTE skip last
    } // for (int tIdx{}; tIdx < tools.size(); ++tIdx) {
}

File::File()
    : GCode::File() { }

File::File(GCode::Params&& gcp)
    : GCode::File{std::move(gcp)} {
    if(this->gcp.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    auto& proj = App::project();
    const QRectF rect = proj.worckRect();
    for(size_t x{}; x < proj.stepsX(); ++x) {
        for(size_t y{}; y < proj.stepsY(); ++y) {
            const QPointF offset{(rect.width() + proj.spaceX()) * x, (rect.height() + proj.spaceY()) * y};
            if(toolType == Tool::Laser)
                saveLaserPocket(offset);
            else
                saveMillingPocket(offset);

            if(gcp.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    // switch (gcp.gcType) {
    // case GCode::Raster:
    // createGiRaster();
    // break;
    // case GCode::Pocket:
    createGiPocket();
    // break;
    // default: break;
    // }

    itemGroup()->setVisible(true);
}

} // namespace PocketOffset
