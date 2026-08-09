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

namespace Voronoi {

class VoronoiJc : public virtual GCode::Creator {

protected:
    struct Pair {
        Point64 first;
        Point64 second;
        int32_t id;
        bool operator==(const Pair& b) const { return first == b.first && second == b.second; }
        friend size_t qHash(const Pair& tag, uint = 0) {
            return ::qHash(tag.first.x ^ tag.second.x) ^ ::qHash(tag.first.y ^ tag.second.y);
        }
    };

    using Pairs = QSet<Pair>;
    using Pairss = mvector<Pairs>;
    struct OrdPath {
        int count = 1;
        Point64 Pt;
        OrdPath* Next = nullptr;
        OrdPath* Prev = nullptr;
        OrdPath* Last = nullptr;
        inline void push_back(OrdPath* opt) {
            ++count;
            Last->Next = opt;
            Last = opt->Prev->Last;
            opt->Prev = this;
        }
        Geo::Polyline toPath() {
            Geo::Polyline rp;
            rp.reserve(count);
            rp.push_back(Pt);
            OrdPath* next = Next;
            while(next) {
                rp.push_back(next->Pt);
                next = next->Next;
            }
            return rp;
        }
    };

    void jcVoronoi();
    Paths64 toPath(const Pairs& pairs);
};

} // namespace Voronoi
