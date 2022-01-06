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

#include "drillitem.h"

namespace GCode {

class File;

class DrillItem final : public AbstractDrillItem {
public:
    DrillItem(double diameter, GCode::File* file);
    ~DrillItem() override = default;

    // GraphicsItem interface
    Paths paths(int alternate = {}) const override;

    // AbstractDrillItem interface
    bool isSlot() override;

protected:
    // AbstractDrillItem interface
    void create() override;
    double m_diameter = 0.0;
};

}
