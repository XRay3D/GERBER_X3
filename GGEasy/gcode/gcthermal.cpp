// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcthermal.h"
#include "gcfile.h"
#include "project.h"

#include "leakdetector.h"

namespace GCode {

ThermalCreator::ThermalCreator() { }

void ThermalCreator::create()
{
    createThermal(
        App::project()->file(m_gcp.params[GCodeParams::FileId].toInt()),
        m_gcp.tools.front(),
        m_gcp.params[GCodeParams::Depth].toDouble());
}

void ThermalCreator::createThermal(FileInterface* file, const Tool& tool, const double depth)
{
    m_toolDiameter = tool.getDiameter(depth);
    const double dOffset = m_toolDiameter * uScale * 0.5;

    { // create tool path
        { // execute offset
            ClipperOffset offset;
            offset.AddPaths(m_workingPs, jtRound, etClosedPolygon);
            offset.Execute(m_returnPs, dOffset);
        }
        // fix direction
        if (m_gcp.side() == Outer && !m_gcp.convent())
            ReversePaths(m_returnPs);
        else if (m_gcp.side() == Inner && m_gcp.convent())
            ReversePaths(m_returnPs);

        for (Path& path : m_returnPs)
            path.push_back(path.front());

        if (m_returnPs.empty()) {
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
        if (!m_gcp.params[GCodeParams::IgnoreCopper].toInt()) {
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
        for (const Paths& paths : m_supportPss)
            clipper.AddPaths(paths, ptClip, true);
        clipper.Execute(ctUnion, framePaths, pftEvenOdd);
    }

    { // Execute
        Clipper clipper;
        clipper.AddPaths(m_returnPs, ptSubject, false);
        clipper.AddPaths(framePaths, ptClip, true);
        clipper.Execute(ctDifference, m_returnPs, pftPositive);
        sortBE(m_returnPs);
    }

    if (m_returnPs.size())
        m_returnPss.push_back(sortB(m_returnPs));

    if (m_returnPss.empty()) {
        emit fileReady(nullptr);
    } else {
        m_gcp.gcType = Thermal;
        m_file = new GCode::File(sortB(m_returnPss), m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}
}
