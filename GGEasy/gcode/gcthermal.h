/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "gccreator.h"

class FileInterface;

namespace GCode {
class ThermalCreator : public Creator {
public:
    ThermalCreator();
    ~ThermalCreator() override = default;

private:
    void createThermal(FileInterface* file, const Tool& tool, const double depth);

protected:
    void create() override; // Creator interface
    GCodeType type() override { return Thermal; }
};
}
