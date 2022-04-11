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

struct ThParam {
    double angle = 0.0;
    double tickness = 0.5;
    int count = 4;
};

class ThermalModel;

struct ThParam2 {
    bool aperture = false;
    bool path = false;
    bool pour = false;
    double areaMax = 0.0;
    double areaMin = 0.0;
};
