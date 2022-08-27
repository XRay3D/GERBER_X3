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

#include "gc_creator.h"

class FileInterface;

namespace Thermal {

class Creator : public ::GCode::Creator {
public:
    Creator();
    ~Creator() override = default;

private:
    void createThermal(FileInterface* file, const Tool& tool, const double depth);

protected:
    void create() override; // Creator interface
    GCode::GCodeType type() override { return ::GCode::Thermal; }
};

} // namespace Thermal
