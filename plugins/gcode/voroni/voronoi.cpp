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
 *******************************************************************************/
#include "voronoi.h"
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
    const auto& tool = gcp_.tools.front();
    const auto depth = gcp_.params[GCodeParams::Depth].toDouble();
    const auto width = gcp_.params[GCodeParams::Width].toDouble();

    groupedPaths(CopperPaths);
    switch (gcp_.params[GCodeParams::VorT].toInt()) {
    case 0:
        boostVoronoi();
        break;
    case 1:
        jcVoronoi();
        break;
    }

    if (width < tool.getDiameter(depth)) {
        returnPs.resize(returnPs.size() - 1); // remove frame
        gcp_.gcType = Voronoi;
        file_ = new File({ sortBE(returnPs) }, std::move(gcp_));
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    } else {
        Paths copy { returnPs };
        copy.resize(copy.size() - 1); // remove frame
        createOffset(tool, depth, width);
        gcp_.gcType = Voronoi;
        { // создание пермычек.
            Clipper clipper;
            clipper.AddPaths(workingRawPs, ptClip, true);
            clipper.AddPaths(copy, ptSubject, false);
            clipper.Execute(ctDifference, copy, pftNonZero);
            sortBE(copy);
            for (auto&& p : copy)
                returnPss.push_back({ p });
        }
        dbgPaths(returnPs, "создание пермычек");
        { // создание заливки.
            ClipperOffset offset(uScale);
            offset.AddPaths(workingRawPs, jtRound, etClosedPolygon);
            offset.AddPaths(copy, jtRound, etOpenRound);
            offset.Execute(workingRawPs, dOffset + 10);
        }
        // erase empty paths
        auto begin = returnPss.begin();
        while (begin != returnPss.end()) {
            if (begin->empty())
                returnPss.erase(begin);
            else
                ++begin;
        }

        file_ = new File(returnPss, std::move(gcp_), workingRawPs);
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    }
}

void VoronoiCreator::createOffset(const Tool& tool, double depth, const double width) {
    msg = tr("Create Offset");
    toolDiameter = tool.getDiameter(depth) * uScale;
    dOffset = toolDiameter / 2;
    stepOver = tool.stepover() * uScale;
    const Path frame(returnPs.takeLast());
    { // create offset
        ClipperOffset offset;
        offset.AddPaths(returnPs, jtRound /*jtMiter*/, etOpenRound);
        offset.Execute(returnPs, width * uScale * 0.5);
    }
    { // fit offset to copper
        Clipper clipper;
        clipper.AddPaths(returnPs, ptSubject, true);
        for (const Paths& paths : groupedPss)
            clipper.AddPaths(paths, ptClip, true);
        clipper.Execute(ctDifference, returnPs, pftPositive, pftNegative);
    }
    if (0) { // cut to copper rect
        Clipper clipper;
        clipper.AddPaths(returnPs, ptSubject, true);
        clipper.AddPath(frame, ptClip, true);
        clipper.Execute(ctIntersection, returnPs, pftNonZero);
        CleanPolygons(returnPs, 0.001 * uScale);
    }
    { // create pocket
        ClipperOffset offset(uScale);
        offset.AddPaths(returnPs, jtRound, etClosedPolygon);
        Paths tmpPaths1;
        offset.Execute(tmpPaths1, -dOffset);
        workingRawPs = tmpPaths1;
        Paths tmpPaths;
        do {
            tmpPaths.append(tmpPaths1);
            offset.Clear();
            offset.AddPaths(tmpPaths1, jtMiter, etClosedPolygon);
            offset.Execute(tmpPaths1, -stepOver);
        } while (tmpPaths1.size());
        returnPs = tmpPaths;
    }
    if (returnPs.empty()) {
        emit fileReady(nullptr);
        return;
    }
    stacking(returnPs);
    // returnPss_.push_back({frame});
}

} // namespace GCode
