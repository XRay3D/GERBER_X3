/**
 * \file
 * \brief Intersection graph for Boolean operations
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2015 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#include <2geom/intersection-graph.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/utils.h>
#include <iostream>
#include <iterator>

namespace Geom {

/// Function object for comparing intersection vertices based on the intersection time.
struct PathIntersectionGraph::IntersectionVertexLess {
    bool operator()(IntersectionVertex const &a, IntersectionVertex const &b) const {
        return a.pos < b.pos;
    }
};

PathIntersectionGraph::PathIntersectionGraph(PathVector const &a, PathVector const &b, Coord precision)
    : _graph_valid(true)
{
    _pv[0] = a;
    _pv[1] = b;

    if (a.empty() || b.empty()) return;

    _prepareArguments();
    bool has_intersections = _prepareIntersectionLists(precision);
    if (!has_intersections) return;

    _assignEdgeWindingParities(precision);

    // If a path has only degenerate intersections, assign its status now.
    // This protects against later accidentally picking a point for winding
    // determination that is exactly at a removed intersection.
    _assignComponentStatusFromDegenerateIntersections();
    _removeDegenerateIntersections();
    if (_graph_valid) {
        _verify();
    }
}

/** Prepare the operands stored in PathIntersectionGraph::_pv by closing all of their constituent
 *  paths and removing degenerate segments from them.
 */
void PathIntersectionGraph::_prepareArguments()
{
    // all paths must be closed, otherwise we will miss some intersections
    for (auto & w : _pv) {
        for (auto & i : w) {
            i.close();
        }
    }
    // remove degenerate segments
    for (auto & w : _pv) {
        for (std::size_t i = w.size(); i > 0; --i) {
            if (w[i-1].empty()) {
                w.erase(w.begin() + (i-1));
                continue;
            }
            for (std::size_t j = w[i-1].size(); j > 0; --j) {
                if (w[i-1][j-1].isDegenerate()) {
                    w[i-1].erase(w[i-1].begin() + (j-1));
                }
            }
        }
    }
}

/** @brief Compute the lists of intersections between the constituent paths of both operands.
 *  @param precision – the precision setting for the sweepline algorithm.
 *  @return Whether any intersections were found.
 */
bool PathIntersectionGraph::_prepareIntersectionLists(Coord precision)
{
    std::vector<PVIntersection> pxs = _pv[0].intersect(_pv[1], precision);
    // NOTE: this early return means that the path data structures will not be created
    // if there are no intersections at all!
    if (pxs.empty()) return false;

    // prepare intersection lists for each path component
    for (unsigned w = 0; w < 2; ++w) {
        for (std::size_t i = 0; i < _pv[w].size(); ++i) {
            _components[w].push_back(new PathData(w, i));
        }
    }

    // create intersection vertices
    for (auto & px : pxs) {
        IntersectionVertex *xa, *xb;
        xa = new IntersectionVertex();
        xb = new IntersectionVertex();
        //xa->processed = xb->processed = false;
        xa->which = 0; xb->which = 1;
        xa->pos = px.first;
        xb->pos = px.second;
        xa->p = xb->p = px.point();
        xa->neighbor = xb;
        xb->neighbor = xa;
        xa->next_edge = xb->next_edge = OUTSIDE;
        xa->defective = xb->defective = false;
        _xs.push_back(xa);
        _xs.push_back(xb);
        _components[0][xa->pos.path_index].xlist.push_back(*xa);
        _components[1][xb->pos.path_index].xlist.push_back(*xb);
    }

    // sort intersections in each component according to time value
    for (auto & _component : _components) {
        for (std::size_t i = 0; i < _component.size(); ++i) {
            _component[i].xlist.sort(IntersectionVertexLess());
        }
    }

    return true;
}

/** Determine whether path portions between consecutive intersections lie inside or outside
 *  of the other path-vector.
 */
