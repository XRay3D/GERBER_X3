#ifndef CELL_H
#define CELL_H

#include "halfedge.h"

#include <QRectF>
#include <QVector>
namespace Vrn {
class Cell {
    // ---------------------------------------------------------------------------
    // Cell methods

public:
    Cell(Vertex* site);
   ~ Cell();

    //    Cell* init(Vertex* site)
    //    {
    //        this->site = site;
    //        halfedges.clear(); // = [];
    //        this->closeMe = false;
    //        return this;
    //    }

    double prepareHalfedges();

    // Return a list of the neighbor Ids
    //    QVector<int> getNeighborIds()
    //    {
    //        QVector<int> neighbors; // = []
    //        int iHalfedge = halfedges.length();
    //        while (iHalfedge--) {
    //            Edge* edge = this->halfedges[iHalfedge]->edge;
    //            if (edge->lSite != nullptr && edge->lSite->voronoiId != this->site->voronoiId)
    //                neighbors.push_back(edge->lSite->voronoiId);
    //            else if (edge->rSite != nullptr && edge->rSite->voronoiId != this->site->voronoiId)
    //                neighbors.push_back(edge->rSite->voronoiId);
    //        }
    //        return neighbors;
    //    }

    // Compute bounding box
    //    QRectF getBbox()
    //    {
    //        int iHalfedge = halfedges.length();
    //        double xmin = std::numeric_limits<double>::max(),
    //                ymin = std::numeric_limits<double>::max(),
    //                xmax = -std::numeric_limits<double>::max(),
    //                ymax = -std::numeric_limits<double>::max(),
    //                vx, vy;
    //        while (iHalfedge--) {
    //            Vertex* v = halfedges[iHalfedge]->getStartpoint();
    //            vx = v->x;
    //            vy = v->y;
    //            if (vx < xmin)
    //                xmin = vx;
    //            if (vy < ymin)
    //                ymin = vy;
    //            if (vx > xmax)
    //                xmax = vx;
    //            if (vy > ymax)
    //                ymax = vy;
    //            // we dont need to take into account end point,
    //            // since each end point matches a start point
    //        }
    //        return QRectF{
    //            xmin,
    //                    ymin,
    //                    xmax - xmin,
    //                    ymax - ymin
    //        };
    //    }

    // Return whether a point is inside, on, or outside the cell:
    //   -1: point is outside the perimeter of the cell
    //    0: point is on the perimeter of the cell
    //    1: point is inside the perimeter of the cell
    //
    //    int pointIntersection(double x, double y)
    //    {
    //        // Check if point in polygon. Since all polygons of a Voronoi
    //        // diagram are convex, then:
    //        // http://paulbourke.net/geometry/polygonmesh/
    //        // Solution 3 (2D):
    //        //   "If the polygon is convex then one can consider the polygon
    //        //   "as a 'path' from the first vertex. A point is on the interior
    //        //   "of this polygons if it is always on the same side of all the
    //        //   "line segments making up the path. ...
    //        //   "(y - y0) (x1 - x0) - (x - x0) (y1 - y0)
    //        //   "if it is less than 0 then P is to the right of the line segment,
    //        //   "if greater than 0 it is to the left, if equal to 0 then it lies
    //        //   "on the line segment"
    //        int iHalfedge = halfedges.length();
    //        Vertex *p0, *p1;
    //        while (iHalfedge--) {
    //            Halfedge* halfedge = halfedges[iHalfedge];
    //            p0 = halfedge->getStartpoint();
    //            p1 = halfedge->getEndpoint();
    //            double r = (y - p0->y) * (p1->x - p0->x) - (x - p0->x) * (p1->y - p0->y);
    //            if (qFuzzyIsNull(r)) //  if (!r)
    //                return 0;
    //            if (r > 0)
    //                return -1;
    //        }
    //        return 1;
    //    }

    Vertex* site;
    QVector<Halfedge*> halfedges;
    bool closeMe = false;
};
}
#endif // CELL_H
