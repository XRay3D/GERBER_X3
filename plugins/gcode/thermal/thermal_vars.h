/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "geo/polygon.h"

#include <QString>
#include <map>
#include <utility>
#include <vector>

namespace Thermal {

struct ThParam {
    double angle{};
    double tickness = 0.5;
    int count = 4;
};

class Model;

using PreviewGiMapValVec = std::vector<std::pair<Geo::Polygons, QPointF>>;
using PreviewGiMap = std::map<QString, PreviewGiMapValVec>;

} // namespace Thermal
