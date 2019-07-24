#ifndef VERTEX_H
#define VERTEX_H

#include <QDebug>
namespace Vrn {
class Vertex {
public:
    Vertex(double x, double y);
    Vertex(double x, double y, int id);
    ~Vertex();
    double x;
    double y;
    int voronoiId = -1;
    int id = -1;
    constexpr inline bool operator==(const Vertex& p2)
    {
        return ((!x || !p2.x) ? qFuzzyIsNull(x - p2.x) : qFuzzyCompare(x, p2.x))
            && ((!y || !p2.y) ? qFuzzyIsNull(y - p2.y) : qFuzzyCompare(y, p2.y));
    }

    constexpr inline bool operator!=(const Vertex& p2)
    {
        return !(*this == p2);
    }
};
}
#endif // VERTEX_H
