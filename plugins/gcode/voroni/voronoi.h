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
#pragma once

#include "types.h"
#include "voronoi_boost.h"
#include "voronoi_cgal.h"
#include "voronoi_jc.h"

namespace Voronoi {

class Creator :
#if __has_include(<CGAL/Algebraic_structure_traits_.h>)
    public VoronoiCgal,
#endif
    public VoronoiJc
    // #if __has_include(<boost/polygon/voronoi.hpp>)
    ,
    public VoronoiBoost
// #endif
{

public:
    Creator() { }
    ~Creator() override = default;

protected:
    void create() override; // Creator interface
    uint32_t type() override { return VORONOI; }

private:
    void createOffset(const Tool& tool, double depth, const double width);
};

} // namespace Voronoi
