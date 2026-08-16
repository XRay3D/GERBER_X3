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

#include "gc_creator.h"

#include <set>
#include <tuple>

namespace Voronoi {

class VoronoiJc : public virtual GCode::Creator {

protected:
    // Ключ ребра диаграммы -- по КООРДИНАТАМ концов, не по указателям: одно и
    // то же ребро приходит от диаграммы дважды, каждый раз с новыми jcv_point.
    struct Pair {
        QPointF first;
        QPointF second;
        int32_t id{};
        auto operator<=>(const Pair& b) const {
            return std::make_tuple(first.x(), first.y(), second.x(), second.y()) <=> std::make_tuple(b.first.x(), b.first.y(), b.second.x(), b.second.y());
        }
        bool operator==(const Pair& b) const { return first == b.first && second == b.second; }
    };

    using Pairs = std::set<Pair>;

    void jcVoronoi();
    Geo::Polylines toPath(const Pairs& pairs);
};

} // namespace Voronoi
