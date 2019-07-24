#include "voronoi.h"
#include <QElapsedTimer>
#include <QElapsedTimer>
#include <cmath> // sqrt(), floor()


namespace Vrn {
Voronoi::Voronoi()
{
    qDeleteAll(vertices);
    qDeleteAll(edges);
    qDeleteAll(cells);
    qDeleteAll(beachsectionJunkyard);
    qDeleteAll(circleEventJunkyard);
    //qDeleteAll(vertexJunkyard);
    //qDeleteAll(edgeJunkyard);
    //qDeleteAll(cellJunkyard);

    vertices.clear();
    edges.clear();
    cells.clear();
    beachsectionJunkyard.clear();
    circleEventJunkyard.clear();
    //vertexJunkyard.clear();
    //edgeJunkyard.clear();
    //cellJunkyard.clear();
}

void Voronoi::reset()
{
    //qDebug() << Q_FUNC_INFO;
    //    if (!beachline)
    //        beachline = new RBTree();

    // Move leftover beachsections to the beachsection junkyard.
    //    if (beachline.root) {
    //        Node* beachsection = beachline.getFirst(beachline.root);
    //        while (beachsection) {
    //            beachsectionJunkyard.push_back(beachsection); // mark for reuse
    //            beachsection = beachsection->rbNext;
    //        }
    //    }
    qDeleteAll(beachsectionJunkyard);
    beachsectionJunkyard.clear();

    qDeleteAll(circleEventJunkyard);
    circleEventJunkyard.clear();

    beachline.root = nullptr;
    //    if (!circleEvents)
    //        circleEvents = new RBTree();

    circleEvents.root = nullptr;
    firstCircleEvent = nullptr;

    qDeleteAll(vertices);
    qDeleteAll(edges);
    qDeleteAll(cells);
    vertices.clear();
    edges.clear();
    cells.clear();
}

bool Voronoi::equalWithEpsilon(double a, double b) { return abs(a - b) < epsilon; }

bool Voronoi::greaterThanWithEpsilon(double a, double b) { return a - b > epsilon; }

bool Voronoi::greaterThanOrEqualWithEpsilon(double a, double b) { return b - a < epsilon; }

bool Voronoi::lessThanWithEpsilon(double a, double b) { return b - a > epsilon; }

bool Voronoi::lessThanOrEqualWithEpsilon(double a, double b) { return a - b < epsilon; }

Cell* Voronoi::createCell(Vertex* site)
{
    //qDebug() << Q_FUNC_INFO;
    //    Cell* cell = nullptr;
    //    if (cellJunkyard.size())
    //        cell = cellJunkyard.takeLast(); //->pop();
    //    if (cell) {
    //        return cell->init(site);
    //    }
    return new Cell(site);
}

Halfedge* Voronoi::createHalfedge(Edge* edge, Vertex* lSite, Vertex* rSite)
{
    //qDebug() << Q_FUNC_INFO;
    return new Halfedge(edge, lSite, rSite);
}

Vertex* Voronoi::createVertex(double x, double y)
{
    //qDebug() << Q_FUNC_INFO;
    Vertex* v = nullptr;
    //    if (vertexJunkyard.size())
    //        v = vertexJunkyard.takeLast(); //->pop();
    //    if (!v) {
    v = new Vertex(x, y);
    //    } else {
    //        v->x = x;
    //        v->y = y;
    //    }
    vertices.push_back(v);
    return v;
}

Edge* Voronoi::createEdge(Vertex* lSite, Vertex* rSite, Vertex* va, Vertex* vb)
{
    //qDebug() << Q_FUNC_INFO;
    Edge* edge = nullptr;
    //    if (edgeJunkyard.size())
    //        edge = edgeJunkyard.takeLast(); //->pop();
    //    if (!edge) {
    edge = new Edge(lSite, rSite);
    //    } else {
    //        edge->lSite = lSite;
    //        edge->rSite = rSite;
    //        edge->va = edge->vb = nullptr;
    //    }

    edges.push_back(edge);
    if (va)
        setEdgeStartpoint(edge, lSite, rSite, va);
    if (vb)
        setEdgeEndpoint(edge, lSite, rSite, vb);

    cells[lSite->voronoiId]->halfedges.push_back(createHalfedge(edge, lSite, rSite));
    cells[rSite->voronoiId]->halfedges.push_back(createHalfedge(edge, rSite, lSite));
    return edge;
}

Edge* Voronoi::createBorderEdge(Vertex* lSite, Vertex* va, Vertex* vb)
{
    //qDebug() << Q_FUNC_INFO;
    Edge* edge = nullptr;
    //    if (edgeJunkyard.size())
    //        edge = edgeJunkyard.takeLast(); //->pop();
    //    if (!edge) {
    edge = new Edge(lSite, nullptr);
    //    } else {
    //        edge->lSite = lSite;
    //        //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter converted the original "null" assignment to a call to "delete", but you should review memory allocation of all pointer variables in the converted code:
    //        /*delete*/ edge->rSite = nullptr;
    //    }
    edge->va = va;
    edge->vb = vb;
    edges.push_back(edge);
    return edge;
}

void Voronoi::setEdgeStartpoint(Edge* edge, Vertex* lSite, Vertex* rSite, Vertex* vertex)
{
    //qDebug() << Q_FUNC_INFO;
    if (!edge->va && !edge->vb) {
        edge->va = vertex;
        edge->lSite = lSite;
        edge->rSite = rSite;
    } else if (edge->lSite == rSite) {
        edge->vb = vertex;
    } else {
        edge->va = vertex;
    }
}

void Voronoi::setEdgeEndpoint(Edge* edge, Vertex* lSite, Vertex* rSite, Vertex* vertex)
{
    //qDebug() << Q_FUNC_INFO;
    setEdgeStartpoint(edge, rSite, lSite, vertex);
}

Node* Voronoi::createBeachsection(Vertex* site)
{
    //qDebug() << Q_FUNC_INFO;
    Node* beachsection = nullptr;
    if (beachsectionJunkyard.size())
        beachsection = beachsectionJunkyard.takeLast(); //->pop();
    if (!beachsection) {
        beachsection = new Node();
    }
    beachsection->site = site;
    return beachsection;
}

double Voronoi::leftBreakPoint(CircleEvent* arc, double directrix)
{
    //qDebug() << Q_FUNC_INFO;
    // http://en.wikipedia.org/wiki/Parabola
    // http://en.wikipedia.org/wiki/Quadratic_equation
    // h1 = x1,
    // k1 = (y1+directrix)/2,
    // h2 = x2,
    // k2 = (y2+directrix)/2,
    // p1 = k1-directrix,
    // a1 = 1/(4*p1),
    // b1 = -h1/(2*p1),
    // c1 = h1*h1/(4*p1)+k1,
    // p2 = k2-directrix,
    // a2 = 1/(4*p2),
    // b2 = -h2/(2*p2),
    // c2 = h2*h2/(4*p2)+k2,
    // x = (-(b2-b1) + Math.sqrt((b2-b1)*(b2-b1) - 4*(a2-a1)*(c2-c1))) / (2*(a2-a1))
    // When x1 become the x-origin:
    // h1 = 0,
    // k1 = (y1+directrix)/2,
    // h2 = x2-x1,
    // k2 = (y2+directrix)/2,
    // p1 = k1-directrix,
    // a1 = 1/(4*p1),
    // b1 = 0,
    // c1 = k1,
    // p2 = k2-directrix,
    // a2 = 1/(4*p2),
    // b2 = -h2/(2*p2),
    // c2 = h2*h2/(4*p2)+k2,
    // x = (-b2 + Math.sqrt(b2*b2 - 4*(a2-a1)*(c2-k1))) / (2*(a2-a1)) + x1

    // change code below at your own risk: care has been taken to
    // reduce errors due to computers" finite arithmetic precision.
    // Maybe can still be improved, will see if any more of this
    // kind of errors pop up again.
    Vertex* site = arc->site;
    double rfocx = site->x,
           rfocy = site->y,
           pby2 = rfocy - directrix;
    // parabola in degenerate case where focus is on directrix
    if (!pby2) {
        return rfocx;
    }
    Node* lArc = arc->rbPrevious;
    if (!lArc) {
        return -std::numeric_limits<double>::max();
    }
    site = lArc->site;
    double lfocx = site->x, lfocy = site->y, plby2 = lfocy - directrix;
    // parabola in degenerate case where focus is on directrix
    if (!plby2) {
        return lfocx;
    }
    double hl = lfocx - rfocx,
           aby2 = 1 / pby2 - 1 / plby2,
           b = hl / plby2;
    if (aby2) {
        return (-b + sqrt(b * b - 2 * aby2 * (hl * hl / (-2 * plby2) - lfocy + plby2 / 2 + rfocy - pby2 / 2))) / aby2 + rfocx;
    }
    // both parabolas have same distance to directrix, thus break point is midway
    return (rfocx + lfocx) / 2;
}

double Voronoi::rightBreakPoint(CircleEvent* arc, double directrix)
{
    //qDebug() << Q_FUNC_INFO;
    CircleEvent* rArc = static_cast<CircleEvent*>(arc->rbNext);
    if (rArc) {
        return leftBreakPoint(rArc, directrix);
    }
    Vertex* site = arc->site;
    return site->y == directrix ? site->x : std::numeric_limits<double>::max();
}

void Voronoi::detachBeachsection(Node* beachsection)
{
    //qDebug() << Q_FUNC_INFO;
    detachCircleEvent(beachsection); // detach potentially attached circle event
    beachline.rbRemoveNode(beachsection); // remove from RB-tree
    beachsectionJunkyard.push_back(beachsection); // mark for reuse
}

void Voronoi::removeBeachsection(Node* beachsection)
{
    //qDebug() << Q_FUNC_INFO;
    CircleEvent* circle = beachsection->circleEvent;
    double x = circle->x, y = circle->ycenter;
    Vertex* vertex = createVertex(x, y);
    Node *previous = beachsection->rbPrevious, *next = beachsection->rbNext;
    QVector<Node*> disappearingTransitions{ beachsection };

    // remove collapsed beachsection from beachline
    detachBeachsection(beachsection);

    // there could be more than one empty arc at the deletion point, this
    // happens when more than two edges are linked by the same vertex,
    // so we will collect all those edges by looking up both sides of
    // the deletion point.
    // by the way, there is *always* a predecessor/successor to any collapsed
    // beach section, it"s just impossible to have a collapsing first/last
    // beach sections on the beachline, since they obviously are unconstrained
    // on their left/right side.

    // look left
    Node* lArc = previous;
    while (lArc->circleEvent && abs(x - lArc->circleEvent->x) < epsilon && abs(y - lArc->circleEvent->ycenter) < epsilon) {
        previous = lArc->rbPrevious;
        disappearingTransitions.push_front(lArc);
        detachBeachsection(lArc); // mark for reuse
        lArc = previous;
    }
    // even though it is not disappearing, I will also add the beach section
    // immediately to the left of the left-most collapsed beach section, for
    // convenience, since we need to refer to it later as this beach section
    // is the "left" site of an edge for which a start point is set.
    disappearingTransitions.push_front(lArc); //->unshift
    detachCircleEvent(lArc);

    // look right
    Node* rArc = next;
    while (rArc->circleEvent && abs(x - rArc->circleEvent->x) < epsilon && abs(y - rArc->circleEvent->ycenter) < epsilon) {
        next = rArc->rbNext;
        disappearingTransitions.push_back(rArc);
        detachBeachsection(rArc); // mark for reuse
        rArc = next;
    }
    // we also have to add the beach section immediately to the right of the
    // right-most collapsed beach section, since there is also a disappearing
    // transition representing an edge"s start point on its left.
    disappearingTransitions.push_back(rArc);
    detachCircleEvent(rArc);

    // walk through all the disappearing transitions between beach sections and
    // set the start point of their (implied) edge.
    int nArcs = disappearingTransitions.length(), iArc;
    for (iArc = 1; iArc < nArcs; iArc++) {
        rArc = disappearingTransitions[iArc];
        lArc = disappearingTransitions[iArc - 1];
        setEdgeStartpoint(rArc->edge, lArc->site, rArc->site, vertex);
    }

    // create a new edge as we have now a new transition between
    // two beach sections which were previously not adjacent.
    // since this edge appears as a new vertex is defined, the vertex
    // actually define an end point of the edge (relative to the site
    // on the left)
    lArc = disappearingTransitions[0];
    rArc = disappearingTransitions[nArcs - 1];
    rArc->edge = createEdge(lArc->site, rArc->site, nullptr /*undefined*/, vertex);

    // create circle events if any for beach sections left in the beachline
    // adjacent to collapsed sections
    attachCircleEvent(lArc);
    attachCircleEvent(rArc);
}

void Voronoi::addBeachsection(Vertex* site)
{
    //qDebug() << Q_FUNC_INFO;
    double x = site->x, directrix = site->y;

    // find the left and right beach sections which will surround the newly
    // created beach section.
    // rhill 2011-06-01: This loop is one of the most often executed,
    // hence we expand in-place the comparison-against-epsilon calls.
    Node *lArc = nullptr, *rArc = nullptr, *node = beachline.root;
    double dxl, dxr;

    while (node) {
        dxl = leftBreakPoint(node, directrix) - x;
        // x lessThanWithEpsilon xl => falls somewhere before the left edge of the beachsection
        if (dxl > epsilon) {
            // this case should never happen
            // if (!node.rbLeft) {
            //    rArc = node.rbLeft;
            //    break;
            //    }
            node = node->rbLeft;
        } else {
            dxr = x - rightBreakPoint(node, directrix);
            // x greaterThanWithEpsilon xr => falls somewhere after the right edge of the beachsection
            if (dxr > epsilon) {
                if (!node->rbRight) {
                    lArc = node;
                    break;
                }
                node = node->rbRight;
            } else {
                // x equalWithEpsilon xl => falls exactly on the left edge of the beachsection
                if (dxl > -epsilon) {
                    lArc = node->rbPrevious;
                    rArc = node;
                }
                // x equalWithEpsilon xr => falls exactly on the right edge of the beachsection
                else if (dxr > -epsilon) {
                    lArc = node;
                    rArc = node->rbNext;
                }
                // falls exactly somewhere in the middle of the beachsection
                else {
                    lArc = rArc = node;
                }
                break;
            }
        }
    }
    // at this point, keep in mind that lArc and/or rArc could be
    // undefined or null.

    // create a new beach section object for the site and add it to RB-tree
    Node* newArc = createBeachsection(site);
    beachline.rbInsertSuccessor(lArc, newArc);

    // cases:
    //

    // [null,null]
    // least likely case: new beach section is the first beach section on the
    // beachline.
    // This case means:
    //   no new transition appears
    //   no collapsing beach section
    //   new beachsection become root of the RB-tree
    if (!lArc && !rArc) {
        return;
    }

    // [lArc,rArc] where lArc == rArc
    // most likely case: new beach section split an existing beach
    // section.
    // This case means:
    //   one new transition appears
    //   the left and right beach section might be collapsing as a result
    //   two new nodes added to the RB-tree
    if (lArc == rArc) {
        // invalidate circle event of split beach section
        detachCircleEvent(lArc);

        // split the beach section into two separate beach sections
        rArc = createBeachsection(lArc->site);
        beachline.rbInsertSuccessor(newArc, rArc);

        // since we have a new transition between two beach sections,
        // a new edge is born
        newArc->edge = rArc->edge = createEdge(lArc->site, newArc->site);

        // check whether the left and right beach sections are collapsing
        // and if so create circle events, to be notified when the point of
        // collapse is reached.
        attachCircleEvent(lArc);
        attachCircleEvent(rArc);
        return;
    }

    // [lArc,null]
    // even less likely case: new beach section is the *last* beach section
    // on the beachline -- this can happen *only* if *all* the previous beach
    // sections currently on the beachline share the same y value as
    // the new beach section.
    // This case means:
    //   one new transition appears
    //   no collapsing beach section as a result
    //   new beach section become right-most node of the RB-tree
    if (lArc && !rArc) {
        newArc->edge = createEdge(lArc->site, newArc->site);
        return;
    }

    // [null,rArc]
    // impossible case: because sites are strictly processed from top to bottom,
    // and left to right, which guarantees that there will always be a beach section
    // on the left -- except of course when there are no beach section at all on
    // the beach line, which case was handled above.
    // rhill 2011-06-02: No point testing in non-debug version
    //if (!lArc && rArc) {
    //    throw "Voronoi.addBeachsection(): What is this I don"t even";
    //    }

    // [lArc,rArc] where lArc != rArc
    // somewhat less likely case: new beach section falls *exactly* in between two
    // existing beach sections
    // This case means:
    //   one transition disappears
    //   two new transitions appear
    //   the left and right beach section might be collapsing as a result
    //   only one new node added to the RB-tree
    if (lArc != rArc) {
        // invalidate circle events of left and right sites
        detachCircleEvent(lArc);
        detachCircleEvent(rArc);

        // an existing transition disappears, meaning a vertex is defined at
        // the disappearance point.
        // since the disappearance is caused by the new beachsection, the
        // vertex is at the center of the circumscribed circle of the left,
        // new and right beachsections.
        // http://mathforum.org/library/drmath/view/55002.html
        // Except that I bring the origin at A to simplify
        // calculation
        Vertex* lSite = lArc->site;
        double ax = lSite->x,
               ay = lSite->y,
               bx = site->x - ax,
               by = site->y - ay;

        Vertex* rSite = rArc->site;
        double cx = rSite->x - ax,
               cy = rSite->y - ay,
               d = 2 * (bx * cy - by * cx),
               hb = bx * bx + by * by,
               hc = cx * cx + cy * cy;
        Vertex* vertex = createVertex((cy * hb - by * hc) / d + ax, (bx * hc - cx * hb) / d + ay);

        // one transition disappear
        setEdgeStartpoint(rArc->edge, lSite, rSite, vertex);

        // two new transitions appear at the new vertex location
        newArc->edge = createEdge(lSite, site, nullptr /*undefined*/, vertex);
        rArc->edge = createEdge(site, rSite, nullptr /*undefined*/, vertex);

        // check whether the left and right beach sections are collapsing
        // and if so create circle events, to handle the point of collapse.
        attachCircleEvent(lArc);
        attachCircleEvent(rArc);
        return;
    }
}

void Voronoi::attachCircleEvent(Node* arc)
{
    //qDebug() << Q_FUNC_INFO;
    Node *lArc = arc->rbPrevious, *rArc = arc->rbNext;
    if (!lArc || !rArc) {
        return;
    } // does that ever happen?
    Vertex *lSite = lArc->site, *cSite = arc->site, *rSite = rArc->site;

    // If site of left beachsection is same as site of
    // right beachsection, there can"t be convergence
    if (lSite == rSite) {
        return;
    }

    // Find the circumscribed circle for the three sites associated
    // with the beachsection triplet.
    // rhill 2011-05-26: It is more efficient to calculate in-place
    // rather than getting the resulting circumscribed circle from an
    // object returned by calling Voronoi.circumcircle()
    // http://mathforum.org/library/drmath/view/55002.html
    // Except that I bring the origin at cSite to simplify calculations.
    // The bottom-most part of the circumcircle is our Fortune "circle
    // event", and its center is a vertex potentially part of the final
    // Voronoi diagram.
    double bx = cSite->x,
           by = cSite->y,
           ax = lSite->x - bx,
           ay = lSite->y - by,
           cx = rSite->x - bx,
           cy = rSite->y - by;

    // If points l->c->r are clockwise, then center beach section does not
    // collapse, hence it can"t end up as a vertex (we reuse "d" here, which
    // sign is reverse of the orientation, hence we reverse the test.
    // http://en.wikipedia.org/wiki/Curve_orientation#Orientation_of_a_simple_polygon
    // rhill 2011-05-21: Nasty finite precision error which caused circumcircle() to
    // return infinites: 1e-12 seems to fix the problem.
    double d = 2 * (ax * cy - ay * cx);
    if (d >= -2e-12) {
        return;
    }

    double ha = ax * ax + ay * ay,
           hc = cx * cx + cy * cy,
           x = (cy * ha - ay * hc) / d,
           y = (ax * hc - cx * ha) / d,
           ycenter = y + by;

    // Important: ybottom should always be under or at sweep, so no need
    // to waste CPU cycles by checking

    // recycle circle event object if possible
    CircleEvent* circleEvent = nullptr;
    if (circleEventJunkyard.size())
        circleEvent = circleEventJunkyard.takeLast(); //->pop();
    if (!circleEvent) {
        circleEvent = new CircleEvent();
    }
    circleEvent->arc = arc;
    circleEvent->site = cSite;
    circleEvent->x = x + bx;
    circleEvent->y = ycenter + sqrt(x * x + y * y); // y bottom
    circleEvent->ycenter = ycenter;
    arc->circleEvent = circleEvent;

    // find insertion point in RB-tree: circle events are ordered from
    // smallest to largest
    CircleEvent *predecessor = nullptr, *node = circleEvents.root;
    while (node) {
        if (circleEvent->y < node->y || (circleEvent->y == node->y && circleEvent->x <= node->x)) {
            if (node->rbLeft) {
                node = node->rbLeft;
            } else {
                predecessor = node->rbPrevious;
                break;
            }
        } else {
            if (node->rbRight) {
                node = node->rbRight;
            } else {
                predecessor = node;
                break;
            }
        }
    }
    circleEvents.rbInsertSuccessor(predecessor, circleEvent);
    if (!predecessor) {
        firstCircleEvent = circleEvent;
    }
}

void Voronoi::detachCircleEvent(CircleEvent* arc)
{
    //qDebug() << Q_FUNC_INFO;
    CircleEvent* circleEvent = arc->circleEvent;
    if (circleEvent) {
        if (!circleEvent->rbPrevious) {
            firstCircleEvent = circleEvent->rbNext;
        }
        circleEvents.rbRemoveNode(circleEvent); // remove from RB-tree
        circleEventJunkyard.push_back(circleEvent);
        //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter converted the original "null" assignment to a call to "delete", but you should review memory allocation of all pointer variables in the converted code:
        /*delete*/ arc->circleEvent = nullptr;
    }
}

bool Voronoi::connectEdge(Edge* edge, QRectF bbox)
{
    //qDebug() << Q_FUNC_INFO;
    // skip if end point already connected
    Vertex* vb = edge->vb;
    if (/*!!*/ vb) {
        return true;
    }

    // make local copy for performance purpose
    Vertex* va = edge->va;
    double xl = bbox.left(), //::xl,
        xr = bbox.right(), //::xr,
        yt = bbox.top(), //::yt,
        yb = bbox.bottom(); //::yb,
    Vertex *lSite = edge->lSite, *rSite = edge->rSite;
    double lx = lSite->x,
           ly = lSite->y,
           rx = rSite->x,
           ry = rSite->y,
           fx = (lx + rx) / 2,
           fy = (ly + ry) / 2,
           fm = std::numeric_limits<double>::max(),
           fb;

    // if we reach here, this means cells which use this edge will need
    // to be closed, whether because the edge was removed, or because it
    // was connected to the bounding box.
    cells[lSite->voronoiId]->closeMe = true;
    cells[rSite->voronoiId]->closeMe = true;

    // get the line equation of the bisector if line is not vertical
    if (ry != ly) {
        fm = (lx - rx) / (ry - ly);
        fb = fy - fm * fx;
    }

    // remember, direction of line (relative to left site):
    // upward: left.x < right.x
    // downward: left.x > right.x
    // horizontal: left.x == right.x
    // upward: left.x < right.x
    // rightward: left.y < right.y
    // leftward: left.y > right.y
    // vertical: left.y == right.y

    // depending on the direction, find the best side of the
    // bounding box to use to determine a reasonable start point

    // rhill 2013-12-02:
    // While at it, since we have the values which define the line,
    // clip the end of va if it is outside the bbox.
    // https://github.com/gorhill/Javascript-Voronoi/issues/15
    // TODO: Do all the clipping here rather than rely on Liang-Barsky
    // which does not do well sometimes due to loss of arithmetic
    // precision. The code here doesn"t degrade if one of the vertex is
    // at a huge distance.

    // special case: vertical line
    if (fm == std::numeric_limits<double>::max()) { ///*fm ==*/undefined) {
        // doesn"t intersect with viewport
        if (fx < xl || fx >= xr) {
            return false;
        }
        // downward
        if (lx > rx) {
            if (!va || va->y < yt) {
                va = createVertex(fx, yt);
            } else if (va->y >= yb) {
                return false;
            }
            vb = createVertex(fx, yb);
        }
        // upward
        else {
            if (!va || va->y > yb) {
                va = createVertex(fx, yb);
            } else if (va->y < yt) {
                return false;
            }
            vb = createVertex(fx, yt);
        }
    }
    // closer to vertical than horizontal, connect start point to the
    // top or bottom side of the bounding box
    else if (fm < -1 || fm > 1) {
        // downward
        if (lx > rx) {
            if (!va || va->y < yt) {
                va = createVertex((yt - fb) / fm, yt);
            } else if (va->y >= yb) {
                return false;
            }
            vb = createVertex((yb - fb) / fm, yb);
        }
        // upward
        else {
            if (!va || va->y > yb) {
                va = createVertex((yb - fb) / fm, yb);
            } else if (va->y < yt) {
                return false;
            }
            vb = createVertex((yt - fb) / fm, yt);
        }
    }
    // closer to horizontal than vertical, connect start point to the
    // left or right side of the bounding box
    else {
        // rightward
        if (ly < ry) {
            if (!va || va->x < xl) {
                va = createVertex(xl, fm * xl + fb);
            } else if (va->x >= xr) {
                return false;
            }
            vb = createVertex(xr, fm * xr + fb);
        }
        // leftward
        else {
            if (!va || va->x > xr) {
                va = createVertex(xr, fm * xr + fb);
            } else if (va->x < xl) {
                return false;
            }
            vb = createVertex(xl, fm * xl + fb);
        }
    }
    edge->va = va;
    edge->vb = vb;

    return true;
}

bool Voronoi::clipEdge(Edge* edge, QRectF bbox)
{
    //qDebug() << Q_FUNC_INFO;
    double ax = edge->va->x,
           ay = edge->va->y,
           bx = edge->vb->x,
           by = edge->vb->y,
           t0 = 0,
           t1 = 1,
           dx = bx - ax,
           dy = by - ay;
    // left
    double q = ax - bbox.left(); //::xl;
    if (dx == 0 && q < 0) {
        return false;
    }
    double r = -q / dx;
    if (dx < 0) {
        if (r < t0) {
            return false;
        }
        if (r < t1) {
            t1 = r;
        }
    } else if (dx > 0) {
        if (r > t1) {
            return false;
        }
        if (r > t0) {
            t0 = r;
        }
    }
    // right
    q = bbox.right() /*::xr*/ - ax;
    if (dx == 0 && q < 0) {
        return false;
    }
    r = q / dx;
    if (dx < 0) {
        if (r > t1) {
            return false;
        }
        if (r > t0) {
            t0 = r;
        }
    } else if (dx > 0) {
        if (r < t0) {
            return false;
        }
        if (r < t1) {
            t1 = r;
        }
    }
    // top
    q = ay - bbox.top(); //::yt;
    if (dy == 0 && q < 0) {
        return false;
    }
    r = -q / dy;
    if (dy < 0) {
        if (r < t0) {
            return false;
        }
        if (r < t1) {
            t1 = r;
        }
    } else if (dy > 0) {
        if (r > t1) {
            return false;
        }
        if (r > t0) {
            t0 = r;
        }
    }
    // bottom
    q = bbox.bottom() /*::yb*/ - ay;
    if (dy == 0 && q < 0) {
        return false;
    }
    r = q / dy;
    if (dy < 0) {
        if (r > t1) {
            return false;
        }
        if (r > t0) {
            t0 = r;
        }
    } else if (dy > 0) {
        if (r < t0) {
            return false;
        }
        if (r < t1) {
            t1 = r;
        }
    }

    // if we reach this point, Voronoi edge is within bbox

    // if t0 > 0, va needs to change
    // rhill 2011-06-03: we need to create a new vertex rather
    // than modifying the existing one, since the existing
    // one is likely shared with at least another edge
    if (t0 > 0) {
        edge->va = createVertex(ax + t0 * dx, ay + t0 * dy);
    }

    // if t1 < 1, vb needs to change
    // rhill 2011-06-03: we need to create a new vertex rather
    // than modifying the existing one, since the existing
    // one is likely shared with at least another edge
    if (t1 < 1) {
        edge->vb = createVertex(ax + t1 * dx, ay + t1 * dy);
    }

    // va and/or vb were clipped, thus we will need to close
    // cells which use this edge.
    if (t0 > 0 || t1 < 1) {
        cells[edge->lSite->voronoiId]->closeMe = true;
        cells[edge->rSite->voronoiId]->closeMe = true;
    }

    return true;
}

void Voronoi::clipEdges(QRectF bbox)
{
    //qDebug() << Q_FUNC_INFO;
    // connect all dangling edges to bounding box
    // or get rid of them if it can"t be done
    int iEdge = edges.length();

    // iterate backward so we can splice safely
    while (iEdge--) {
        Edge* edge = edges[iEdge];
        // edge is removed if:
        //   it is wholly outside the bounding box
        //   it is looking more like a point than a line
        if (!connectEdge(edge, bbox) || !clipEdge(edge, bbox) || (abs(edge->va->x - edge->vb->x) < epsilon && abs(edge->va->y - edge->vb->y) < epsilon)) {
            edge->va = edge->vb = nullptr;
            edges.remove(iEdge); //->splice(iEdge, 1);
            //delete edges.takeAt(iEdge); //->splice(iEdge, 1);
        }
    }
}

void Voronoi::closeCells(QRectF bbox)
{
    double xl = bbox.left(), //::xl,
        xr = bbox.right(), //::xr,
        yt = bbox.top(), //::yt,
        yb = bbox.bottom(); //::yb;

    int iCell = cells.length();

    while (iCell--) {
        Cell* cell = cells[iCell];
        // prune, order halfedges counterclockwise, then add missing ones
        // required to close cells
        if (!cell->prepareHalfedges())
            continue;
        if (!cell->closeMe)
            continue;
        // find first "unclosed" point.
        // an "unclosed" point will be the end point of a halfedge which
        // does not match the start point of the following halfedge
        QVector<Halfedge*>& halfedges = cell->halfedges;
        int nHalfedges = halfedges.length();
        // special case: only one site, in which case, the viewport is the cell
        // ...

        // all other cases
        int iLeft = 0;
        Vertex *va = nullptr, *vz = nullptr;
        while (iLeft < nHalfedges) {
            va = halfedges[iLeft]->getEndpoint();
            vz = halfedges[(iLeft + 1) % nHalfedges]->getStartpoint();
            // if end point is not equal to start point, we need to add the missing
            // halfedge(s) up to vz
            if (abs(va->x - vz->x) >= epsilon || abs(va->y - vz->y) >= epsilon) {

                // rhill 2013-12-02:
                // "Holes" in the halfedges are not necessarily always adjacent.
                // https://github.com/gorhill/Javascript-Voronoi/issues/16

                bool lastBorderSegment;
                Vertex* vb;
                Edge* edge;

                // find entry point:
                // walk downward along left side
                if (equalWithEpsilon(va->x, xl) && lessThanWithEpsilon(va->y, yb)) {
                    bool lastBorderSegment = equalWithEpsilon(vz->x, xl);
                    Vertex* vb = createVertex(xl, lastBorderSegment ? vz->y : yb);
                    Edge* edge = createBorderEdge(cell->site, va, vb);
                    iLeft++;
                    halfedges.insert(iLeft, createHalfedge(edge, cell->site, nullptr));
                    nHalfedges++;
                    if (!lastBorderSegment)
                        va = vb;
                }
                // walk rightward along bottom side
                if (equalWithEpsilon(va->y, yb) && lessThanWithEpsilon(va->x, xr)) {
                    lastBorderSegment = equalWithEpsilon(vz->y, yb);
                    vb = createVertex(lastBorderSegment ? vz->x : xr, yb);
                    edge = createBorderEdge(cell->site, va, vb);
                    iLeft++;
                    halfedges.insert(iLeft, createHalfedge(edge, cell->site, nullptr));
                    nHalfedges++;
                    if (!lastBorderSegment)
                        va = vb;
                    // fall through
                }

                // walk upward along right side
                if (equalWithEpsilon(va->x, xr) && greaterThanWithEpsilon(va->y, yt)) {
                    lastBorderSegment = equalWithEpsilon(vz->x, xr);
                    vb = createVertex(xr, lastBorderSegment ? vz->y : yt);
                    edge = createBorderEdge(cell->site, va, vb);
                    iLeft++;
                    halfedges.insert(iLeft, createHalfedge(edge, cell->site, nullptr));
                    nHalfedges++;
                    if (!lastBorderSegment)
                        va = vb;
                    // fall through
                }
                do {
                    // walk leftward along top side
                    if (equalWithEpsilon(va->y, yt) && greaterThanWithEpsilon(va->x, xl)) {
                        lastBorderSegment = equalWithEpsilon(vz->y, yt);
                        vb = createVertex(lastBorderSegment ? vz->x : xl, yt);
                        edge = createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges.insert(iLeft, createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment)
                            break;
                        else
                            va = vb;

                        // walk downward along left side
                        lastBorderSegment = equalWithEpsilon(vz->x, xl);
                        vb = createVertex(xl, lastBorderSegment ? vz->y : yb);
                        edge = createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges.insert(iLeft, createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment)
                            break;
                        else
                            va = vb;
                        // fall through

                        // walk rightward along bottom side
                        lastBorderSegment = equalWithEpsilon(vz->y, yb);
                        vb = createVertex(lastBorderSegment ? vz->x : xr, yb);
                        edge = createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges.insert(iLeft, createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment)
                            break;
                        else
                            va = vb;
                        // fall through

                        // walk upward along right side
                        lastBorderSegment = equalWithEpsilon(vz->x, xr);
                        vb = createVertex(xr, lastBorderSegment ? vz->y : yt);
                        edge = createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges.insert(iLeft, createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment)
                            break;
                        // fall through
                        qDebug("Voronoi.closeCells() > this makes no sense!");
                        break;
                        throw tr("Voronoi.closeCells() > this makes no sense!");
                    }
                } while (0);
            }
            iLeft++;
        }
        cell->closeMe = false;
    }
}

void Voronoi::quantizeSites(QVector<Vertex*>& sites)
{
    //qDebug() << Q_FUNC_INFO;
    int n = sites.length();
    while (n--) {
        Vertex* site = sites[n];
        site->x = std::floor(site->x / epsilon) * epsilon;
        site->y = std::floor(site->y / epsilon) * epsilon;
    }
}

void Voronoi::recycle(Diagram* diagram)
{
    //qDebug() << Q_FUNC_INFO;
    if (diagram) {
        if (dynamic_cast<Diagram*>(diagram) != nullptr) {
            toRecycle = diagram;
        } else {
            //Date tempVar();
            throw "Voronoi::recycleDiagram() > Need a Diagram object.";
        }
    }
}

Diagram* Voronoi::compute(const QVector<Vertex*>& sites, const QRectF& bbox)
{
    QElapsedTimer timer;
    timer.start();
    try {

        //  size      1'000'000
        //  execTime	1116 ms

        //qDebug() << Q_FUNC_INFO;

        reset();
        //        if (toRecycle) {
        //        for (Vertex* v : toRecycle->vertices) {
        //            if (!vertexJunkyard.contains(v))
        //                vertexJunkyard.append(v);
        //        }
        //        for (Edge* v : toRecycle->edges) {
        //            if (!edgeJunkyard.contains(v))
        //                edgeJunkyard.append(v);
        //        }
        //        for (Cell* v : toRecycle->cells) {
        //            if (!cellJunkyard.contains(v))
        //                cellJunkyard.append(v);
        //        }
        //        vertexJunkyard.append(toRecycle->vertices);
        //        edgeJunkyard.append(toRecycle->edges);
        //        cellJunkyard.append(toRecycle->cells);
        //            toRecycle = nullptr;
        //        }
        QVector<Vertex*> siteEvents = sites; //::slice(0);
        std::sort(siteEvents.begin(), siteEvents.end(), [](Vertex* a, Vertex* b) {
            double r = b->y - a->y;
            if (r)
                return r < 0;
            return (b->x - a->x) <= 0;
        });

        Vertex* site = nullptr;
        if (siteEvents.size())
            site = siteEvents.takeLast(); //->pop(),
        int siteid = 0;
        double xsitex = 0, xsitey = 0;
        for (;;) {
            CircleEvent* circle = firstCircleEvent;
            if (site && (!circle || site->y < circle->y || (site->y == circle->y && site->x < circle->x))) {
                if (site->x != xsitex || site->y != xsitey) {
                    cells.resize(siteid + 1);
                    cells[siteid] = createCell(site);
                    site->voronoiId = siteid++;
                    addBeachsection(site);
                    xsitey = site->y;
                    xsitex = site->x;
                }
                if (siteEvents.size())
                    site = siteEvents.takeLast(); //->pop();
                else
                    site = nullptr;
            } else if (circle) {
                removeBeachsection(circle->arc);
            } else {
                break;
            }
        }
        clipEdges(bbox);
        closeCells(bbox);

    } catch (const QString& e) {
        qDebug() << "catch" << e;
    }
    const int execTime = timer.elapsed();
    Diagram* diagram = new Diagram();
    diagram->cells = cells;
    diagram->edges = edges;
    diagram->vertices = vertices;
    diagram->execTime = execTime;

    qDeleteAll(sites);

    return diagram;
}
}
