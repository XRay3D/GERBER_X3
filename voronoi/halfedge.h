#ifndef HALFEDGE_H
#define HALFEDGE_H

#include "edge.h"
namespace Vrn {
class Halfedge {
public:
    Halfedge(Edge* edge, Vertex* lSite, Vertex* rSite);
    Vertex* getStartpoint();

    Vertex* getEndpoint();

    Vertex* site;
    Edge* edge;
    double angle;
};
}
#endif // HALFEDGE_H
