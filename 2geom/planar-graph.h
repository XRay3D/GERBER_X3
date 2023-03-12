/** @file PlanarGraph – a graph geometrically embedded in the plane.
 */
/*
 * Authors:
 *   Rafał Siejakowski <rs@rs-math.net>
 *
 * Copyright 2022 the Authors
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

// WARNING: This is a private header. Do not include it directly.

#ifndef LIB2GEOM_SEEN_PLANAR_GRAPH_H
#define LIB2GEOM_SEEN_PLANAR_GRAPH_H

#include <algorithm>
#include <iterator>
#include <list>

#include <2geom/angle.h>
#include <2geom/coord.h>
#include <2geom/line.h>
#include <2geom/point.h>
#include <2geom/path.h>
#include <2geom/path-intersection.h>
#include <2geom/utils.h>

namespace Geom {

/**
 * \class PlanarGraph
 * \brief Planar graph - a graph geometrically embedded in the plane.
 *
 * A planar graph is composed of vertices with assigned locations (as points in the plane)
 * and of edges (arcs), which are imagined as non-intersecting paths in the plane connecting
 * the vertices. The edges can hold user-supplied labels (e.g., weights) which support event
 * callbacks for when the graph is reconfigured, allowing the labels to be updated accordingly.
 *
 * \tparam EdgeLabel A user-supplied type; an object of this type will be attached to each
 *                   edge of the planar graph (e.g., a "weight" of the edge). The type must
 *                   satisfy requirements described further below.
 *
 * In order to construct a planar graph, you should specify the clumping precision (passed as
 * a constructor argument) and then use the method insertEdge() to add edges to the graph, as
 * many times as necessary. The graph will automatically figure out the locations of the
 * vertices based on the endpoints of the inserted edges. Vertices will be combined into one
 * when they are positioned within the distance specified as the clumping threshold, and the
 * inserted edges will be attached to them accordingly. It is also possible to insert paths
 * (typically, closed) not attached to any vertices, using the method insertDetached().
 *
 * After the edges are inserted, the graph is in a potentially degenerate state, where several
 * edges may exactly coincide in part or in full. If this is not desired, you can regularize
 * the graph by calling regularize(). During the regularization process, any overlapping edges
 * are combined into one. Partially overlapping edges are first split into overlapping and
 * non-overlapping portions, after which the overlapping portions are combined. If the edges
 * or their parts overlap but run in opposite directions, one of them will be reversed before
 * being merged with the other one. The overlaps are detected using the precision setting passed
 * as the clumping precision in the constructor argument.
 *
 * Note however that the regularization procedure does NOT detect transverse intersections
 * between the edge paths: if such intersections are not desired, the user must pass non-\
 * intersecting paths to the insertEdge() method (the paths may still have common endpoints,
 * which is fine: that's how common vertices are created).
 *
 * The insertion of new edges invalidates the regularized status, which you can check at any
 * time by calling isRegularized().
 *
 * The vertices stored by the graph are sorted by increasing X-coordinate, and if they have
 * equal X-coordinates, by increasing Y-coordinate. Even before regularization, incidences of
 * edges to each vertex are sorted by increasing azimuth of the outgoing tangent (departure
 * heading, but in radians, in the interval \f$(-\pi, \pi]\f$). After regularization, the edges
 * around each vertex are guaranteed to be sorted counterclockwise (when the Y-axis points up)
 * by where they end up going eventually, even if they're tangent at the vertex and therefore
 * have equal or nearly equal departure azimuths.
 *
 * \note
 * Requirements on the \c EdgeLabel template parameter type.
 * In order for the template to instantiate correctly, the following must be satisfied:
 * \li The \c EdgeLabel type provides a method \c onReverse() which gets called whenever
 *     the orientation of the labeled edge is reversed. This is useful when implementing
 *     a directed graph, since the label can keep track of the logical direction.
 * \li The \c EdgeLabel type provides a method \c onMergeWith(EdgeLabel const&) which gets
 *     called when the labeled edge is combined with a geometrically identical (coinciding)
 *     edge (both combined edges having the same orientations). The label of the edge merged
 *     with the current one is provided as an argument to the method. This is useful when
 *     implementing a graph with weights: for example, when two edges are merged, you may
 *     want to combine their weights in some way.
 * \li There is a method \c onDetach() called when the edge is removed from the graph. The
 *     edge objects are never destroyed but may be disconnected from the graph when they're no
 *     longer needed; this allows the user to put the labels of such edges in a "dead" state.
 * \li The \c EdgeLabel objects must be copy-constructible and copy-assignable. This is
 *     because when an edge is subdivided into two, the new edges replacing it get decorated
 *     with copies of the original edge's label.
 */
template<typename EdgeLabel>
#if __cplusplus >= 202002L
requires requires(EdgeLabel el, EdgeLabel const &other) {
    el.onReverse();
    el.onMergeWith(other);
    el.onDetach();
    el = other;
}
#endif
class PlanarGraph
{
public:

    /** Represents the joint between an edge and a vertex. */
    struct Incidence
    {
        using Sign = bool;
        inline static Sign const START = false;
        inline static Sign const END   = true;

        double azimuth; ///< Angle of the edge's departure.
        unsigned index; ///< Index of the edge in the parent graph.
        Sign sign;      ///< Whether this is the start or end of the edge.
        bool invalid = false; ///< Whether this incidence has been marked for deletion.

        Incidence(unsigned edge_index, double departure_azimuth, Sign which_end)
            : azimuth{departure_azimuth}
            , index{edge_index}
            , sign{which_end}
        {
        }
        ~Incidence() = default;

        /// Compare incidences based on their azimuths in radians.
        inline bool operator<(Incidence const &other) const { return azimuth < other.azimuth; }

        /// Compare the azimuth of an incidence with the given angle.
        inline bool operator<(double angle) const { return azimuth < angle; }

        /// Check equality (only tests edges and their ends).
        inline bool operator==(Incidence const &other) const
        {
            return index == other.index && sign == other.sign;
        }
    };
    using IncIt = typename std::list<Incidence>::iterator;
    using IncConstIt = typename std::list<Incidence>::const_iterator;

