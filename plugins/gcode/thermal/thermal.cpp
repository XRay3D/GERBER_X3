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
#include "thermal.h"
#include "gc_file.h"
#include "project.h"

namespace Thermal {

Creator::Creator() { }

void Creator::create() {
    createThermal(
        App::project()->file(gcp_.params[::GCode::GCodeParams::FileId].toInt()),
        gcp_.tools.front(),
        gcp_.params[::GCode::GCodeParams::Depth].toDouble());
}

void Creator::createThermal(FileInterface* file, const Tool& tool, const double depth) {
    toolDiameter = tool.getDiameter(depth);
    const double dOffset = toolDiameter * uScale * 0.5;

    {     // create tool path
        { // execute offset
            ClipperOffset offset;
            offset.AddPaths(workingPs, jtRound, etClosedPolygon);
            offset.Execute(returnPs, dOffset);
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
                    offset.AddPaths(go->polyLineW(), jtRound, etClosedPolygon);
            }
            offset.Execute(framePaths, dOffset - 0.005 * uScale);
            clipper.AddPaths(framePaths, ptSubject, true);
        }
        if (!gcp_.params[::GCode::GCodeParams::IgnoreCopper].toInt()) {
            ClipperOffset offset;
            for (auto go : graphicObjects) {
                //                if (go->closed()) {
                //                    if (go->positive())
                offset.AddPaths(go->polygonWholes(), jtRound, etClosedPolygon);
                //                    else {
                //                        Paths paths(go->polygonWholes());
                //                        ReversePaths(paths);
                //                        offset.AddPaths(paths, jtMiter, etClosedPolygon);
                //                    }
                //                }
            }
            offset.Execute(framePaths, dOffset - 0.005 * uScale);
            clipper.AddPaths(framePaths, ptClip, true);
        }
        for (const Paths& paths : supportPss)
            clipper.AddPaths(paths, ptClip, true);
        clipper.Execute(ctUnion, framePaths, pftEvenOdd);
    }

    { // Execute
        Clipper clipper;
        clipper.AddPaths(returnPs, ptSubject, false);
        clipper.AddPaths(framePaths, ptClip, true);
        clipper.Execute(ctDifference, returnPs, pftPositive);
        sortBE(returnPs);
    }

    if (returnPs.size())
        returnPss.push_back(sortB(returnPs));

    if (returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        gcp_.gcType = ::GCode::Thermal;
        file_ = new GCode::File(sortB(returnPss), std::move(gcp_));
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    }
}

} // namespace Thermal
