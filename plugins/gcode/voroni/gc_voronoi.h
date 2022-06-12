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
#pragma once

#include "gc_voronoiboost.h"
#include "gc_voronoicgal.h"
#include "gc_voronoijc.h"

namespace GCode {
class VoronoiCreator : /*public VoronoiCgal,*/ public VoronoiJc, public VoronoiBoost {

public:
    VoronoiCreator() { }
    ~VoronoiCreator() override = default;

protected:
    void create() override; // Creator interface
    GCodeType type() override { return Voronoi; }

private:
    void createOffset(const Tool& tool, double depth, const double width);
};
} // namespace GCode
