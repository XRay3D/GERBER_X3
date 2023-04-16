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

namespace Voronoi {

class VoronoiJc : public virtual GCode::Creator {

protected:
    struct Pair {
        Point first;
        Point second;
        int32_t id;
        bool operator==(const Pair& b) const { return first == b.first && second == b.second; }
    };

    friend inline size_t qHash(const Pair& tag, uint seed);

    using Pairs = QSet<Pair>;
    using Pairss = mvector<Pairs>;
    struct OrdPath {
        int count = 1;
        Point Pt;
        OrdPath* Next = nullptr;
        OrdPath* Prev = nullptr;
        OrdPath* Last = nullptr;
        inline void push_back(OrdPath* opt) {
            ++count;
            Last->Next = opt;
            Last = opt->Prev->Last;
            opt->Prev = this;
        }
        Path toPath() {
            Path rp;
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
    Paths toPath(const Pairs& pairs);
};

} // namespace Voronoi
