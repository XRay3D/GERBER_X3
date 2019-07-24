#include "cell.h"

#include <QObject>
#include <limits>
namespace Vrn {
Cell::Cell(Vertex* site)
{
    this->site = site;
    this->closeMe = false;
}

Cell::~Cell()
{
    qDeleteAll(halfedges);
}

double Cell::prepareHalfedges()
{
    //Halfedge* halfedges = halfedges.begin();
    int iHalfedge = halfedges.length();
    // get rid of unused halfedges
    // rhill 2011-05-27: Keep it simple, no point here in trying
    // to be fancy: dangling edges are a typically a minority.
    while (iHalfedge--) {
        Edge* edge = halfedges[iHalfedge]->edge;
        if (!edge->vb || !edge->va) {
            delete halfedges.takeAt(iHalfedge);
        }
    }

    // rhill 2011-05-26: I tried to use a binary search at insertion
    // time to keep the array sorted on-the-fly (in Cell.addHalfedge()).
    // There was no real benefits in doing so, performance on
    // Firefox 3.6 was improved marginally, while performance on
    // Opera 11 was penalized marginally.
    //halfedges->sort(function(a, b) { return b->angle - a->angle; });
    std::sort(halfedges.begin(), halfedges.end(), [](Halfedge* a, Halfedge* b) { return (b->angle - a->angle) <= 0; });
    return halfedges.length();
}
}