    /** Represents the vertex of a planar graph. */
    class Vertex
    {
    private:
        Point const _position;            ///< Geometric position of the vertex.
        std::list<Incidence> _incidences; ///< List of incidences of edges to this vertex.
        unsigned mutable _flags = 0;      ///< User-settable flags.

        inline static Point::LexLess<X> const _cmp; ///< Point sorting function object.

    public:
        Vertex(Point const &pos)
            : _position{pos}
        {
        }

        /** Get the geometric position of the vertex. */
        Point const &point() const { return _position; }

        /** Get the list of incidences to the vertex. */
        auto const &getIncidences() const { return _incidences; }

        /** Compare vertices based on their coordinates (lexicographically). */
        bool operator<(Vertex const &other) const { return _cmp(_position, other._position); }

        unsigned flags() const { return _flags; } ///< Get the user flags.
        void setFlags(unsigned flags) const { _flags = flags; } ///< Set the user flags.

        /** Get the cyclic-next incidence after the passed one, in the CCW direction. */
        IncConstIt cyclicNextIncidence(IncConstIt it) const { return cyclic_next(it, _incidences); }

        /** Get the cyclic-next incidence after the passed one, in the CW direction. */
        IncConstIt cyclicPrevIncidence(IncConstIt it) const { return cyclic_prior(it, _incidences); }

        /** The same but with pointers. */
        Incidence *cyclicNextIncidence(Incidence *from)
        {
            return &(*cyclicNextIncidence(_incidencePtr2It(from)));
        }
        Incidence *cyclicPrevIncidence(Incidence *from)
        {
            return &(*cyclicPrevIncidence(_incidencePtr2It(from)));
        }

    private:
        /** Same as above, but not const (so only for private use). */
        IncIt cyclicNextIncidence(IncIt it) { return cyclic_next(it, _incidences); }
        IncIt cyclicPrevIncidence(IncIt it) { return cyclic_prior(it, _incidences); }

        /** Insert an incidence; for internal use by the PlanarGraph class. */
        Incidence &_addIncidence(unsigned edge_index, double azimuth, typename Incidence::Sign sign)
        {
            auto where = std::find_if(_incidences.begin(), _incidences.end(), [=](auto &inc) -> bool {
                return inc.azimuth >= azimuth;
            });
            return *(_incidences.emplace(where, edge_index, azimuth, sign));
        }

        /** Return a valid iterator to an incidence passed by pointer;
         * if the pointer is invalid, return a start iterator. */
        IncIt _incidencePtr2It(Incidence *pointer)
        {
            auto it = std::find_if(_incidences.begin(), _incidences.end(),
                                   [=](Incidence const &i) -> bool { return &i == pointer; });
            return (it == _incidences.end()) ? _incidences.begin() : it;
        }

        friend class PlanarGraph<EdgeLabel>;
    };
    using VertexIterator = typename std::list<Vertex>::iterator;

    /** Represents an edge of the planar graph. */
    struct Edge
    {
        Vertex *start = nullptr, *end = nullptr; ///< Start and end vertices.
        Path path; ///< The path associated to the edge.
        bool detached = false; ///< Whether the edge is detached from the graph.
        bool inserted_as_detached = false; ///< Whether the edge was inserted as detached.
        EdgeLabel mutable label; ///< The user-supplied label of the edge.

        /** Construct an edge with a given label. */
        Edge(Path &&movein_path, EdgeLabel &&movein_label)
            : path{movein_path}
            , label{movein_label}
        {
        }

        /** Detach the edge from the graph. */
        void detach()
        {
            detached = true;
            label.onDetach();
        }
    };
    using EdgeIterator = typename std::vector<Edge>::iterator;
    using EdgeConstIterator = typename std::vector<Edge>::const_iterator;

private:
    double const _precision; ///< Numerical epsilon for vertex clumping.
    std::list<Vertex> _vertices; ///< Vertices of the graph.
    std::vector<Edge> _edges; ///< Edges of the graph.
    std::vector< std::pair<Vertex *, Incidence *> > _junk; ///< Incidences that should be purged.
    bool _regularized = true; // An empty graph is (trivially) regularized.

public:
    PlanarGraph(Coord precision = EPSILON)
        : _precision{precision}
    {
    }

    std::list<Vertex> const &getVertices() const { return _vertices; }
    std::vector<Edge> const &getEdges() const { return _edges; }
    Edge const &getEdge(size_t index) const { return _edges.at(index); }
    size_t getEdgeIndex(Edge const &edge) const { return &edge - _edges.data(); }
    double getPrecision() const { return _precision; }
    size_t numVertices() const { return _vertices.size(); }
    size_t numEdges(bool include_detached = true) const
    {
        if (include_detached) {
            return _edges.size();
        }
        return std::count_if(_edges.begin(), _edges.end(),
                             [](Edge const &e) -> bool { return !e.detached; });
    }

    /** Check if the graph has been regularized. */
    bool isRegularized() const { return _regularized; }

    // 0x1p-50 is about twice the distance between M_PI and the next representable double.
    void regularize(double angle_precision = 0x1p-50, bool remove_collapsed_loops = true);

    /** Allocate memory to store the specified number of edges. */
    void reserveEdgeCapacity(size_t capacity) { _edges.reserve(capacity); }

    unsigned insertEdge(Path &&path, EdgeLabel &&edge = EdgeLabel());
    unsigned insertDetached(Path &&path, EdgeLabel &&edge = EdgeLabel());

    /** Edge insertion with a copy of the path. */
    unsigned insertEdge(Path const &path, EdgeLabel &&edge = EdgeLabel())
    {
        return insertEdge(Path(path), std::forward<EdgeLabel>(edge));
    }
    unsigned insertDetached(Path const &path, EdgeLabel &&edge = EdgeLabel())
    {
        return insertDetached(Path(path), std::forward<EdgeLabel>(edge));
    }

    /** \brief Find the incidence at the specified endpoint of the edge.
     *
     * \param edge_index The index of the edge whose incidence we wish to query.
     * \param sign Which end of the edge do we want an incidence of.
     * \return A pair consisting of pointers to the vertex and the incidence.
     *         If not found, both pointers will be null.
     */
    std::pair<Vertex *, Incidence *>
    getIncidence(unsigned edge_index, typename Incidence::Sign sign) const
    {
        if (edge_index >= _edges.size() || _edges[edge_index].detached) {
            return {nullptr, nullptr};
        }
        Vertex *vertex = (sign == Incidence::START) ? _edges[edge_index].start
                                                    : _edges[edge_index].end;
        if (!vertex) {
            return {nullptr, nullptr};
        }
        auto it = std::find(vertex->_incidences.begin(), vertex->_incidences.end(),
                            Incidence(edge_index, 42, sign)); // azimuth doesn't matter.
        if (it == vertex->_incidences.end()) {
            return {nullptr, nullptr};
        }
        return {vertex, &(*it)};
    }

