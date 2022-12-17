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
#pragma once

#include "voronoi_boost.h"
#include "voronoi_cgal.h"
#include "voronoi_jc.h"

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
