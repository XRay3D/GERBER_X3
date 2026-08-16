/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "hatching.h"
#include "gc_pathutils.h"
#include "gi_point.h"
#include "project.h"

#include "geo/boolean.h"
#include "geo/fill.h"

namespace CrossHatch {

void Creator::create() {
    createRaster(
        gcp.tools.front(),
        gcp.params[GCode::Params::Depth].toDouble(),
        gcp.params[UseAngle].toDouble(),
        gcp.params[HathStep].toDouble(),
        gcp.params[Pass].toInt());
}

void Creator::createRaster(const Tool& tool, const double depth, const double angle, const double hatchStep, const int prPass) {
    switch(gcp.side()) {
    case GCode::Outer: groupedPaths(GCode::Grouping::Cutoff, 1.0); break;
    case GCode::Inner: groupedPaths(GCode::Grouping::Copper); break;
    case GCode::On   : return;
    }

    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter * 0.5;
    stepOver = tool.stepover();

    Geo::Polylines profilePaths;

    for(const Geo::Polygon& group: groupedPss) {
        // Карман -- группа, сжатая на радиус (delta у Inflate -- полная ширина).
        const Geo::Polygons pocket = Geo::Inflate(Geo::Polygons{group}, -toolDiameter);
        if(pocket.empty()) continue;

        if(prPass) profilePaths.append_range(pocket.contours());

        // Штриховка -- два прохода змейки со сдвигом на 90°.
        returnPs.append_range(Geo::zigzagFill(pocket, hatchStep, angle));
        returnPs.append_range(Geo::zigzagFill(pocket, hatchStep, angle + 90));
    }

    GCode::mergePolylines(returnPs, Geo::exitWeldTolerance);
    GCode::sortByProximity(returnPs, App::home().pos() + App::zero().pos());

    if(!profilePaths.empty() && prPass) {
        GCode::sortByProximity(profilePaths, App::home().pos() + App::zero().pos());
        if(gcp.convent())
            r::for_each(profilePaths, &Geo::Polyline::reverse);
    }

    returnPss.clear();
    switch(prPass) {
    case NoProfilePass:
        if(!returnPs.empty()) returnPss.push_back(std::move(returnPs));
        break;
    case First:
        if(!profilePaths.empty()) returnPss.push_back(std::move(profilePaths));
        if(!returnPs.empty()) returnPss.push_back(std::move(returnPs));
        break;
    case Last:
        if(!returnPs.empty()) returnPss.push_back(std::move(returnPs));
        if(!profilePaths.empty()) returnPss.push_back(std::move(profilePaths));
        break;
    default: break;
    }

    if(returnPss.size()) {
        gcp.toolPathss = std::move(returnPss);
        file_ = new File{std::move(gcp)};
        file_->setFileName(tool.nameEnc());
    }
}

/////////////////////////////////////////
File::File()
    : GCode::File() { }

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        regenerate();
    }
}

void File::createGi() {
    createGiRaster();
    itemGroup()->setVisible(true);
}

} // namespace CrossHatch