    /**
     * \brief Go clockwise or counterclockwise around the vertex and find the next incidence.
     * The notions of "clockwise"/"counterclockwise" correspond to the y-axis pointing up.
     *
     * \param vertex The vertex around which to orbit.
     * \param incidence The incidence from which to start traversal.
     * \param clockwise Whether to go clockwise instead of (default) counterclockwise.
     * \return The next incidence encountered going in the specified direction.
     */
    inline Incidence const &nextIncidence(VertexIterator const &vertex, IncConstIt const &incidence,
                                          bool clockwise = false) const
    {
        return clockwise ? *(vertex->_cyclicPrevIncidence(incidence))
                         : *(vertex->_cyclicNextIncidence(incidence));
    }

    /** As above, but taking references instead of iterators. */
    inline Incidence const &nextIncidence(Vertex const &vertex, Incidence const &incidence,
                                          bool clockwise = false) const
    {
        IncConstIt it = std::find(vertex._incidences.begin(), vertex._incidences.end(), incidence);
        if (it == vertex._incidences.end()) {
            return incidence;
        }
        return clockwise ? *(vertex.cyclicPrevIncidence(it))
                         : *(vertex.cyclicNextIncidence(it));
    }

    /** As above, but return an iterator to a const incidence. */
    inline IncConstIt nextIncidenceIt(Vertex const &vertex, Incidence const &incidence,
                                    bool clockwise = false) const
    {
        IncConstIt it = std::find(vertex._incidences.begin(), vertex._incidences.end(), incidence);
        if (it == vertex._incidences.end()) {
            return vertex._incidences.begin();
        }
        return clockwise ? vertex.cyclicPrevIncidence(it)
                         : vertex.cyclicNextIncidence(it);
    }
    inline IncConstIt nextIncidenceIt(Vertex const &vertex, IncConstIt const &incidence,
                                    bool clockwise = false) const
    {
        return clockwise ? vertex.cyclicPrevIncidence(incidence)
                         : vertex.cyclicNextIncidence(incidence);
    }

    /** As above, but start at the prescribed departure azimuth (in radians).
     *
     * \return A pointer to the incidence emanating from the vertex at or immediately after
     *         the specified azimuth, when going around the vertex in the specified direction.
     *         If the vertex has no incidences, return value is nullptr.
    */
    Incidence *nextIncidence(VertexIterator const &vertex, double azimuth,
                             bool clockwise = false) const;

    /** Get the incident path, always oriented away from the vertex. */
    Path getOutgoingPath(Incidence const *incidence) const
    {
        return incidence ? _getPathImpl(incidence, Incidence::START) : Path();
    }

    /** Get the incident path, always oriented towards the vertex. */
    Path getIncomingPath(Incidence const *incidence) const
    {
        return incidence ? _getPathImpl(incidence, Incidence::END) : Path();
    }

private:
    inline Path _getPathImpl(Incidence const *incidence, typename Incidence::Sign origin) const
    {
        return (incidence->sign == origin) ? _edges[incidence->index].path
                                           : _edges[incidence->index].path.reversed();
    }

    /** Earmark an incidence for future deletion. */
    inline void _throwAway(Vertex *vertex, Incidence *incidence)
    {
        if (!vertex || !incidence) {
            return;
        }
        incidence->invalid = true;
        _junk.emplace_back(vertex, incidence);
    }

    // Topological reconfiguration functions; see their definitions for documentation.
    bool _compareAndReglue(Vertex &vertex, Incidence *first, Incidence *second, bool deloop);
    Vertex *_ensureVertexAt(Point const &position);
    void _mergeCoincidingEdges(Incidence *first, Incidence *second);
    void _mergeShorterLonger(Vertex &vertex, Incidence *shorter, Incidence *longer,
                             PathTime const &time_on_longer);
    void _mergeWyeConfiguration(Vertex &vertex, Incidence *first, Incidence *second,
                                PathIntersection const &split);
    void _purgeJunkIncidences();
    void _reglueLasso(Vertex &vertex, Incidence *first, Incidence *second,
                      PathIntersection const &split);
    bool _reglueTeardrop(Vertex &vertex, Incidence *first, Incidence *second, bool deloop);
    void _reglueTangentFan(Vertex &vertex, IncIt const &first, IncIt const &last, bool deloop);
    void _regularizeVertex(Vertex &vertex, double angle_precision, bool deloop);

    // === Static stuff ===

    /** Return the angle between the vector and the positive X axis or 0 if undefined. */
    inline static double _getAzimuth(Point const &vec) { return vec.isZero() ? 0.0 : atan2(vec); }

    /** Return path time corresponding to the same point on the reversed path. */
    inline static PathTime _reversePathTime(PathTime const &time, Path const &path)
    {
        int new_index = path.size() - time.curve_index - 1;
        Coord new_time = 1.0 - time.t;
        if (new_index < 0) {
            new_index = 0;
            new_time = 0;
        }
        return PathTime(new_index, new_time);
    }

    /** Return path time at the end of the path. */
    inline static PathTime _pathEnd(Path const &path) { return PathTime(path.size() - 1, 1.0); }
    inline static auto const PATH_START = PathTime(0, 0);

public:
    static double closedPathArea(Path const &path);
    static bool deviatesLeft(Path const &first, Path const &second);
};

/**
 * \brief Insert a new vertex or reuse an existing one.
 *
 * Ensures that there is a vertex at or near the specified position
 * (within the distance of _precision).
 *
 * \param pos The desired geometric position of the new vertex.
 * \return A pointer to the inserted vertex or a pre-existing vertex near the
 *         desired position.
 */
