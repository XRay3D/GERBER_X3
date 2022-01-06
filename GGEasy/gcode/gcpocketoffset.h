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
#include "gccreator.h"

namespace GCode {
class PocketCreator : public Creator {
public:
    PocketCreator();

private:
    void createFixedSteps(const Tool& tool, const double depth, const int steps);
    void createStdFull(const Tool& tool, const double depth);
    void createMultiTool(mvector<Tool>& tools, double depth);

protected:
    void create() override; // Creator interface
    GCodeType type() override { return Pocket; }
};
}
