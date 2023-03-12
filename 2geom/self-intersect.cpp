/**
 * @file Implementation of Path::intersectSelf() and PathVector::intersectSelf().
 */
/* An algorithm for finding self-intersections of paths and path-vectors.
 *
 * Authors:
 *   Rafał Siejakowski <rs@rs-math.net>
 *
 * (C) Copyright 2022 Authors
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

#include <list>

#include <2geom/coord.h>
#include <2geom/curve.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/point.h>
#include <2geom/sweeper.h>

namespace Geom {

/** @brief The PathSelfIntersector class is a sweepset class used for intersecting curves in the
 * same path with one another. It is intended to be used as the template parameter of Sweeper.
 */
class PathSelfIntersector
{
public:
    using ItemIterator = Path::iterator;

private:
    Path _path; ///< The path searched for self-crossings, cleaned of degenerate curves.
    std::list<ItemIterator> _active; ///< List of active curves during the sweepline passage.
    std::vector<PathIntersection> _crossings; ///< Stores the crossings found.
    std::vector<size_t> _original_indices; ///< Curve indices before removal of degenerate curves.
    double const _precision; ///< Numerical epsilon.

public:
    PathSelfIntersector(Path const &path, double precision)
        : _path{path.initialPoint()}
        , _precision{precision}
    {
        _original_indices.reserve(path.size());
        for (size_t i = 0; i < path.size(); i++) {
            if (!path[i].isDegenerate()) {
                _path.append(path[i]);
                _original_indices.push_back(i);
            }
        }
        _path.close(path.closed());
    }

    // === SweepSet API ===
    auto &items() { return _path; }
    Interval itemBounds(ItemIterator curve) const { return curve->boundsFast()[X]; }
    /// Callback for when the sweepline starts intersecting a new item.
    void addActiveItem(ItemIterator incoming)
    {
        _intersectWithActive(incoming);
        _intersectWithSelf(incoming);
        _active.push_back(incoming);
    }
    /// Callback for when the sweepline stops intersecting an item.
    void removeActiveItem(ItemIterator to_remove)
    {
        auto it = std::find(_active.begin(), _active.end(), to_remove);
        _active.erase(it);
    }
    // ===

    std::vector<PathIntersection> &&moveOutCrossings() { return std::move(_crossings); }

private:
    /** Find and store all intersections of a curve with itself. */
    void _intersectWithSelf(ItemIterator curve)
    {
        size_t const index = std::distance(_path.begin(), curve);
        for (auto &&self_x : curve->intersectSelf(_precision)) {
            _appendCurveCrossing(std::move(self_x), index, index);
        }
    }

    /** Find and store all intersections of a curve with the active curves. */
    void _intersectWithActive(ItemIterator curve)
    {
        size_t const index = std::distance(_path.begin(), curve);
        for (auto const &other : _active) {
            if (!curve->boundsFast().intersects(other->boundsFast())) {
                continue;
            }

            size_t const other_index = std::distance(_path.begin(), other);
            auto const &[smaller, larger] = std::minmax(index, other_index);
            /// Whether the curves meet at a common node in the path.
            bool consecutive = smaller + 1 == larger;
            /// Whether the curves meet at the closure point of the path.
            bool wraparound = _path.closed() && smaller == 0 && larger + 1 == _path.size();
            for (auto &&xing : curve->intersect(*other, _precision)) {
                _appendCurveCrossing(std::move(xing), index, other_index, consecutive, wraparound);
            }
        }
    }

    /** Append a curve crossing to the store as long as it satisfies nondegeneracy criteria. */
    void _appendCurveCrossing(CurveIntersection &&xing, size_t first_index, size_t second_index,
                              bool consecutive = false, bool wraparound = false)
    {
        // Filter out crossings that aren't real but rather represent the agreement of final
        // and initial points of consecutive curves – a consequence of the path's continuity.
        auto const should_exclude = [&](bool flipped) -> bool {
            // Filter out spurious self-intersections by using squared geometric average.
            bool const first_is_first = (first_index < second_index) ^ flipped;
            double const geom2 = first_is_first ? (1.0 - xing.first) * xing.second
                                                : (1.0 - xing.second) * xing.first;
            return geom2 < EPSILON;
        };

        if ((consecutive && should_exclude(false)) || (wraparound && should_exclude(true))) {
            return;
        }

        // Convert curve indices to the original ones (before the removal of degenerate curves).
        _crossings.emplace_back(PathTime(_original_indices[first_index], xing.first),
                                PathTime(_original_indices[second_index], xing.second),
                                xing.point());
    }
};

// Compute all crossings of a path with itself.
std::vector<PathIntersection> Path::intersectSelf(Coord precision) const
{
    auto intersector = PathSelfIntersector(*this, precision);
    Sweeper(intersector).process();
    auto result = intersector.moveOutCrossings();
    std::sort(result.begin(), result.end());
    return result;
}