template<typename EL>
typename PlanarGraph<EL>::Vertex *PlanarGraph<EL>::_ensureVertexAt(Point const &pos)
{
    auto const insert_at_front = [&, this]() -> Vertex* {
        _vertices.emplace_front(pos);
        return &(_vertices.front());
    };

    if (_vertices.empty()) {
        return insert_at_front();
    }

    // TODO: Use a heap?
    auto it = std::find_if(_vertices.begin(), _vertices.end(), [&](Vertex const &v) -> bool {
        return Vertex::_cmp(pos, v._position); // existing vertex position > pos.
    });

    if (it != _vertices.end()) {
        if (are_near(pos, it->_position, _precision)) {
            return &(*it); // Reuse existing vertex.
        }
        if (it == _vertices.begin()) {
            return insert_at_front();
        }
    }
    // Look at the previous element, reuse if near, insert before `it` otherwise.
    return &(*(are_near(pos, std::prev(it)->_position, _precision) ? std::prev(it)
                                                                   : _vertices.emplace(it, pos)));
}

/**
 * \brief Move-insert a new labeled edge into the planar graph.
 *
 * \param path The geometric path of the edge.
 * \param label Optionally, the label (extra user data) associated to this edge.
 *              If absent, a default-constructed label will be used.
 * \return The index of the inserted edge.
 */
template<typename EdgeLabel>
unsigned PlanarGraph<EdgeLabel>::insertEdge(Path &&path, EdgeLabel &&label)
{
    unsigned edge_index = _edges.size();
    auto &inserted = _edges.emplace_back(std::forward<Path>(path),
                                         std::forward<EdgeLabel>(label));

    // Calculate the outgoing azimuths at both endpoints.
    double const start_azimuth = _getAzimuth(inserted.path.initialUnitTangent());
    double const end_azimuth = _getAzimuth(-inserted.path.finalUnitTangent());

    // Get the endpoints into the graph.
    auto start = _ensureVertexAt(inserted.path.initialPoint());
    auto end = _ensureVertexAt(inserted.path.finalPoint());

    // Inform the edge about its endpoints.
    inserted.start = start;
    inserted.end = end;

    // Add incidences at the endpoints.
    start->_addIncidence(edge_index, start_azimuth, Incidence::START);
    end->_addIncidence(edge_index, end_azimuth, Incidence::END);

    _regularized = false;
    return edge_index;
}

/**
 * \brief Move-insert a new labeled edge but do not connect it to the graph.
 *
 * Although the graph will hold the path data of an edge inserted in this way, the edge
 * will not be connected to any vertex. This can be used to store information about closed
 * paths (loops) in the instance, without having to specify starting points for them.
 *
 * \param path The geometric path of the edge.
 * \param label Optionally, the label (extra user data) associated to this edge; if absent,
 *              the label will be default-constructed.
 * \return The index of the inserted edge.
 */
template<typename EdgeLabel>
unsigned PlanarGraph<EdgeLabel>::insertDetached(Path &&path, EdgeLabel &&label)
{
    unsigned edge_index = _edges.size();
    auto &inserted = _edges.emplace_back(std::forward<Path>(path),
                                         std::forward<EdgeLabel>(label));
    inserted.detached = true;
    inserted.inserted_as_detached = true;
    return edge_index;
}

/** Remove incidences previously marked as junk. */
template<typename EdgeLabel>
void PlanarGraph<EdgeLabel>::_purgeJunkIncidences()
{
    for (auto &[vertex, incidence] : _junk) {
        Incidence *to_remove = incidence;
        auto it = std::find_if(vertex->_incidences.begin(), vertex->_incidences.end(),
                               [=](Incidence const &inc) -> bool { return &inc == to_remove; });
        if (it != vertex->_incidences.end()) {
            vertex->_incidences.erase(it);
        }
    }
    _junk.clear();
}

/**
 * \brief Merge overlapping edges or their portions, adding vertices if necessary.
 *
 * \param angle_precision The numerical epsilon for radian angle comparisons.
 * \param remove_collapsed_loops Whether to detach edges with both ends incident to the same
 *                               vertex (loops) when these loops don't enclose any area.
 *
 * This function performs the following operations:
 * \li Edges that are tangent at a vertex but don't otherwise overlap are sorted correctly
 *     in the counterclockwise cyclic order around the vertex.
 * \li Degenerate loops which don't enclose any area are removed if the argument is true.
 * \li Edges that coincide completely are reversed if needed and merged into one.
 * \li Edges that coincide partially are split into overlapping and non-overlapping portions.
 *     Any overlapping portions are oriented consistently and then merged.
 * \li As a sub-case of the above, any non-degenerate loop with an initial self-everlap
 *     (a "lasso") is replaced with a shorter non-overlapping loop and a simple path leading
 *     to it.
 */
template<typename EdgeLabel>
void PlanarGraph<EdgeLabel>::regularize(double angle_precision, bool remove_collapsed_loops)
{
    for (auto it = _vertices.begin(); it != _vertices.end(); ++it) {
        // Note: the list of vertices may grow during the execution of this loop,
        // so don't replace it with a range-for (which stores the end iterator).
        // We want the loop to carry on going over the elements it inserted.
        if (it->_incidences.size() < 2) {
            continue;
        }
        _regularizeVertex(*it, angle_precision, remove_collapsed_loops);
    }
    _purgeJunkIncidences();
    _regularized = true;
}

/**
 * \brief Analyze and regularize all edges emanating from a given vertex.
 *
 * This function goes through the list of incidences at the vertex (roughly sorted by
 * azimuth, i.e., departure heading in radians), picking out runs of mutually tangent
 * edges and calling _reglueTangentFan() on each run. The algorithm is quite complicated
 * because the incidences have to be treated as a cyclic container and a run of mutually
 * tangent edges may straddle the "end" of the list, including the possibility that the
 * entire list is a single such run.
 *
 * \param vertex The vertex whose incidences should be analyzed.
 * \param angle_precision The numerical epsilon for radian angle comparisons.
 * \param deloop Whether loops that don't enclose any area should be detached.
 */
