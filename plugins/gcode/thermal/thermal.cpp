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
#include "thermal.h"
#include "gi_point.h"
#include "project.h"

namespace Thermal {

Creator::Creator() { }

void Creator::create() {
    qDebug(__FUNCTION__);
    createThermal(
        App::project().file(gcp.params[FileId].toInt()),
        gcp.tools.front(),
        gcp.params[GCode::Params::Depth].toDouble());
}

void Creator::createThermal(AbstractFile* file, const Tool& tool, const double depth) {
    toolDiameter = tool.getDiameter(depth);
    const double dOffset = toolDiameter * uScale * 0.5;

    dbgPaths(closedSrcPaths, u"closedSrcPaths"_s);

    {     // create tool path
        { // execute offset
            // ClipperOffset offset;
            // offset.AddPaths(closedSrcPaths, cl::JoinType::Round, cl::EndType::Polygon);
            // returnPs = offset.Execute(dOffset);
            returnPs = InflateRoundPolygon(closedSrcPaths, dOffset * 2);
        }
        dbgPaths(returnPs, u"returnPs"_s);

        // fix direction
        if(gcp.side() == GCode::Outer && !gcp.convent())
            ReversePaths(returnPs);
        else if(gcp.side() == GCode::Inner && gcp.convent())
            ReversePaths(returnPs);

        for(Geo::Polyline& path: returnPs)
            path.push_back(path.front());

        if(returnPs.empty()) {
            emit fileReady(nullptr);
            return;
        }
    }

    Paths64 framePaths;
    { // create frame
        const auto graphicObjects(file->graphicObjects());
        cl::Clipper64 clipper;
        {
            cl::ClipperOffset offset;
            for(auto go: graphicObjects | v::filter([](auto* go) { return go->positive(); }))
                offset.AddPaths(toPaths(go->fill) /*polyLineW()*/, cl::JoinType::Round, cl::EndType::Polygon);
            offset.Execute(dOffset - 0.005 * uScale, framePaths); // FIXME
            clipper.AddSubject(framePaths);
        }
        if(!gcp.params[IgnoreCopper].toInt()) {
            cl::ClipperOffset offset;
            for(auto go: graphicObjects) {
                // if (go->closed()) {
                // if (go->positive())
                offset.AddPaths(toPaths(go->fill) /*polygonWholes()*/, cl::JoinType::Round, cl::EndType::Polygon);
                // else {
                // Paths64 paths(go->polygonWholes());
                // ReversePaths(paths);
                // offset.AddPaths(paths, cl::JoinType::Miter, cl::EndType::Polygon);
                // }
                // }
            }
            offset.Execute(dOffset - 0.005 * uScale, framePaths); // FIXME
            clipper.AddClip(framePaths);
        }
        for(const Paths64& paths: supportPss)
            clipper.AddClip(paths);
        clipper.Execute(cl::ClipType::Union, cl::FillRule::EvenOdd, framePaths);
    }

    { // Execute
        cl::Clipper64 clipper;
        clipper.AddOpenSubject(returnPs);
        clipper.AddClip(framePaths);
        clipper.Execute(cl::ClipType::Difference, cl::FillRule::Positive, framePaths, returnPs);
        sortBeginEnd(returnPs, ~(App::home().pos() + App::zero().pos()));
    }

    if(returnPs.size())
        returnPss.push_back(sortB(returnPs, ~(App::home().pos() + App::zero().pos())));

    if(returnPss.size()) {
        sortB(returnPss, ~(App::home().pos() + App::zero().pos()));
        gcp.toolPathss = toCurvess(returnPss);
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

//////////////////////////////////////////////////////

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
                saveLaserProfile(offset);
            else
                saveMillingProfile(offset);

            if(gcp.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    createGiProfile();
    itemGroup()->setVisible(true);
}
} // namespace Thermal