/**
 * @brief The PathVectorSelfIntersector class is an implementation of a SweepSet whose intended
 * use is the search for self-intersections in a single PathVector. It's designed to be used as
 * the template parameter for the Sweeper class template.
 */
class PathVectorSelfIntersector
{
public:
    using ItemIterator = PathVector::const_iterator;

private:
    PathVector const &_pathvector; ///< A reference to the path-vector searched for self-crossings.
    std::list<ItemIterator> _active; ///< A list of active paths during sweepline passage.
    std::vector<PathVectorIntersection> _crossings; ///< Stores the crossings found.
    double const _precision; ///< Numerical epsilon.

public:
    PathVectorSelfIntersector(PathVector const &subject, double precision)
        : _pathvector{subject}
        , _precision{precision}
    {
    }

    // == SweepSet API ===
    auto const &items() { return _pathvector; }
    Interval itemBounds(ItemIterator path)
    {
        auto const r = path->boundsFast();
        return r ? (*r)[X] : Interval(); // Sweeplines are vertical
    }

    /// Callback for when the sweepline starts intersecting a new item.
    void addActiveItem(ItemIterator incoming)
    {
        _intersectWithActive(incoming);
        _intersectWithSelf(incoming);
        _active.push_back(incoming);
    }

    /// Callback for when the sweepline stops intersecting an item.
    void removeActiveItem(ItemIterator to_remove)
    {
        auto it = std::find(_active.begin(), _active.end(), to_remove);
        _active.erase(it);
    }
    // ===

    std::vector<PathVectorIntersection> &&moveOutCrossings() { return std::move(_crossings); }

private:
    /**
     * @brief Find all intersections of the path pointed to by the given
     *        iterator with all currently active paths and store results
     *        in the instance of the class.
     *
     * @param it An iterator to a path to be intersected with the active ones.
     */
    void _intersectWithActive(ItemIterator &it);

    /**
     * @brief Find all intersections of the path pointed to by the given
     *        iterator with itself and store the results in the class instance.
     *
     * @param it An iterator to a path which will be intersected with itself.
     */
    void _intersectWithSelf(ItemIterator &it);

    /// Append a path crossing to the store.
    void _appendPathCrossing(PathIntersection const &xing, size_t first_index, size_t second_index)
    {
        auto const first_time  = PathVectorTime(first_index,  xing.first);
        auto const second_time = PathVectorTime(second_index, xing.second);
        _crossings.emplace_back(first_time, second_time, xing.point());
    }

public:

    std::vector<PathVectorIntersection>
    filterDeduplicate(std::vector<PathVectorIntersection> &&xings) const;
};

/** Remove duplicate intersections (artifacts of the path/curve crossing algorithms). */
std::vector<PathVectorIntersection>
PathVectorSelfIntersector::filterDeduplicate(std::vector<PathVectorIntersection> &&xings) const
{
    std::vector<PathVectorIntersection> result;
    result.reserve(xings.size());

    auto const are_same_times = [&](Coord a1, Coord a2, Coord b1, Coord b2) -> bool {
        return (are_near(a1, b1) && are_near(a2, b2)) ||
               (are_near(a1, b2) && are_near(a2, b1));
    };

    Coord last_time_1 = -1.0, last_time_2 = -1.0; // Invalid path times
    for (auto &&x : xings) {
        auto const current_1 = x.first.asFlatTime(), current_2 = x.second.asFlatTime();
        if (!are_same_times(current_1, current_2, last_time_1, last_time_2)) {
            result.push_back(std::move(x));
        }
        last_time_1 = current_1;
        last_time_2 = current_2;
    }

    return result;
}

/** Compute and store intersections of a path with all active paths. */
void PathVectorSelfIntersector::_intersectWithActive(ItemIterator &it)
{
    auto const start = _pathvector.begin();
    for (auto &path : _active) {
        if (!path->boundsFast().intersects(it->boundsFast())) {
            continue;
        }
        for (auto &&xing : path->intersect(*it, _precision)) {
            _appendPathCrossing(std::move(xing), std::distance(start, path),
                                std::distance(start, it));
        }
    }
}

/** Compute and store intersections of a constituent path with itself. */
void PathVectorSelfIntersector::_intersectWithSelf(ItemIterator &it)
{
    size_t const path_index = std::distance(_pathvector.begin(), it);
    for (auto &&xing : it->intersectSelf(_precision)) {
        _appendPathCrossing(std::move(xing), path_index, path_index);
    }
}

// Compute self-intersections in a path-vector.
std::vector<PathVectorIntersection> PathVector::intersectSelf(Coord precision) const
{
    auto intersector = PathVectorSelfIntersector(*this, precision);
    Sweeper(intersector).process();
    auto result = intersector.moveOutCrossings();
    std::sort(result.begin(), result.end());
    return (result.size() > 1) ? intersector.filterDeduplicate(std::move(result)) : result;
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