template<typename EdgeLabel>
void PlanarGraph<EdgeLabel>::_regularizeVertex(typename PlanarGraph<EdgeLabel>::Vertex &vertex,
                                               double angle_precision, bool deloop)
{
    auto &incidences = vertex._incidences;

    /// Compare two polar angles in the interval [-π, π] modulo 2π to within angle_precision:
    auto const angles_equal = [=](double az1, double az2) -> bool {
        static double const twopi = 2.0 * M_PI;
        return are_near(std::fmod(az1 + twopi, twopi), std::fmod(az2 + twopi, twopi),
                        angle_precision);
    };

    IncIt run_begin; // First element in the last discovered run of equal azimuths.

    /// Find and reglue runs of nearly identical azimuths in the specified range.
    auto const process_runs = [&](IncIt begin, IncIt end) -> bool
    {
        double current_azimuth = 42; // Invalid radian angle.
        bool in_a_run = false;

        for (auto it = begin; it != end; ++it) {
            bool const equal = angles_equal(it->azimuth, current_azimuth);
            if (equal && !in_a_run) {
                run_begin = std::prev(it); // Save to enclosing scope.
                in_a_run = true;
            } else if (!equal && in_a_run) {
                _reglueTangentFan(vertex, run_begin, std::prev(it), deloop);
                in_a_run = false;
            }
            current_azimuth = it->azimuth;
        }
        return in_a_run;
    };

    double const last_azimuth = incidences.back().azimuth;

    if (angles_equal(incidences.front().azimuth, last_azimuth)) {
        // The cyclic list contains a run of equal azimuths which straddles the "end".
        // This means that we must skip the part of this run on the "begin" side on the
        // first pass and handle it once we've traversed the remainder of the list.

        bool processed = false; ///< Whether we've cleared the straddling run.
        double previous_azimuth = last_azimuth;
        IncIt straddling_run_last;

        for (auto it = incidences.begin(); it != incidences.end(); ++it) {
            if (!angles_equal(it->azimuth, previous_azimuth)) {
                straddling_run_last = std::prev(it);
                process_runs(it, incidences.end());
                processed = true;
                break;
            }
            previous_azimuth = it->azimuth;
        }
        if (processed) {
            // Find the first element of the straddling run.
            auto it = std::prev(incidences.end());
            while (angles_equal(it->azimuth, last_azimuth)) {
                --it;
            }
            ++it; // Now we're at the start of the straddling run.
            _reglueTangentFan(vertex, it, straddling_run_last, deloop);
        } else {
            // We never encountered anything outside of the straddling run: reglue everything.
            _reglueTangentFan(vertex, incidences.begin(), std::prev(incidences.end()), deloop);
        }
    } else if (process_runs(incidences.begin(), incidences.end())) {
        // Our run got rudely interrupted by the end of the container; reglue till the end.
        _reglueTangentFan(vertex, run_begin, std::prev(incidences.end()), deloop);
    }
}

/**
 * \brief Regularize a fan of mutually tangent edges emanating from a vertex.
 *
 * This function compares the tangent edges pairwise and ensures that the sequence of their
 * incidences to the vertex ends up being sorted by the ultimate direction in which the
 * emanating edges fan out, in the counterclockwise order.
 *
 * If a partial or complete overlap between edges is detected, these edges are reglued.
 *
 * \param vertex The vertex from which the fan emanates.
 * \param first An iterator pointing to the first incidence in the fan.
 * \param last An iterator pointing to the last incidence in the fan.
 *             NOTE: This iterator must point to the actual last incidence, not "past" it.
 *                   The reason is that we're iterating over a cyclic collection, so there
 *                   isn't really a meaningful end.
 * \param deloop Whether loops that don't enclose any area should be detached.
 */
template<typename EL>
void PlanarGraph<EL>::_reglueTangentFan(typename PlanarGraph<EL>::Vertex &vertex,
                                        typename PlanarGraph<EL>::IncIt const &first,
                                        typename PlanarGraph<EL>::IncIt const &last, bool deloop)
{
    // Search all pairs (triangular pattern), skipping invalid incidences.
    for (auto it = first; it != last; it = vertex.cyclicNextIncidence(it)) {
        if (it->invalid) {
            continue;
        }
        for (auto is = vertex.cyclicNextIncidence(it); true; is = vertex.cyclicNextIncidence(is)) {
            if (!is->invalid && _compareAndReglue(vertex, &(*it), &(*is), deloop)) {
                // Swap the incidences, effectively implementing "bubble sort".
                std::swap(*it, *is);
            }
            if (is == last) {
                break;
            }
        }
    }
}

/**
 * \brief Compare a pair of edges emanating from the same vertex in the same direction.
 *
 * If the edges overlap in part or in full, they get reglued, which means that the topology
 * of the graph may get modified. Otherwise, if the detailed comparison shows that the edges
 * aren't correctly ordered around the vertex (because the second edge deviates to the right
 * instead of to the left of the first, when looking away from the vertex), then the function
 * will return true, signalling that the incidences should be swapped.
 *
 * \param vertex The vertex where the mutually tangent paths meet.
 * \param first The incidence appearing as the first one in the provisional cyclic order.
 * \param second The incidence appearing as the second one in the provisional cyclic order.
 * \param deloop Whether to detach collapsed loops (backtracks) which don't enclose any area.
 * \return Whether the incidences should be swapped.
 */
template<typename EL>
bool PlanarGraph<EL>::_compareAndReglue(typename PlanarGraph<EL>::Vertex &vertex,
                                        typename PlanarGraph<EL>::Incidence *first,
                                        typename PlanarGraph<EL>::Incidence *second, bool deloop)
{
    if (first->index == second->index) {
        return _reglueTeardrop(vertex, first, second, deloop);
    }

    // Get paths corresponding to the edges but travelling away from the vertex.
    auto first_path_out = getOutgoingPath(first);
    auto second_path_out = getOutgoingPath(second);
    auto split = parting_point(first_path_out, second_path_out, _precision);

    if (are_near(split.point(), vertex.point(), _precision)) {
        // Paths deviate immediately, so no gluing is needed. The incidences should
        // be swapped if the first edge path departs to the left of the second one.
        return deviatesLeft(first_path_out, second_path_out);
    }

    // Determine the nature of the initial overlap between the paths.
    bool const till_end_of_1st = are_near(split.point(), first_path_out.finalPoint(), _precision);
    bool const till_end_of_2nd = are_near(split.point(), second_path_out.finalPoint(), _precision);

    if (till_end_of_1st && till_end_of_2nd) { // Paths coincide completely.
        _mergeCoincidingEdges(first, second);
    } else if (till_end_of_1st) {
        // Paths coincide until the end of the 1st one, which however isn't the end of the
        // 2nd one; for example, the first one could be the vertical riser of the letter L
        // whereas the second one – the entire letter stroke.
        _mergeShorterLonger(vertex, first, second, split.second);
    } else if (till_end_of_2nd) {
        // The same but with with the second edge shorter than the first one.
        _mergeShorterLonger(vertex, second, first, split.first);
    } else { // A Y-shaped split.
        _mergeWyeConfiguration(vertex, first, second, split);
    }
    return false; // We've glued so no need to swap anything.
}

