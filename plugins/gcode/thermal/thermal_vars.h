/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
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

using PreviewGiMapValVec = mvector<std::pair<Paths, Point>>;
using PreviewGiMap = std::map<QString, PreviewGiMapValVec>;

} // namespace Thermal
