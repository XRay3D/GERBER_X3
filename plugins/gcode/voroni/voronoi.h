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
#pragma once

#include "types.h"
#include "voronoi_boost.h"
#include "voronoi_jc.h"

namespace Voronoi {

class Creator : public VoronoiJc, public VoronoiBoost {

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
