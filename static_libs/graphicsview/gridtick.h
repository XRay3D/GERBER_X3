/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include <cmath>
#include <cstdint>

// Shared by Ruler::paintEvent and GraphicsView::drawForeground so both draw
// grid lines / tick marks with identical, drift-free math.
//
// Every tick position is `origin + index * step`, computed once by
// multiplication rather than accumulated with `current += step` in a loop,
// so there is no floating point drift when panned far from the origin or
// when zoomed in on a tiny step. `index` is also handed to the callback so
// callers can classify ticks (e.g. every 5th / 10th) without a separate
// dedup pass.
template <typename Fn>
void forEachGridTick(double origin, double rangeStart, double rangeEnd, double step, Fn&& fn) {
    if(!(step > 0.0)) [[unlikely]]
        return;

    const auto firstIndex = static_cast<int64_t>(std::ceil((rangeStart - origin) / step));
    const auto lastIndex = static_cast<int64_t>(std::floor((rangeEnd - origin) / step));

    for(int64_t index = firstIndex; index <= lastIndex; ++index)
        fn(index, origin + index * step);
}

// Classifies a fine-grid tick index into the coarser grid it also belongs
// to: 2 = every 10th tick, 1 = every 5th tick, 0 = every other tick.
// Relies on the 1x/5x/10x grid steps used throughout graphicsview being
// exact integer multiples of one another.
inline int gridTickLevel(int64_t index) {
    if(index < 0) index = -index;
    if(index % 10 == 0) return 2;
    if(index % 5 == 0) return 1;
    return 0;
}
