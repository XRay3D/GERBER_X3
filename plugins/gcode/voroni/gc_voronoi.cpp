// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#include "gc_voronoi.h"
#include "gc_file.h"
#include "jc_voronoi.h"

namespace ClipperLib {
inline size_t qHash(const IntPoint& key, uint /*seed*/ = 0) { return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(IntPoint))); }
} // namespace ClipperLib

namespace GCode {

inline size_t qHash(const GCode::VoronoiCreator::Pair& tag, uint = 0) {
    return ::qHash(tag.first.X ^ tag.second.X) ^ ::qHash(tag.first.Y ^ tag.second.Y);
}

void VoronoiCreator::create() {
    const auto& tool = m_gcp.tools.front();
    const auto depth = m_gcp.params[GCodeParams::Depth].toDouble();
    const auto width = m_gcp.params[GCodeParams::Width].toDouble();

    groupedPaths(CopperPaths);
    switch (m_gcp.params[GCodeParams::VorT].toInt()) {
    case 0:
        boostVoronoi();
        break;
    case 1:
        jcVoronoi();
        break;
    }

    if (width < tool.getDiameter(depth)) {
        m_returnPs.resize(m_returnPs.size() - 1); // remove frame
        m_gcp.gcType = Voronoi;
        m_file = new File({ sortBE(m_returnPs) }, m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    } else {
        Paths copy { m_returnPs };
        copy.resize(copy.size() - 1); // remove frame
        createOffset(tool, depth, width);
        m_gcp.gcType = Voronoi;
        { // создание пермычек.
            Clipper clipper;
            clipper.AddPaths(m_workingRawPs, ptClip, true);
            clipper.AddPaths(copy, ptSubject, false);
            clipper.Execute(ctDifference, copy, pftNonZero);
            sortBE(copy);
            for (auto&& p : copy)
                m_returnPss.push_back({ p });
        }
        dbgPaths(m_returnPs, "создание пермычек");
        { // создание заливки.
            ClipperOffset offset(uScale);
            offset.AddPaths(m_workingRawPs, jtRound, etClosedPolygon);
            offset.AddPaths(copy, jtRound, etOpenRound);
            offset.Execute(m_workingRawPs, m_dOffset + 10);
        }
        // erase empty paths
        auto begin = m_returnPss.begin();
        while (begin != m_returnPss.end()) {
            if (begin->empty())
                m_returnPss.erase(begin);
            else
                ++begin;
        }

        m_file = new File(m_returnPss, m_gcp, m_workingRawPs);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

void VoronoiCreator::createOffset(const Tool& tool, double depth, const double width) {
    msg = tr("Create Offset");
    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;
    const Path frame(m_returnPs.takeLast());
    { // create offset
        ClipperOffset offset;
        offset.AddPaths(m_returnPs, jtRound /*jtMiter*/, etOpenRound);
        offset.Execute(m_returnPs, width * uScale * 0.5);
    }
    { // fit offset to copper
        Clipper clipper;
        clipper.AddPaths(m_returnPs, ptSubject, true);
        for (const Paths& paths : m_groupedPss)
            clipper.AddPaths(paths, ptClip, true);
        clipper.Execute(ctDifference, m_returnPs, pftPositive, pftNegative);
    }
    if (0) { // cut to copper rect
        Clipper clipper;
        clipper.AddPaths(m_returnPs, ptSubject, true);
        clipper.AddPath(frame, ptClip, true);
        clipper.Execute(ctIntersection, m_returnPs, pftNonZero);
        CleanPolygons(m_returnPs, 0.001 * uScale);
    }
    { // create pocket
        ClipperOffset offset(uScale);
        offset.AddPaths(m_returnPs, jtRound, etClosedPolygon);
        Paths tmpPaths1;
        offset.Execute(tmpPaths1, -m_dOffset);
        m_workingRawPs = tmpPaths1;
        Paths tmpPaths;
        do {
            tmpPaths.append(tmpPaths1);
            offset.Clear();
            offset.AddPaths(tmpPaths1, jtMiter, etClosedPolygon);
            offset.Execute(tmpPaths1, -m_stepOver);
        } while (tmpPaths1.size());
        m_returnPs = tmpPaths;
    }
    if (m_returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    stacking(m_returnPs);
    // m_returnPss.push_back({frame});
}

} // namespace GCode
