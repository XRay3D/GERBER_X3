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
#include "file.h"
#include "project.h"

namespace Thermal {

Creator::Creator() { }

void Creator::create() {
    createThermal(
        App::project()->file(gcp_.params[::GCode::GCodeParams::FileId].toInt()),
        gcp_.tools.front(),
        gcp_.params[::GCode::GCodeParams::Depth].toDouble());
}

void Creator::createThermal(AbstractFile* file, const Tool& tool, const double depth) {
    toolDiameter = tool.getDiameter(depth);
    const double dOffset = toolDiameter * uScale * 0.5;

    {     // create tool path
        { // execute offset
            ClipperOffset offset;
            offset.AddPaths(workingPs, JoinType::Round, EndType::Polygon);
            returnPs = offset.Execute(dOffset);
        }
        // fix direction
        if (gcp_.side() == ::GCode::Outer && !gcp_.convent())
            ReversePaths(returnPs);
        else if (gcp_.side() == ::GCode::Inner && gcp_.convent())
            ReversePaths(returnPs);

        for (Path& path : returnPs)
            path.push_back(path.front());

        if (returnPs.empty()) {
            emit fileReady(nullptr);
            return;
        }
    }

    Paths framePaths;
    { // create frame
        const auto graphicObjects(file->graphicObjects());
        Clipper clipper;
        {
            ClipperOffset offset;
            for (auto go : graphicObjects) {
                if (go->positive())
                    offset.AddPaths(go->polyLineW(), JoinType::Round, EndType::Polygon);
            }
            framePaths = offset.Execute(dOffset - 0.005 * uScale);
            clipper.AddSubject(framePaths);
        }
        if (!gcp_.params[::GCode::GCodeParams::IgnoreCopper].toInt()) {
            ClipperOffset offset;
            for (auto go : graphicObjects) {
                //                if (go->closed()) {
                //                    if (go->positive())
                offset.AddPaths(go->polygonWholes(), JoinType::Round, EndType::Polygon);
                //                    else {
                //                        Paths paths(go->polygonWholes());
                //                        ReversePaths(paths);
                //                        offset.AddPaths(paths, JoinType::Miter, EndType::Polygon);
                //                    }
                //                }
            }
            framePaths = offset.Execute(dOffset - 0.005 * uScale);
            clipper.AddClip(framePaths);
        }
        for (const Paths& paths : supportPss)
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

    if (returnPs.size())
        returnPss.push_back(sortB(returnPs));

    if (returnPss.empty()) {
        emit fileReady(nullptr);
    } else {

        sortB(returnPss);
        file_ = new ::GCode::ThermalFile(std::move(gcp_), std::move(returnPss));
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    }
}

} // namespace Thermal
