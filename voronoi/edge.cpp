#include "edge.h"

namespace Vrn {

Edge::Edge(Vertex *lSite, Vertex *rSite)
{
    this->lSite = lSite;
    this->rSite = rSite;
    this->va = this->vb = nullptr;
}
}
