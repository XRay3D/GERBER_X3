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
#include "voronoi.h"
#include "file.h"
#include "jc_voronoi.h"

namespace ClipperLib {
inline size_t qHash(const Point& key, uint /*seed*/ = 0) { return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(Point))); }

} // namespace ClipperLib

namespace GCode {

inline size_t qHash(const GCode::VoronoiCreator::Pair& tag, uint = 0) {
    return ::qHash(tag.first.x ^ tag.second.x) ^ ::qHash(tag.first.y ^ tag.second.y);
}

void VoronoiCreator::create() {
    const auto& tool = gcp_.tools.front();
    const auto depth = gcp_.params[GCodeParams::Depth].toDouble();
    const auto width = gcp_.params[GCodeParams::Width].toDouble();

    groupedPaths(Grouping::Copper);
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
        
        file_ = new VoronoiFile(std::move(gcp_), {sortBE(returnPs)}, {});
        file_->setFileName(tool.nameEnc());
        emit fileReady(file_);
    } else {
        Paths copy {returnPs};
        copy.resize(copy.size() - 1); // remove frame
        createOffset(tool, depth, width);
        
        { // создание пермычек.
            Clipper clipper;
            clipper.AddClip(workingRawPs);
            clipper.AddOpenSubject(copy);
            clipper.Execute(ClipType::Difference, FillRule::NonZero, copy, copy);
            sortBE(copy);
            for (auto&& p : copy)
                returnPss.push_back({p});
        }
        dbgPaths(returnPs, "создание пермычек");
        { // создание заливки.
            ClipperOffset offset(uScale);
            offset.AddPaths(workingRawPs, JoinType::Round, EndType::Polygon);
            offset.AddPaths(copy, JoinType::Round, EndType::Round);
            workingRawPs = offset.Execute(dOffset + 10);
        }
        // erase empty paths
        auto begin = returnPss.begin();
        while (begin != returnPss.end()) {
            if (begin->empty())
                returnPss.erase(begin);
            else
                ++begin;
        }

        file_ = new VoronoiFile(std::move(gcp_), std::move(returnPss), std::move(workingRawPs));
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
        offset.AddPaths(returnPs, JoinType::Round /*JoinType::Miter*/, EndType::Round);
        returnPs = offset.Execute(width * uScale * 0.5);
    }
    { // fit offset to copper
        Clipper clipper;
        clipper.AddSubject(returnPs);
        for (const Paths& paths : groupedPss)
            clipper.AddClip(paths);
        // clipper.Execute(ClipType::Difference, returnPs, FillRule::Positive, FillRule::Negative);
        clipper.Execute(ClipType::Difference, FillRule::Positive, returnPs);
    }
    if (0) { // cut to copper rect
        Clipper clipper;
        clipper.AddSubject(returnPs);
        clipper.AddClip({frame});
        clipper.Execute(ClipType::Intersection, FillRule::NonZero, returnPs);
        CleanPaths(returnPs, 0.001 * uScale);
    }
    { // create pocket
        ClipperOffset offset(uScale);
        offset.AddPaths(returnPs, JoinType::Round, EndType::Polygon);
        Paths tmpPaths1;
        tmpPaths1 = offset.Execute(-dOffset);
        workingRawPs = tmpPaths1;
        Paths tmpPaths;
        do {
            tmpPaths.append(tmpPaths1);
            offset.Clear();
            offset.AddPaths(tmpPaths1, JoinType::Miter, EndType::Polygon);
            tmpPaths1 = offset.Execute(-stepOver);
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