void PathIntersectionGraph::_assignEdgeWindingParities(Coord precision)
{
    for (unsigned w = 0; w < 2; ++w) {
        unsigned ow = (w+1) % 2; ///< The index of the other operand

        for (unsigned li = 0; li < _components[w].size(); ++li) { // Traverse all paths in the component
            IntersectionList &xl = _components[w][li].xlist;
            for (ILIter i = xl.begin(); i != xl.end(); ++i) { // Traverse all intersections in the path
                ILIter n = cyclic_next(i, xl);
                std::size_t pi = i->pos.path_index;

                /// Path time interval from the current crossing to the next one
                PathInterval ival = forward_interval(i->pos, n->pos, _pv[w][pi].size());
                PathTime mid = ival.inside(precision);

                Point wpoint = _pv[w][pi].pointAt(mid);
                _winding_points.push_back(wpoint);
                int wdg = _pv[ow].winding(wpoint);
                if (wdg % 2) {
                    i->next_edge = INSIDE;
                } else {
                    i->next_edge = OUTSIDE;
                }
            }
        }
    }
}

/** Detect the situation where a path is either entirely inside or entirely outside of the other
 * path-vector and set the status flag accordingly.
 */
void PathIntersectionGraph::_assignComponentStatusFromDegenerateIntersections()
{
    for (auto & _component : _components) {
        for (unsigned li = 0; li < _component.size(); ++li) {
            IntersectionList &xl = _component[li].xlist;
            bool has_in = false;
            bool has_out = false;
            for (auto & i : xl) {
                has_in |= (i.next_edge == INSIDE);
                has_out |= (i.next_edge == OUTSIDE);
            }
            if (has_in && !has_out) {
                _component[li].status = INSIDE;
            }
            if (!has_in && has_out) {
                _component[li].status = OUTSIDE;
            }
        }
    }
}

/** Remove intersections that don't change between in/out.
 *
 * In general, a degenerate intersection can happen at a point where
 * two shapes "kiss" (are tangent) but do not cross into each other.
 */
void PathIntersectionGraph::_removeDegenerateIntersections()
{
    for (auto & _component : _components) {
        for (unsigned li = 0; li < _component.size(); ++li) {
            IntersectionList &xl = _component[li].xlist;
            for (ILIter i = xl.begin(); i != xl.end();) {
                ILIter n = cyclic_next(i, xl);
                if (i->next_edge == n->next_edge) { // Both edges inside or both outside
                    bool last_node = (i == n); ///< Whether this is the last remaining crossing.
                    ILIter nn = _getNeighbor(n);
                    IntersectionList &oxl = _getPathData(nn).xlist;

                    // When exactly 3 out of 4 edges adjacent to an intersection
                    // have the same winding, we have a defective intersection,
                    // which is neither degenerate nor normal. Those can occur in paths
                    // that contain overlapping segments.
                    if (cyclic_prior(nn, oxl)->next_edge != nn->next_edge) {
                        // Not a backtrack - set the defective flag.
                        _graph_valid = false;
                        n->defective = true;
                        nn->defective = true;
                        ++i;
                        continue;
                    }
                    // Erase the degenerate or defective crossings
                    oxl.erase(nn);
                    xl.erase(n);
                    if (last_node) break;
                } else {
                    ++i;
                }
            }
        }
    }
}

/** Verify that all paths contain an even number of intersections and that
 *  the intersection graph does not contain leaves (degree one vertices).
 */
void PathIntersectionGraph::_verify()
{
#ifndef NDEBUG
    for (auto & _component : _components) {
        for (unsigned li = 0; li < _component.size(); ++li) {
            IntersectionList &xl = _component[li].xlist;
            assert(xl.size() % 2 == 0);
            for (ILIter i = xl.begin(); i != xl.end(); ++i) {
                ILIter j = cyclic_next(i, xl);
                assert(i->next_edge != j->next_edge);
            }
        }
    }
#endif
}

PathVector PathIntersectionGraph::getUnion()
{
    PathVector result = _getResult(false, false);
    _handleNonintersectingPaths(result, 0, false);
    _handleNonintersectingPaths(result, 1, false);
    return result;
}

PathVector PathIntersectionGraph::getIntersection()
{
    PathVector result = _getResult(true, true);
    _handleNonintersectingPaths(result, 0, true);
    _handleNonintersectingPaths(result, 1, true);
    return result;
}

PathVector PathIntersectionGraph::getAminusB()
{
    PathVector result = _getResult(false, true);
    _handleNonintersectingPaths(result, 0, false);
    _handleNonintersectingPaths(result, 1, true);
    return result;
}

