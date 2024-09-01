// This is a personal academic project. Dear PVS-Studio, please check it.
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
#include "voronoi.h"
#include "gi_gcpath.h"
#include "jc_voronoi.h"
#include "project.h"

// namespace ClipperLib {
// inline size_t qHash(const Point& key, uint /*seed*/ = 0) { return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(Point))); }
// } // namespace ClipperLib

namespace Voronoi {

inline size_t qHash(const Creator::Pair& tag, uint = 0) {
    return ::qHash(tag.first.x ^ tag.second.x) ^ ::qHash(tag.first.y ^ tag.second.y);
}

void Creator::create() {
    const auto& tool = gcp_.tools.front();
    const auto depth = gcp_.params[GCode::Params::Depth].toDouble();
    const auto width = gcp_.params[Width].toDouble();

    groupedPaths(GCode::Grouping::Copper);
    switch(gcp_.params[VoronoiType].toInt()) {
    case 0:
        boostVoronoi();
        break;
    case 1:
        jcVoronoi();
        break;
    }

    if(width < tool.getDiameter(depth)) {
        returnPs.resize(returnPs.size() - 1); // remove frame

        file_ = new File{std::move(gcp_), {sortBeginEnd(returnPs)}, {}};
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    } else {
        Paths copy{returnPs};
        copy.resize(copy.size() - 1); // remove frame
        createOffset(tool, depth, width);

        { // создание пермычек.
            Clipper clipper;
            clipper.AddClip(openSrcPaths);
            clipper.AddOpenSubject(copy);
            clipper.Execute(ClipType::Difference, FillRule::NonZero, copy, copy);
            sortBeginEnd(copy);
            for(auto&& p: copy)
                returnPss.emplace_back(Paths{p});
        }
        dbgPaths(returnPs, "создание пермычек");
        { // создание заливки.
            Clipper2Lib::ClipperOffset offset(uScale);
            offset.AddPaths(openSrcPaths, JoinType::Round, EndType::Polygon);
            offset.AddPaths(copy, JoinType::Round, EndType::Round);
            offset.Execute(dOffset + 10, openSrcPaths); // FIXME maybe dOffset * 0.5
        }
        // erase empty paths
        auto begin = returnPss.begin();
        while(begin != returnPss.end())
            if(begin->empty())
                returnPss.erase(begin);
            else
                ++begin;

        file_ = new File{std::move(gcp_), std::move(returnPss), std::move(openSrcPaths)};
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    }
}

void Creator::createOffset(const Tool& tool, double depth, const double width) {
    msg = tr("Create Offset");
    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;
    const Path frame{returnPs.back()};
    // returnPs.pop_back();
    { // create offset
      // ClipperOffset offset;
      // offset.AddPaths(returnPs, JoinType::Round /*JoinType::Miter*/, EndType::Round);
      // returnPs = offset.Execute(width * uScale * 0.5);
        returnPs = Inflate(returnPs, width * uScale * 0.5, JoinType::Round /*JoinType::Miter*/, EndType::Round);
    }
    { // fit offset to copper
        Clipper clipper;
        clipper.AddSubject(returnPs);
        for(const Paths& paths: groupedPss)
            clipper.AddClip(paths);
        // clipper.Execute(ClipType::Difference, returnPs, FillRule::Positive, FillRule::Negative);
        clipper.Execute(ClipType::Difference, FillRule::Positive, returnPs);
    }
    if(0) { // cut to copper rect
        Clipper clipper;
        clipper.AddSubject(returnPs);
        clipper.AddClip({frame});
        clipper.Execute(ClipType::Intersection, FillRule::NonZero, returnPs);
        CleanPaths(returnPs, 0.001 * uScale);
    }
    { // create pocket
        // ClipperOffset offset(uScale);
        // offset.AddPaths(returnPs, JoinType::Round, EndType::Polygon);
        // Paths tmpPaths1;
        // tmpPaths1 = offset.Execute(-dOffset);
        Paths tmpPaths1 = InflateRoundPolygon(returnPs, -dOffset);
        openSrcPaths = tmpPaths1;
        Paths tmpPaths;
        do {
            tmpPaths += tmpPaths1;
            // offset.Clear();
            // offset.AddPaths(tmpPaths1, JoinType::Miter, EndType::Polygon);
            // tmpPaths1 = offset.Execute(-stepOver);
            tmpPaths1 = InflateMiterPolygon(tmpPaths1, -stepOver);
        } while(tmpPaths1.size());
        returnPs = tmpPaths;
    }
    if(returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    stacking(returnPs);
    // returnPss_.push_back({frame});
}

/////////////////////////////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& gcp, Pathss&& toolPathss, Paths&& pocketPaths)
    : GCode::File(std::move(gcp), std::move(toolPathss), std::move(pocketPaths)) {
    if(gcp_.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project().worckRect();
    for(size_t x = 0; x < App::project().stepsX(); ++x) {
        for(size_t y = 0; y < App::project().stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);

            if(toolType() == Tool::Laser)
                if(toolPathss_.size() > 1)
                    saveLaserPocket(offset);
                else
                    saveLaserProfile(offset);
            else if(toolPathss_.size() > 1)
                saveMillingPocket(offset);
            else
                saveMillingProfile(offset);

            if(gcp_.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    if(toolPathss_.size() > 1) {
        Gi::Item* item;
        item = new Gi::GcPath{toolPathss_.back().back(), this};
        item->setPen(QPen(Qt::black, gcp_.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
        createGiPocket();
    } else
        createGiProfile();

    itemGroup()->setVisible(true);
}
} // namespace Voronoi
