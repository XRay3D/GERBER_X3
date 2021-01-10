/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

struct ThParam {
    double angle = 0.0;
    double tickness = 0.5;
    int count = 4;
};

class ThermalModel;
class ThParam2 {
public:
    ThParam par;
    ThermalModel* model = nullptr;
    bool perture = false;
    bool path = false;
    bool pour = false;
    double areaMax = 0.0;
    double areaMin = 0.0;
};