PathVector PathIntersectionGraph::getBminusA()
{
    PathVector result = _getResult(true, false);
    _handleNonintersectingPaths(result, 1, false);
    _handleNonintersectingPaths(result, 0, true);
    return result;
}

PathVector PathIntersectionGraph::getXOR()
{
    PathVector r1, r2;
    r1 = getAminusB();
    r2 = getBminusA();
    std::copy(r2.begin(), r2.end(), std::back_inserter(r1));
    return r1;
}

std::size_t PathIntersectionGraph::size() const
{
    std::size_t result = 0;
    for (std::size_t i = 0; i < _components[0].size(); ++i) {
        result += _components[0][i].xlist.size();
    }
    return result;
}

std::vector<Point> PathIntersectionGraph::intersectionPoints(bool defective) const
{
    std::vector<Point> result;

    for (std::size_t i = 0; i < _components[0].size(); ++i) {
        for (const auto & j : _components[0][i].xlist) {
            if (j.defective == defective) {
                result.push_back(j.p);
            }
        }
    }
    return result;
}

void PathIntersectionGraph::fragments(PathVector &in, PathVector &out) const
{
    typedef boost::ptr_vector<PathData>::const_iterator PIter;
    for (unsigned w = 0; w < 2; ++w) {
        for (PIter li = _components[w].begin(); li != _components[w].end(); ++li) {
            for (CILIter k = li->xlist.begin(); k != li->xlist.end(); ++k) {
                CILIter n = cyclic_next(k, li->xlist);
                // TODO: investigate why non-contiguous paths are sometimes generated here
                Path frag(k->p);
                frag.setStitching(true);
                PathInterval ival = forward_interval(k->pos, n->pos, _pv[w][k->pos.path_index].size());
                _pv[w][k->pos.path_index].appendPortionTo(frag, ival, k->p, n->p);
                if (k->next_edge == INSIDE) {
                    in.push_back(frag);
                } else {
                    out.push_back(frag);
                }
            }
        }
    }
}

/** @brief Compute the partial result of a boolean operation by looking at components containing
 *  intersections and stitching the correct path portions between them, depending on the truth
 *  table of the operation.
 *
 * @param enter_a – whether the path portions contained inside operand A should be part of the boundary
 *                  of the boolean operation's result.
 * @param enter_b – whether the path portions contained inside operand B should be part of the boundary
 *                  of the boolean operation's result.
 *
 * These two flags completely determine how to resolve the crossings when building the result
 * and therefore encode which boolean operation we are performing. For example, the boolean intersection
 * corresponds to enter_a == true and enter_b == true, as can be seen by looking at a Venn diagram.
 */
PathVector PathIntersectionGraph::_getResult(bool enter_a, bool enter_b)
{
    PathVector result;
    if (_xs.empty()) return result;

    // Create the list of intersections to process
    _ulist.clear();
    for (auto & _component : _components) {
        for (auto & li : _component) {
            for (auto & k : li.xlist) {
                _ulist.push_back(k);
            }
        }
    }

    unsigned n_processed = 0;

    while (true) {
        // get unprocessed intersection
        if (_ulist.empty()) break;
        IntersectionVertex &iv = _ulist.front();
        unsigned w = iv.which;
        ILIter i = _components[w][iv.pos.path_index].xlist.iterator_to(iv);

        result.push_back(Path(i->p));
        result.back().setStitching(true);
        bool reverse = false; ///< Whether to traverse the current component in the backwards direction.
        while (i->_proc_hook.is_linked()) {
            ILIter prev = i;
            std::size_t pi = i->pos.path_index; ///< Index of the path in its PathVector
            // determine which direction to go
            // union: always go outside
            // intersection: always go inside
            // a minus b: go inside in b, outside in a
            // b minus a: go inside in a, outside in b
            if (w == 0) { // The path we're on is a part of A
                reverse = (i->next_edge == INSIDE) ^ enter_a;
            } else { // The path we're on is a part of B
                reverse = (i->next_edge == INSIDE) ^ enter_b;
            }

            // get next intersection
            if (reverse) {
                i = cyclic_prior(i, _components[w][pi].xlist);
            } else {
                i = cyclic_next(i, _components[w][pi].xlist);
            }

            // append portion of path to the result
            PathInterval ival = PathInterval::from_direction(
                prev->pos.asPathTime(), i->pos.asPathTime(),
                reverse, _pv[i->which][pi].size());

            _pv[i->which][pi].appendPortionTo(result.back(), ival, prev->p, i->p);

            // count both vertices as processed
            n_processed += 2;
            if (prev->_proc_hook.is_linked()) {
                _ulist.erase(_ulist.iterator_to(*prev));
            }
            if (i->_proc_hook.is_linked()) {
                _ulist.erase(_ulist.iterator_to(*i));
            }

            // switch to the other path
            i = _getNeighbor(i);
            w = i->which;
        }
        result.back().close(true);
        if (reverse){
            result.back() = result.back().reversed();
        }
        if (result.back().empty()) {
            // std::cerr << "Path is empty" << std::endl;
            throw GEOM_ERR_INTERSECGRAPH;
        }
    }
    
    if (n_processed != size() * 2) {
        // std::cerr << "Processed " << n_processed << " intersections, expected " << (size() * 2) << std::endl;
        throw GEOM_ERR_INTERSECGRAPH;
    }

    return result;
}

