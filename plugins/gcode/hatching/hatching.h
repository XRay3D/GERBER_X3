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

#include "gc_creator.h"

namespace GCode {

class HatchingCreator : public Creator {
public:
    HatchingCreator();
    ~HatchingCreator() override = default;

    // Creator interface
protected:
    void create() override; // Creator interface
    GCodeType type() override { return Raster; }

private:
    enum {
        NoProfilePass,
        First,
        Last
    };

    void createRaster(const Tool& tool, const double depth, const double angle, const double hatchStep, const int prPass);

    Rect rect;
};

} // namespace GCode
