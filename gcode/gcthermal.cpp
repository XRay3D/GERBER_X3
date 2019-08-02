#include "gcthermal.h"

#include <gbrfile.h>
#include <project.h>

namespace GCode {
ThermalCreator::ThermalCreator()
{
}

void ThermalCreator::create(const GCodeParams& gcp)
{
    m_gcp = gcp;
    createThermal(Project::file<Gerber::File>(m_gcp.dParam[FileId]), m_gcp.tool.first(), m_gcp.dParam[Depth]);
}

void ThermalCreator::createThermal(Gerber::File* file, const Tool& tool, const double depth)
{
    try {
        self = this;
        m_toolDiameter = tool.getDiameter(depth);
        const double dOffset = m_toolDiameter * uScale * 0.5;

        // execute offset
        ClipperOffset offset;
        offset.AddPaths(m_workingPaths, jtRound, etClosedPolygon);
        offset.Execute(m_returnPaths, dOffset);

        // fix direction
        if (m_gcp.side == Outer && !m_gcp.convent)
            ReversePaths(m_returnPaths);
        else if (m_gcp.side == Inner && m_gcp.convent)
            ReversePaths(m_returnPaths);

        for (Path& path : m_returnPaths)
            path.append(path.first());

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            return;
        }

        // create frame
        Paths framePaths;
        {
            ClipperOffset offset;

            offset.AddPaths(m_returnPaths, jtMiter, etClosedLine);

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
            for (const Paths& paths : m_supportPathss) {
                clipper.AddPaths(paths, ptClip, true);
            }
            clipper.AddPaths(framePaths, ptClip, true);
            clipper.Execute(ctIntersection, framePaths, pftPositive);
        }

        // create thermal

        //self = nullptr;

        for (int index = 0, size = m_returnPaths.size(); index < size; ++index) {
            Path& path = m_returnPaths[index];
            //            {
            //                ClipperOffset offset;
            //                offset.AddPaths(paths, jtRound, etClosedPolygon);
            //                offset.Execute(paths, dOffset);
            //                for (Path& path : paths) {
            //                    path.append(path.last());
            //                }
            //            }

            Paths tmpPaths;
            // cut toolPath
            {
                Clipper clipper;
                clipper.AddPath(path, ptSubject, false);
                clipper.AddPaths(framePaths, ptClip, true);
                //                clipper.AddPaths(m_supportPathss[index], ptClip, true);
                PolyTree polytree;
                clipper.Execute(ctDifference, polytree, pftPositive);
                PolyTreeToPaths(polytree, tmpPaths);
            }
            // merge result toolPaths
            mergeSegments(tmpPaths);

            --size;
            m_returnPaths.remove(index--);
            m_returnPaths.append(tmpPaths);
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
        } else {
            m_file = new GCode::File(sortByStratDistance(m_returnPaths), tool, depth, Thermal);
            m_file->setFileName(tool.name());
            emit fileReady(m_file);
        }
    } catch (...) {
        //qDebug() << "catch";
    }
}
}