/** @brief Select intersection-free path components ahead of a boolean operation based on whether
 *  they should be a part of that operation's result.
 *
 * Every component that has intersections will be processed by _getResult().
 * Here we take care of paths that don't have any intersections. They are either
 * completely inside or completely outside the other path-vector.
 *
 * @param result – output parameter to store the selected components.
 * @param which – which of the two operands to search for intersection-free paths.
 * @param inside – If set to true, add paths entirely contained inside the other path-vector to
 *    the result. If set to false, add paths entirely outside of the other path-vector instead.
 */
void PathIntersectionGraph::_handleNonintersectingPaths(PathVector &result, unsigned which, bool inside)
{
    unsigned w = which;
    unsigned ow = (w+1) % 2;

    for (std::size_t i = 0; i < _pv[w].size(); ++i) {
        // the path data vector might have been left empty if there were no intersections at all
        bool has_path_data = !_components[w].empty();
        // Skip if the path has intersections
        if (has_path_data && !_components[w][i].xlist.empty()) continue;
        bool path_inside = false;

        // Use the status flag set in the constructor if available.
        if (has_path_data && _components[w][i].status == INSIDE) {
            path_inside = true;
        } else if (has_path_data && _components[w][i].status == OUTSIDE) {
            path_inside = false;
        } else {
            // The status flag is ambiguous: we evaluate the winding number of the initial point.
            int wdg = _pv[ow].winding(_pv[w][i].initialPoint());
            path_inside = wdg % 2 != 0;
        }

        if (path_inside == inside) {
            result.push_back(_pv[w][i]);
        }
    }
}

/** @brief Get an iterator to the corresponding crossing on the other path-vector.
 *
 *  @param ILIter – an iterator to a list of intersections in one of the path-vectors.
 *  @return An iterator to the corresponding intersection in the other path-vector.
 */
PathIntersectionGraph::ILIter PathIntersectionGraph::_getNeighbor(ILIter iter)
{
    unsigned ow = (iter->which + 1) % 2;
    return _components[ow][iter->neighbor->pos.path_index].xlist.iterator_to(*iter->neighbor);
}

/** Get the path data for the path containing the intersection given an iterator to the intersection */
PathIntersectionGraph::PathData &
PathIntersectionGraph::_getPathData(ILIter iter)
{
    return _components[iter->which][iter->pos.path_index];
}

/** Format the PathIntersectionGraph for output. */
std::ostream &operator<<(std::ostream &os, PathIntersectionGraph const &pig)
{
    os << "Intersection graph:\n"
       << pig._xs.size()/2 << " total intersections\n"
       << pig.size() << " considered intersections\n";
    for (std::size_t i = 0; i < pig._components[0].size(); ++i) {
        PathIntersectionGraph::IntersectionList const &xl = pig._components[0][i].xlist;
        for (const auto & j : xl) {
            os << j.pos << " - " << j.neighbor->pos << " @ " << j.p << "\n";
        }
    }
    return os;
}

} // namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

