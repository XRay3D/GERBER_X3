#ifndef VORONOI_H
#define VORONOI_H

#include "diagram.h"
#include "rbtree.h"
#include <QObject>
#include <QRectF>

// rhill 2011-06-07: For some reasons, performance suffers significantly
// when instanciating a literal object instead of an empty ctor

namespace Vrn {

class Voronoi : public QObject {

public:
    // ---------------------------------------------------------------------------
    Voronoi();
    QVector<Vertex*> vertices;
    QVector<Edge*> edges;
    QVector<Cell*> cells;
    //this->toRecycle = nullptr;
    QVector<Node*> beachsectionJunkyard;
    QVector<CircleEvent*> circleEventJunkyard;
    //    QVector<Vertex*> vertexJunkyard;
    //    QVector<Edge*> edgeJunkyard;
    //    QVector<Cell*> cellJunkyard;
    RBTree beachline;
    RBTree circleEvents;
    CircleEvent* firstCircleEvent = nullptr;
    Diagram* toRecycle = nullptr;

    void reset();

    const double epsilon = 1e-9;
    const double invEpsilon = 1.0 / epsilon;
    bool equalWithEpsilon(double a, double b);
    bool greaterThanWithEpsilon(double a, double b);
    bool greaterThanOrEqualWithEpsilon(double a, double b);
    bool lessThanWithEpsilon(double a, double b);
    bool lessThanOrEqualWithEpsilon(double a, double b);

    Cell* createCell(Vertex* site);

    // ---------------------------------------------------------------------------
    // Edge methods
    //

    Halfedge* createHalfedge(Edge* edge, Vertex* lSite, Vertex* rSite = nullptr);

    // this create and add a vertex to the internal collection

    Vertex* createVertex(double x, double y);

    // this create and add an edge to internal collection, and also create
    // two halfedges which are added to each site's counterclockwise array
    // of halfedges.

    Edge* createEdge(Vertex* lSite, Vertex* rSite, Vertex* va = nullptr, Vertex* vb = nullptr);

    Edge* createBorderEdge(Vertex* lSite, Vertex* va, Vertex* vb);

    void setEdgeStartpoint(Edge* edge, Vertex* lSite, Vertex* rSite, Vertex* vertex);

    void setEdgeEndpoint(Edge* edge, Vertex* lSite, Vertex* rSite, Vertex* vertex);

    // ---------------------------------------------------------------------------
    // Beachline methods

    // rhill 2011-06-02: A lot of Beachsection instanciations
    // occur during the computation of the Voronoi diagram,
    // somewhere between the number of sites and twice the
    // number of sites, while the number of Beachsections on the
    // beachline at any given time is comparatively low. For this
    // reason, we reuse already created Beachsections, in order
    // to avoid new memory allocation. This resulted in a measurable
    // performance gain.

    Node* createBeachsection(Vertex* site);

    // calculate the left break point of a particular beach section,
    // given a particular sweep line
    double leftBreakPoint(CircleEvent* arc, double directrix);

    // calculate the right break point of a particular beach section,
    // given a particular directrix
    double rightBreakPoint(CircleEvent* arc, double directrix);

    void detachBeachsection(Node* beachsection);

    void removeBeachsection(Node* beachsection);

    void addBeachsection(Vertex* site);

    // ---------------------------------------------------------------------------
    // Circle event methods

    // rhill 2011-06-07: For some reasons, performance suffers significantly
    // when instanciating a literal object instead of an empty ctor

    void attachCircleEvent(Node* arc);

    void detachCircleEvent(Node* arc);

    // ---------------------------------------------------------------------------
    // Diagram completion methods

    // connect dangling edges (not if a cursory test tells us
    // it is not going to be visible.
    // return value:
    //   false: the dangling endpoint couldn't be connected
    //   true: the dangling endpoint could be connected
    bool connectEdge(Edge* edge, QRectF bbox);

    // line-clipping code taken from:
    //   Liang-Barsky function by Daniel White
    //   http://www.skytopia.com/project/articles/compsci/clipping.html
    // Thanks!
    // A bit modified to minimize code paths
    bool clipEdge(Edge* edge, QRectF bbox);

    // Connect/cut edges at bounding box
    void clipEdges(QRectF bbox);

    // Close the cells.
    // The cells are bound by the supplied bounding box.
    // Each cell refers to its associated site, and a list
    // of halfedges ordered counterclockwise.
    void closeCells(QRectF bbox);

    // ---------------------------------------------------------------------------
    // Helper: Quantize sites

    // rhill 2013-10-12:
    // This is to solve https://github.com/gorhill/Javascript-Voronoi/issues/15
    // Since not all users will end up using the kind of coord values which would
    // cause the issue to arise, I chose to let the user decide whether or not
    // he should sanitize his coord values through this helper. This way, for
    // those users who uses coord values which are known to be fine, no overhead is
    // added.

    void quantizeSites(QVector<Vertex*>& sites);

    // ---------------------------------------------------------------------------
    // Helper: Recycle diagram: all vertex, edge and cell objects are
    // "surrendered" to the Voronoi object for reuse.
    // TODO: rhill-voronoi-core v2: more performance to be gained
    // when I change the semantic of what is returned.

    void recycle(Diagram* diagram);

    Diagram* compute(const QVector<Vertex*>& sites, const QRectF& bbox);

    // ---------------------------------------------------------------------------
    // Top-level Fortune loop
    // rhill 2011-05-19:
    //   Voronoi sites are kept client-side now, to allow
    //   user to freely modify content. At compute time,
    //   *references* to sites are copied locally.
    // to measure execution time
    // init internal state
    // any diagram data available for recycling?
    // I do that here so that this is included in execution time
    // Initialize site event queue
    // process queue
    // main loop
    // we need to figure whether we handle a site or circle event
    // for this we find out if there is a site event and it is
    // 'earlier' than the circle event
    // add beach section
    // only if site is not a duplicate
    // first create cell for new site
    // then create a beachsection for that site
    // remember last site coords to detect duplicate
    // remove beach section
    // all done, quit
    // wrapping-up:
    //   connect dangling edges to bounding box
    //   cut edges as per bounding box
    //   discard edges completely outside bounding box
    //   discard edges which are point-like
    //   add missing edges in order to close opened cells
    // to measure execution time
    // prepare return values
    // clean up
    /// <summary>
    ///*************************************************************************** </summary>
};
}
#endif // VORONOI_H
