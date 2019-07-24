#ifndef EDGE_H
#define EDGE_H

#include "vertex.h"
namespace Vrn {
class Edge {
public:
    Edge(Vertex* lSite, Vertex* rSite);
    Vertex* lSite;
    Vertex* rSite;
    Vertex* va;
    Vertex* vb;
};
}
#endif // EDGE_H