/**
 * \brief Analyze a loop path a with self-tangency at the attachment point (a teardrop).
 *
 * The following steps are taken:
 * \li If the loop encloses zero area and \c deloop is true, the loop is detached.
 * \li If the two arms of the loop split out immediately, the loop is left alone and we
 *     only check whether the incidences should be swapped.
 * \li If the loop overlaps itself near the vertex, resembling a lasso, we split it into
 *     a shorter simple path and a smaller loop attached to the end of the shorter path.
 *
 * \param vertex The vertex at which the teardrop originates.
 * \param first The first incidence of the loop to the vertex.
 * \param second The second incidence of the loop to the vertex.
 * \param deloop Whether the loop should be removed if it doesn't enclose any area
 *               (i.e., the path exactly backtracks on itself).
 * \return Whether the two incidences of the loop to the vertex should be swapped.
 */
template<typename EL>
bool PlanarGraph<EL>::_reglueTeardrop(typename PlanarGraph<EL>::Vertex &vertex,
                                      typename PlanarGraph<EL>::Incidence *first,
                                      typename PlanarGraph<EL>::Incidence *second, bool deloop)
{
    // Calculate the area enclosed by the teardrop.
    // The convention is that the unit circle (cos(t), sint(t)), t from 0 to 2pi,
    // encloses an area of +pi.
    auto &edge = _edges[first->index];
    Path loop = edge.path; loop.close();
    double signed_area = closedPathArea(loop);

    if (deloop && are_near(signed_area, 0.0, _precision)) {
        edge.detach();
        _throwAway(&vertex, first);
        _throwAway(&vertex, second);
        return false;
    }

    auto split = parting_point(loop, loop.reversed(), _precision);
    if (are_near(split.point(), vertex.point(), _precision)) {
        // The loop spreads out immediately. We simply check if the incidences should be swapped.
        // We want them to be ordered such that the signed area encircled by the path going out
        // at the first incidence and coming back at the second (with this orientation) is > 0.
        return (first->sign == Incidence::START) ^ (signed_area > 0.0);
    }

    // The loop encloses a nonzero area, but the two branches don't separate at the starting
    // point. Instead, they travel together for a while before they split like a lasso.
    _reglueLasso(vertex, first, second, split);
    return false;
}

/**
 * \brief Reglue a lasso-shaped loop, separating it into the "free rope" and the "hoop".
 *
 * The lasso is an edge looping back to the same vertex, where the closed path encloses
 * a non-zero area, but its two branches don't separate at the starting point. Instead,
 * they travel together for a while (forming the doubled-up "free rope") before they
 * split like a lasso. This function cuts the lasso at the split point:
 * \code{.unparsed}
 *                ____                                                  ____
 *               /    \                                                /    \
 *  VERTEX =====<      |           ==>      VERTEX ------ NEW  +  NEW <      |
 *               \____/ (lasso)                    (rope)              \____/  (hoop)
 *
 * \endcode
 *
 * \param vertex A reference to the vertex where the lasso is attached.
 * \param first The first incidence of the lasso to the vertex.
 * \param second The second incidence of the lasso to the vertex.
 * \param split The point where the free rope of the lasso ends and the hoop begins.
 */
template<typename EL>
void PlanarGraph<EL>::_reglueLasso(typename PlanarGraph<EL>::Vertex &vertex,
                                   typename PlanarGraph<EL>::Incidence *first,
                                   typename PlanarGraph<EL>::Incidence *second,
                                   PathIntersection const &split)
{
    unsigned lasso = first->index;

    // Create the "free rope" path.
    auto rope = _edges[lasso].path.portion(PATH_START, split.first);
    rope.setInitial(vertex.point());
    rope.setFinal(split.point());
    double const rope_final_backward_azimuth = _getAzimuth(-rope.finalUnitTangent());

    // Compute the new label of the rope edge.
    auto oriented_as_loop = _edges[lasso].label;
    auto reversed = oriented_as_loop; reversed.onReverse();
    oriented_as_loop.onMergeWith(reversed);

    // Insert the rope and its endpoint.
    unsigned const rope_index = _edges.size();
    auto &rope_edge = _edges.emplace_back(std::move(rope), std::move(oriented_as_loop));
    auto const new_split_vertex = _ensureVertexAt(split.point());

    // Reuse lasso's first incidence as the incidence to the rope (azimuth can stay).
    first->index = rope_index;
    first->sign = Incidence::START;

    // Connect the rope to the newly created split vertex.
    new_split_vertex->_addIncidence(rope_index, rope_final_backward_azimuth, Incidence::END);
    rope_edge.start = &vertex;
    rope_edge.end = new_split_vertex;

    // Insert the hoop
    auto hoop = _edges[lasso].path.portion(split.first,
                                           _reversePathTime(split.second, _edges[lasso].path));
    hoop.setInitial(split.point());
    hoop.setFinal(split.point());
    insertEdge(std::move(hoop), EL(_edges[lasso].label));

    // Detach the original lasso edge and mark the second incidence for cleanup.
    _edges[lasso].detach();
    _throwAway(&vertex, second);
}

/**
 * \brief Completely coallesce two fully overlapping edges.
 *
 * In practice, the first edge stays and the second one gets detached from the graph.
 *
 * \param first An iterator to the first edge's incidence to a common vertex.
 * \param second An iterator to the second edge's incidence to a common vertex.
 */
