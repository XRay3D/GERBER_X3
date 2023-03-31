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

class AbstractFile;

namespace Thermal {

class Creator : public ::GCode::Creator {
public:
    Creator();
    ~Creator() override = default;

private:
    void createThermal(AbstractFile* file, const Tool& tool, const double depth);

protected:
    void create() override; // Creator interface
    GCode::uint32_t type() override { return ::GCode::Thermal; }
};

} // namespace Thermal
