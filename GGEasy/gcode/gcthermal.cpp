// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcthermal.h"
#ifdef GERBER
#include "gbrfile.h"
#endif
#include "gcfile.h"
#include "project.h"

#include "leakdetector.h"

namespace GCode {

ThermalCreator::ThermalCreator() { }

void ThermalCreator::create()
{
#ifdef GERBER
    createThermal(
        App::project()->file<Gerber::File>(m_gcp.params[GCodeParams::FileId].toInt()),
        m_gcp.tools.first(),
        m_gcp.params[GCodeParams::Depth].toDouble());
#else
    emit fileReady(nullptr);
#endif
}

void ThermalCreator::createThermal(Gerber::File* file, const Tool& tool, const double depth)
{
#ifdef GERBER

//    m_toolDiameter = tool.getDiameter(depth);
//    const double dOffset = m_toolDiameter * uScale * 0.5;

//    { // create tool path
//        // execute offset
//        {
//            ClipperOffset offset;
//            offset.AddPaths(m_workingPs, jtRound, etClosedPolygon);
//            offset.Execute(m_returnPs, dOffset);
//        }

//        // fix direction
//        if (m_gcp.side() == Outer && !m_gcp.convent())
//            ReversePaths(m_returnPs);
//        else if (m_gcp.side() == Inner && m_gcp.convent())
//            ReversePaths(m_returnPs);

//        for (Path& path : m_returnPs)
//            path.push_back(path.first());

//        if (m_returnPs.isEmpty()) {
//            emit fileReady(nullptr);
//            return;
//        }
//        //dbgPaths(m_returnPs, "tool path");
//    }

//    // create frame
//    Paths framePaths;
//    {
//        Clipper clipper;
//        {
//            ClipperOffset offset;
//            for (const Gerber::GraphicObject& go : *file) {
//                if (go.state().type() == Gerber::Line && go.state().imgPolarity() == Gerber::Positive) {
//                    offset.AddPaths(go.paths(), jtMiter, etClosedPolygon);
//                }
//            }
//            offset.Execute(framePaths, dOffset - 0.005 * uScale);
//            //dbgPaths(framePaths, "Gerber::Line", true);
//            clipper.AddPaths(framePaths, ptSubject, true);
//        }
//        if (!m_gcp.params[GCodeParams::IgnoreCopper].toInt()) {
//            ClipperOffset offset;
//            for (const Gerber::GraphicObject& go : *file) {
//                if (go.state().type() == Gerber::Region) {
//                    if (go.state().imgPolarity() == Gerber::Positive)
//                        offset.AddPaths(go.paths(), jtMiter, etClosedPolygon);
//                    else {
//                        Paths paths(go.paths());
//                        ReversePaths(paths);
//                        offset.AddPaths(paths, jtMiter, etClosedPolygon);
//                    }
//                }
//            }
//            offset.Execute(framePaths, dOffset - 0.005 * uScale);
//            //dbgPaths(framePaths, "Gerber::Region", true);
//            clipper.AddPaths(framePaths, ptClip, true);
//        }
//        for (const Paths& paths : m_supportPss) {
//            clipper.AddPaths(paths, ptClip, true);
//        }
//        clipper.Execute(ctUnion, framePaths, pftEvenOdd);
//        //dbgPaths(framePaths, "framePaths", true);
//    }

//    {
//        Clipper clipper;
//        clipper.AddPaths(m_returnPs, ptSubject, false);
//        clipper.AddPaths(framePaths, ptClip, true);
//        clipper.Execute(ctDifference, m_returnPs, pftPositive);
//        sortBE(m_returnPs);
//    }

//    if (m_returnPs.size()) {
//        //        for (Path& path : m_returnPs)
//        //            path.push_back(path.first());
//        m_returnPss.push_back(sortB(m_returnPs));
//    }
//    if (m_returnPss.isEmpty()) {
//        emit fileReady(nullptr);
//    } else {
//        m_gcp.gcType = Thermal;
//        m_file = new GCode::File(sortB(m_returnPss), m_gcp);
//        m_file->setFileName(tool.nameEnc());
//        emit fileReady(m_file);
//    }
#endif
}
}
