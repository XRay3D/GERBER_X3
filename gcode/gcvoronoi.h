#ifndef VORONOI_H
#define VORONOI_H

#include "gccreator.h"

namespace GCode {
class VoronoiCreator : public Creator {

public:
    VoronoiCreator() {}
    ~VoronoiCreator() override = default;

protected:
    void create() override; // Creator interface

private:
    struct Pair {
        IntPoint first;
        IntPoint second;
        int id;
        bool operator==(const Pair& b) const { return first == b.first && second == b.second; }
    };

    friend inline uint qHash(const Pair& tag, uint seed);

    using Pairs = QSet<Pair>;
    using Pairss = QVector<Pairs>;
    struct OrdPath {
        int count = 1;
        IntPoint Pt;
        OrdPath* Next = nullptr;
        OrdPath* Prev = nullptr;
        OrdPath* Last = nullptr;
        inline void append(OrdPath* opt)
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
            rp.append(Pt);
            OrdPath* next = Next;
            while (next) {
                rp.append(std::move(next->Pt));
                next = next->Next;
            }
            return rp;
        }
    };

    void createOffset(const Tool& tool, double depth, const double width);
    void mergePaths(Paths& paths, const double dist);
    void clean(Path& path);
    void cgalVoronoi();
    void jcVoronoi();
    Paths toPath(Pairs&& pairs);
};
}

#endif // VORONOI_H
