// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "gcthermal.h"
#include "gbrfile.h"
#include "gcfile.h"
#include "project.h"

#include "leakdetector.h"

namespace GCode {
ThermalCreator::ThermalCreator()
{
}

void ThermalCreator::create()
{
    createThermal(App::project()->file<Gerber::File>(m_gcp.params[GCodeParams::FileId].toInt()), m_gcp.tools.first(), m_gcp.params[GCodeParams::Depth].toDouble());
}

void ThermalCreator::createThermal(Gerber::File* file, const Tool& tool, const double depth)
{
    App::m_creator = this;
    m_toolDiameter = tool.getDiameter(depth);
    const double dOffset = m_toolDiameter * uScale * 0.5;

    // execute offset
    ClipperOffset offset;
    offset.AddPaths(m_workingPs, jtRound, etClosedPolygon);
    offset.Execute(m_returnPs, dOffset);

    // fix direction
    if (m_gcp.side() == Outer && !m_gcp.convent())
        ReversePaths(m_returnPs);
    else if (m_gcp.side() == Inner && m_gcp.convent())
        ReversePaths(m_returnPs);

    for (Path& path : m_returnPs)
        path.append(path.first());

    if (m_returnPs.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }

    // create frame
    Paths framePaths;
    {
        ClipperOffset offset;
        offset.AddPaths(m_returnPs, jtMiter, etClosedLine);
        offset.Execute(framePaths, +m_toolDiameter * uScale * 0.1);
        Clipper clipper;
        clipper.AddPaths(framePaths, ptSubject, true);
        {
            ClipperOffset offset;
            for (const Gerber::GraphicObject& go : *file) {
                if (go.state().type() == Gerber::Line && go.state().imgPolarity() == Gerber::Positive) {
                    offset.AddPaths(go.paths(), jtMiter, etClosedPolygon);
                }
            }
            offset.Execute(framePaths, dOffset - 0.005 * uScale);
        }
        for (const Paths& paths : m_supportPss) {
            clipper.AddPaths(paths, ptClip, true);
        }
        clipper.AddPaths(framePaths, ptClip, true);
        clipper.Execute(ctIntersection, framePaths, pftPositive);
    }
    // create thermal
    for (int index = 0; index < m_returnPs.size(); ++index) {
        const Path& path = m_returnPs.at(index);
        Paths paths;
        // cut toolPath

        Clipper clipper;
        clipper.AddPath(path, ptSubject, false);
        clipper.AddPaths(framePaths, ptClip, true);
        PolyTree polytree;
        clipper.Execute(ctDifference, polytree /*paths*/, pftPositive);
        PolyTreeToPaths(polytree, paths);
        if (paths.isEmpty())
            continue;
        // merge result toolPaths
        mergeSegments(paths);
        m_returnPs.remove(index--);
        m_returnPss.append(sortBE(paths));
    }
    if (m_returnPs.size()) {
        for (Path& path : m_returnPs)
            path.append(path.first());
        m_returnPss.append(sortB(m_returnPs));
    }
    if (m_returnPss.isEmpty()) {
        emit fileReady(nullptr);
    } else {
        m_gcp.gcType = Thermal;
        m_file = new GCode::File(sortB(m_returnPss), m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}
}
