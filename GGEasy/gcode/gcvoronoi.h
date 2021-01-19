/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
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

namespace GCode {
class VoronoiCreator : public Creator {

public:
    VoronoiCreator() { }
    ~VoronoiCreator() override = default;

protected:
    void create() override; // Creator interface
    GCodeType type() override { return Voronoi; }

private:
    struct Pair {
        IntPoint first;
        IntPoint second;
        int id;
        bool operator==(const Pair& b) const { return first == b.first && second == b.second; }
    };

    friend inline size_t qHash(const Pair& tag, uint seed);

    using Pairs = QSet<Pair>;
    using Pairss = mvector<Pairs>;
    struct OrdPath {
        int count = 1;
        IntPoint Pt;
        OrdPath* Next = nullptr;
        OrdPath* Prev = nullptr;
        OrdPath* Last = nullptr;
        inline void push_back(OrdPath* opt)
        {
            ++count;
            Last->Next = opt;
            Last = opt->Prev->Last;
            opt->Prev = this;
        }
        Path toPath()
        {
            Path rp;
            rp.reserve(count);
            rp.push_back(Pt);
            OrdPath* next = Next;
            while (next) {
                rp.push_back(next->Pt);
                next = next->Next;
            }
            return rp;
        }
    };

    void createVoronoi();
    void createOffset(const Tool& tool, double depth, const double width);
    void mergePaths(Paths& paths, const double dist = 0.0);
    void clean(Path& path);
    void cgalVoronoi();
    void jcVoronoi();
    void boostVoronoi();
    Paths toPath(const Pairs& pairs);
};
}