template<typename EL>
void PlanarGraph<EL>::_mergeCoincidingEdges(typename PlanarGraph<EL>::Incidence *first,
                                            typename PlanarGraph<EL>::Incidence *second)
{
    auto &surviver = _edges[first->index];
    auto &casualty = _edges[second->index];

    auto other_label = casualty.label;
    if (first->sign != second->sign) { // Logically reverse the label before merging.
        other_label.onReverse();
    }
    surviver.label.onMergeWith(other_label);

    // Mark both incidences of the second edge as junk and detach it.
    auto [start_vertex, start_inc] = getIncidence(second->index, Incidence::START);
    _throwAway(start_vertex, start_inc);
    auto [end_vertex, end_inc] = getIncidence(second->index, Incidence::END);
    _throwAway(end_vertex, end_inc);
    casualty.detach();
}

/**
 * \brief Merge a longer edge with a shorter edge that overlaps it.
 *
 * In practice, the shorter edge remains unchanged and the longer one is trimmed to
 * become just the part extending past the shorter one.
 *
 * \param vertex The vertex where the overlap starts.
 * \param shorter The incidence of the shorter edge to the common vertex.
 * \param longer The incidence of the longer edge to the common vertex.
 * \param time_on_longer The PathTime on the longer edge at which it passes through
 *                       the endpoint of the shorter edge.
 */
template<typename EL>
void PlanarGraph<EL>::_mergeShorterLonger(typename PlanarGraph<EL>::Vertex &vertex,
                                          typename PlanarGraph<EL>::Incidence *shorter,
                                          typename PlanarGraph<EL>::Incidence *longer,
                                          PathTime const &time_on_longer)
{
    auto &shorter_edge = _edges[shorter->index];
    auto &longer_edge = _edges[longer->index];

    // Get the vertices at the far ends of both edges.
    auto shorter_far_end = (shorter->sign == Incidence::START) ? shorter_edge.end
                                                               : shorter_edge.start;
    /// Whether the longer edge heads out of the vertex.
    bool const longer_out = (longer->sign == Incidence::START);
    auto longer_far_end = longer_out ? longer_edge.end : longer_edge.start;

    // Copy the longer edge's label and merge with that of the shorter.
    auto longer_label = longer_edge.label;
    if (shorter->sign != longer->sign) {
        longer_label.onReverse();
    }
    shorter_edge.label.onMergeWith(longer_label);

    // Create the trimmed path (longer minus shorter).
    Path trimmed;
    double trimmed_departure_azimuth;
    if (longer_out) {
        trimmed = longer_edge.path.portion(time_on_longer, _pathEnd(longer_edge.path));
        longer_edge.start = shorter_far_end;
        trimmed.setInitial(shorter_far_end->point());
        trimmed.setFinal(longer_far_end->point());
        trimmed_departure_azimuth = _getAzimuth(trimmed.initialUnitTangent());
    } else {
        trimmed = longer_edge.path.portion(PATH_START, _reversePathTime(time_on_longer,
                                                                        longer_edge.path));
        longer_edge.end = shorter_far_end;
        trimmed.setInitial(longer_far_end->point());
        trimmed.setFinal(shorter_far_end->point());
        trimmed_departure_azimuth = _getAzimuth(-trimmed.finalUnitTangent());
    }

    // Set the trimmed path as the new path of the longer edge and set up the incidences:
    longer_edge.path = std::move(trimmed);
    shorter_far_end->_addIncidence(longer->index, trimmed_departure_azimuth, longer->sign);

    // Throw away the old start incidence of the longer edge.
    _throwAway(&vertex, longer);
}

/**
 * \brief Merge a pair of partially overlapping edges, producing a Y-split at a new vertex.
 *
 * This topological modification is performed by inserting a new vertex at the three-way
 * point (where the two paths separate) and clipping the original edges to that point.
 * In this way, the original edges become the "arms" of the Y-shape. In addition, a new
 * edge is inserted, forming the "stem" of the Y.
 *
 * \param vertex The vertex from which the partially overlapping edges originate (bottom of Y).
 * \param first The incidence to the first edge (whose path is the stem and one arm of the Y).
 * \param second The incidence to the second edge (stem and the other arm of the Y).
 * \param fork The splitting point of the two paths.
 */
template<typename EL>
void PlanarGraph<EL>::_mergeWyeConfiguration(typename PlanarGraph<EL>::Vertex &vertex,
                                             typename PlanarGraph<EL>::Incidence *first,
                                             typename PlanarGraph<EL>::Incidence *second,
                                             PathIntersection const &fork)
{
    bool const first_is_out = (first->sign == Incidence::START);
    bool const second_is_out = (second->sign == Incidence::START);

    auto &first_edge = _edges[first->index];
    auto &second_edge = _edges[second->index];

    // Calculate the path forming the stem of the Y:
    auto stem_path = getOutgoingPath(first).portion(PATH_START, fork.first);
    stem_path.setInitial(vertex.point());
    stem_path.setFinal(fork.point());

    /// A closure to clip the path of an original edge to the fork point.
    auto const clip_to_fork = [&](PathTime const &t, Edge &e, bool out) {
        if (out) { // Trim from time to end
            e.path = e.path.portion(t, _pathEnd(e.path));
            e.path.setInitial(fork.point());
        } else { // Trim from reverse-end to reverse-time
            e.path = e.path.portion(PATH_START, _reversePathTime(t, e.path));
            e.path.setFinal(fork.point());
        }
    };

    /// A closure to find the departing azimuth of an edge at the fork point.
    auto const departing_azimuth = [&](Edge const &e, bool out) -> double {
        return _getAzimuth((out) ? e.path.initialUnitTangent()
                                 : -e.path.finalUnitTangent());
    };

    // Clip the paths obtaining the arms of the Y.
    clip_to_fork(fork.first, first_edge, first_is_out);
    clip_to_fork(fork.second, second_edge, second_is_out);

    // Create the fork vertex and set up its incidences.
    auto const fork_vertex = _ensureVertexAt(fork.point());
    fork_vertex->_addIncidence(first->index, departing_azimuth(first_edge, first_is_out),
                               first->sign);
    fork_vertex->_addIncidence(second->index, departing_azimuth(second_edge, second_is_out),
                               second->sign);

    // Repoint the ends of the edges that were clipped
    (first_is_out ? first_edge.start : first_edge.end) = fork_vertex;
    (second_is_out ? second_edge.start : second_edge.end) = fork_vertex;

    /// A closure to get a consistently oriented label of an edge.
    auto upwards_oriented_label = [&](Edge const &e, bool out) -> EL {
        auto label = e.label;
        if (!out) {
            label.onReverse();
        }
        return label;
    };

    auto stem_label = upwards_oriented_label(first_edge, first_is_out);
    stem_label.onMergeWith(upwards_oriented_label(second_edge, second_is_out));
    auto stem_departure_from_fork = _getAzimuth(-stem_path.finalUnitTangent());

    // Insert the stem of the Y-configuration.
    unsigned const stem_index = _edges.size();
    auto &stem_edge = _edges.emplace_back(std::move(stem_path), std::move(stem_label));
    stem_edge.start = &vertex;
    stem_edge.end = fork_vertex;

    // Set up the incidences.
    fork_vertex->_addIncidence(stem_index, stem_departure_from_fork, Incidence::END);
    first->index = stem_index;
    first->sign = Incidence::START;
    _throwAway(&vertex, second);
}

