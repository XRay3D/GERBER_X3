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
#include "thermal.h"
#include "project.h"

namespace Thermal {

Creator::Creator() { }

void Creator::create() {
    qDebug(__FUNCTION__);
    createThermal(
        App::project().file(gcp_.params[FileId].toInt()),
        gcp_.tools.front(),
        gcp_.params[GCode::Params::Depth].toDouble());
}

void Creator::createThermal(AbstractFile* file, const Tool& tool, const double depth) {
    toolDiameter = tool.getDiameter(depth);
    const double dOffset = toolDiameter * uScale * 0.5;

    dbgPaths(closedSrcPaths, "closedSrcPaths");

    {     // create tool path
        { // execute offset
            // ClipperOffset offset;
            // offset.AddPaths(closedSrcPaths, JoinType::Round, EndType::Polygon);
            // returnPs = offset.Execute(dOffset);
            returnPs = InflateRoundPolygon(closedSrcPaths, dOffset * 2);
        }
        dbgPaths(returnPs, "returnPs");

        // fix direction
        if(gcp_.side() == GCode::Outer && !gcp_.convent())
            ReversePaths(returnPs);
        else if(gcp_.side() == GCode::Inner && gcp_.convent())
            ReversePaths(returnPs);

        for(Path& path: returnPs)
            path.push_back(path.front());

        if(returnPs.empty()) {
            emit fileReady(nullptr);
            return;
        }
    }

    Paths framePaths;
    { // create frame
        const auto graphicObjects(file->graphicObjects());
        Clipper clipper;
        {
            Clipper2Lib::ClipperOffset offset;
            for(auto go: graphicObjects | std::views::filter([](auto* go) { return go->positive(); }))
                offset.AddPaths(go->fill /*polyLineW()*/, JoinType::Round, EndType::Polygon);
            offset.Execute(dOffset - 0.005 * uScale, framePaths); // FIXME
            clipper.AddSubject(framePaths);
        }
        if(!gcp_.params[IgnoreCopper].toInt()) {
            Clipper2Lib::ClipperOffset offset;
            for(auto go: graphicObjects) {
                //                if (go->closed()) {
                //                    if (go->positive())
                offset.AddPaths(go->fill /*polygonWholes()*/, JoinType::Round, EndType::Polygon);
                //                    else {
                //                        Paths paths(go->polygonWholes());
                //                        ReversePaths(paths);
                //                        offset.AddPaths(paths, JoinType::Miter, EndType::Polygon);
                //                    }
                //                }
            }
            offset.Execute(dOffset - 0.005 * uScale, framePaths); // FIXME
            clipper.AddClip(framePaths);
        }
        for(const Paths& paths: supportPss)
            clipper.AddClip(paths);
        clipper.Execute(ClipType::Union, FillRule::EvenOdd, framePaths);
    }

    { // Execute
        Clipper clipper;
        clipper.AddOpenSubject(returnPs);
        clipper.AddClip(framePaths);
        clipper.Execute(ClipType::Difference, FillRule::Positive, framePaths, returnPs);
        sortBeginEnd(returnPs);
    }

    if(returnPs.size())
        returnPss.push_back(sortB(returnPs));

    if(returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        sortB(returnPss);
        file_ = new File{std::move(gcp_), std::move(returnPss)};
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    }
}

//////////////////////////////////////////////////////

File::File()
    : GCode::File() { }

File::File(GCode::Params&& gcp, Pathss&& toolPathss)
    : GCode::File(std::move(gcp), std::move(toolPathss)) {
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
                saveLaserProfile(offset);
            else
                saveMillingProfile(offset);

            if(gcp_.params.contains(GCode::Params::NotTile))
                return;
        }
    }
}

void File::createGi() {
    createGiProfile();
    itemGroup()->setVisible(true);
}
} // namespace Thermal
