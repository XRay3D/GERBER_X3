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

#include "myclipper.h"

#include <QString>
#include <map>
#include <utility>

namespace Thermal {

struct ThParam {
    double angle = 0.0;
    double tickness = 0.5;
    int count = 4;
};

class Model;

struct ThParam2 {
    bool aperture = false;
    bool path = false;
    bool pour = false;
    double areaMax = 0.0;
    double areaMin = 0.0;
};

using PreviewGiMapValVec = mvector<std::pair<Paths, IntPoint>>;
using PreviewGiMapVal = std::map<QString, PreviewGiMapValVec>;
using PreviewGiMap = std::map<int, PreviewGiMapVal>;

}