template<typename EL>
typename PlanarGraph<EL>::Incidence*
PlanarGraph<EL>::nextIncidence(typename PlanarGraph<EL>::VertexIterator const &vertex,
                               double azimuth, bool clockwise) const
{
    auto &incidences = vertex._incidences;
    Incidence *result = nullptr;

    if (incidences.empty()) {
        return result;
    }
    // Normalize azimuth to the interval [-pi; pi].
    auto angle = Angle(azimuth);

    if (clockwise) { // Go backwards and find a lower bound
        auto it = std::find_if(incidences.rbegin(), incidences.rend(), [=](auto inc) -> bool {
            return inc.azimuth <= angle;
        });
        if (it == incidences.rend()) {
            // azimuth is lower than the azimuths of all incidences;
            // going clockwise we wrap back to the highest azimuth (last incidence).
            return &incidences.back();
        }
        result = &(*it);
    } else {
        auto it = std::find_if(incidences.begin(), incidences.end, [=](auto inc) -> bool {
            return inc.azimuth >= angle;
        });
        if (it == incidences.end()) {
            // azimuth is higher than the azimuths of all incidences;
            // going counterclockwise we wrap back to the lowest azimuth.
            return &incidences.front();
        }
        result = &(*it);
    }
    return result;
}

/** Return the signed area enclosed by a closed path. */
template<typename EL>
double PlanarGraph<EL>::closedPathArea(Path const &path)
{
    double area;
    Point _;
    centroid(path.toPwSb(), _, area);
    return -area; // Our convention is that the Y-axis points up
}

/** \brief Determine whether the first path deviates to the left of the second.
 *
 * The two paths are assumed to have identical or nearly identical starting points
 * but not an overlapping initial portion. The concept of "left" is based on the
 * y-axis pointing up.
 *
 * \param first The first path.
 * \param second The second path.
 *
 * \return True if the first path deviates towards the left of the second;
 *         False if the first path deviates towards the right of the second.
 */
template<typename EL>
bool PlanarGraph<EL>::deviatesLeft(Path const &first, Path const &second)
{
    auto start = middle_point(first.initialPoint(), second.initialPoint());
    auto tangent_between = middle_point(first.initialUnitTangent(), second.initialUnitTangent());
    if (tangent_between.isZero()) {
        return false;
    }
    auto tangent_line = Line::from_origin_and_vector(start, tangent_between);

    // Find the first non-degenerate curves on both paths
    std::unique_ptr<Curve> c[2];
    auto const find_first_nondegen = [](std::unique_ptr<Curve> &pointer, Path const &path) {
        for (auto const &c : path) {
            if (!c.isDegenerate()) {
                pointer.reset(c.duplicate());
                return;
            }
        }
    };

    find_first_nondegen(c[0], first);
    find_first_nondegen(c[1], second);
    if (!c[0] || !c[1]) {
        return false;
    }

    // Find the bounding boxes
    Rect const bounding_boxes[] {
        c[0]->boundsExact(),
        c[1]->boundsExact()
    };

    // For a bounding box, find the corner that goes the furthest in the direction of the
    // tangent vector.
    auto const furthest_corner = [&](Rect const &r) -> unsigned {
        Coord max_dot = dot(r.corner(0) - start, tangent_between);
        unsigned result = 0;
        for (unsigned i = 1; i < 4; i++) {
            auto current_dot = dot(r.corner(i), tangent_between);
            if (current_dot > max_dot) {
                max_dot = current_dot;
                result = i;
            } else if (current_dot == max_dot) {
                // Disambiguate based on proximity to the tangent line.
                auto const offset = start + tangent_between;
                if (distance(offset, r.corner(i)) < distance(offset, r.corner(result))) {
                    result = i;
                }
            }
        }
        return result;
    };

    // Calculate the corner points overlooking the "rift" between the paths.
    Point corner_points[2];
    for (size_t i : {0, 1}) {
        corner_points[i] = bounding_boxes[i].corner(furthest_corner(bounding_boxes[i]));
    }

    // Find a vantage point from which we can best observe the splitting paths.
    Point vantage_point;
    bool found = false;
    if (corner_points[0] != corner_points[1]) {
        auto line_connecting_corners = Line(corner_points[0], corner_points[1]);
        auto xing = line_connecting_corners.intersect(tangent_line);
        if (!xing.empty()) {
            vantage_point = xing[0].point();
            found = true;
        }
    }
    if (!found) {
        vantage_point = tangent_line.pointAt(tangent_line.timeAtProjection(corner_points[0]));
    }

    // Move to twice as far in the direction of the vantage point.
    vantage_point += vantage_point - start;

    // Find the points on both curves that are nearest to the vantage point.
    Coord nearest[2];
    for (size_t i : {0, 1}) {
        nearest[i] = c[i]->nearestTime(vantage_point);
    }

    // Clip to the nearest points and examine the closed contour.
    Path closed_contour(start);
    closed_contour.setStitching(true);
    closed_contour.append(c[0]->portion(0, nearest[0]));
    closed_contour = closed_contour.reversed();
    closed_contour.setStitching(true);
    closed_contour.append(c[1]->portion(0, nearest[1]));
    closed_contour.close();
    return !path_direction(closed_contour); // Reverse to match the convention that y-axis is up.
}

} // namespace Geom

#endif // LIB2GEOM_SEEN_PLANAR_GRAPH_H

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
