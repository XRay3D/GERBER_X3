/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "voronoi.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "jc_voronoi.h"
#include "project.h"

// namespace ClipperLib {
// inline size_t qHash(const Point64& key, uint /*seed*/ = 0) { return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(Point64))); }
// } // namespace ClipperLib

namespace Voronoi {

// inline size_t qHash(const Creator::Pair& tag, uint = 0) {
// return ::qHash(tag.first.x ^ tag.second.x) ^ ::qHash(tag.first.y ^ tag.second.y);
// }

void Creator::create() {
    const auto& tool = gcp.tools.front();
    const auto depth = gcp.params[GCode::Params::Depth].toDouble();
    const auto width = gcp.params[Width].toDouble();

    groupedPaths(GCode::Grouping::Copper);
    switch(gcp.params[VoronoiType].toInt()) {
    case 0: boostVoronoi(); break;
    case 1: jcVoronoi(); break;
    }

    if(width < tool.getDiameter(depth)) {
        returnPs.resize(returnPs.size() - 1); // remove frame
        gcp.toolPathss = toCurvess(std::array{sortBeginEnd(returnPs, ~(App::home().pos() + App::zero().pos()))});
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    } else {
        Paths64 copy{returnPs};
        copy.resize(copy.size() - 1); // remove frame
        createOffset(tool, depth, width);

        { // создание пермычек.
            cl::Clipper64 clipper;
            clipper.AddClip(openSrcPaths);
            clipper.AddOpenSubject(copy);
            clipper.Execute(cl::ClipType::Difference, cl::FillRule::NonZero, copy, copy);
            sortBeginEnd(copy, ~(App::home().pos() + App::zero().pos()));
            for(auto&& p: copy)
                returnPss.emplace_back(Paths64{p});
        }
        dbgPaths(returnPs, u"создание пермычек"_s);
        { // создание заливки.
            cl::ClipperOffset offset(uScale);
            offset.AddPaths(openSrcPaths, cl::JoinType::Round, cl::EndType::Polygon);
            offset.AddPaths(copy, cl::JoinType::Round, cl::EndType::Round);
            offset.Execute(dOffset + 10, openSrcPaths); // FIXME maybe dOffset * 0.5
        }
        // erase empty paths
        auto begin = returnPss.begin();
        while(begin != returnPss.end())
            if(begin->empty())
                returnPss.erase(begin);
            else
                ++begin;
        gcp.toolPathss = toCurvess(returnPss);
        gcp.setPocketAreaCurves(toCurves(openSrcPaths));
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

void Creator::createOffset(const Tool& tool, double depth, const double width) {
    msg = tr("Create Offset");
    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;
    const Path64 frame{returnPs.back()};
    // returnPs.pop_back();
    { // create offset
      // ClipperOffset offset;
      // offset.AddPaths(returnPs, cl::JoinType::Round /*cl::JoinType::Miter*/, cl::EndType::Round);
      // returnPs = offset.Execute(width * uScale * 0.5);
        returnPs = Inflate64(returnPs, width * uScale * 0.5, cl::JoinType::Round /*cl::JoinType::Miter*/, cl::EndType::Round);
    }
    { // fit offset to copper
        cl::Clipper64 clipper;
        clipper.AddSubject(returnPs);
        for(const Paths64& paths: groupedPss)
            clipper.AddClip(paths);
        // clipper.Execute(ClipType::Difference, returnPs, FillRule::Positive, FillRule::Negative);
        clipper.Execute(cl::ClipType::Difference, cl::FillRule::Positive, returnPs);
    }
    if(0) { // cut to copper rect
        cl::Clipper64 clipper;
        clipper.AddSubject(returnPs);
        clipper.AddClip({frame});
        clipper.Execute(cl::ClipType::Intersection, cl::FillRule::NonZero, returnPs);
        CleanPaths(returnPs, 0.001 * uScale);
    }
    { // create pocket
        // ClipperOffset offset(uScale);
        // offset.AddPaths(returnPs, cl::JoinType::Round, cl::EndType::Polygon);
        // Paths64 tmpPaths1;
        // tmpPaths1 = offset.Execute(-dOffset);
        Paths64 tmpPaths1 = InflateRoundPolygon(returnPs, -dOffset);
        openSrcPaths = tmpPaths1;
        Paths64 tmpPaths;
        do {
            tmpPaths.append_range(tmpPaths1);
            // offset.Clear();
            // offset.AddPaths(tmpPaths1, cl::JoinType::Miter, cl::EndType::Polygon);
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

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        initSave();
        addInfo();
        statFile();
        genGcodeAndTile();
        endFile();
    }
}

void File::genGcodeAndTile() {
    const QRectF rect = App::project().worckRect();
    for(size_t x{}; x < App::project().stepsX(); ++x) {
        for(size_t y{}; y < App::project().stepsY(); ++y) {
            const QPointF offset((rect.width() + App::project().spaceX()) * x, (rect.height() + App::project().spaceY()) * y);

            if(toolType == Tool::Laser)
                if(gcp.toolPathss.size() > 1)
                    saveLaserPocket(offset);
                else
                    saveLaserProfile(offset);
            else if(gcp.toolPathss.size() > 1)
                saveMillingPocket(offset);
            else
                saveMillingProfile(offset);

            if(gcp.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    if(gcp.toolPathss.size() > 1) {
        Gi::Item* item;
        item = new Gi::GcPath{{gcp.toolPathss.back().back()}, this};
        // item->setPen(QPen(Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        // item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
        createGiPocket();
    } else
        createGiProfile();

    itemGroup()->setVisible(true);
}
} // namespace Voronoi
