/** @file Implementation of parting_point(Path const&, Path const&, Coord)
 */
/* An algorithm to find the first parting point of two paths.
 *
 * Authors:
 *   Rafał Siejakowski <rs@rs-math.net>
 *
 * Copyright 2022 the Authors.
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

#include <2geom/path.h>
#include <2geom/point.h>

namespace Geom
{

PathIntersection parting_point(Path const &first, Path const &second, Coord precision)
{
    Path const *paths[2] = { &first, &second };
    Point const starts[2] = { first.initialPoint(), second.initialPoint() };

    if (!are_near(starts[0], starts[1], precision)) {
        auto const invalid = PathTime(0, -1.0);
        return PathIntersection(invalid, invalid, middle_point(starts[0], starts[1]));
    }

    if (first.empty() || second.empty()) {
        auto const start_time = PathTime(0, 0.0);
        return PathIntersection(start_time, start_time, middle_point(starts[0], starts[1]));
    }

    size_t const curve_count[2] = { first.size(), second.size() };
    Coord const max_time[2] = { first.timeRange().max(), second.timeRange().max() };

    /// Curve indices up until which the paths are known to overlap
    unsigned pos[2] = { 0, 0 };
    /// Curve times on the curves with indices pos[] up until which the
    /// curves are known to overlap ahead of the nodes.
    Coord curve_times[2] = { 0.0, 0.0 };

    bool leg = 0; ///< Flag indicating which leg is stepping on the ladder
    bool just_changed_legs = false;

    /* The ladder algorithm takes steps along the two paths, as if they the stiles of
     * an imaginary ladder. Note that the nodes (X) on boths paths may not coincide:
     *
     * paths[0] START--------X-----------X-----------------------X---------X----> ...
     * paths[1] START--------------X-----------------X-----------X--------------> ...
     *
     * The variables pos[0], pos[1] are the indices of the nodes we've cleared so far;
     * i.e., we know that the portions before pos[] overlap.
     *
     * In each iteration of the loop, we move to the next node along one of the paths;
     * the variable `leg` tells us which path. We find the point nearest to that node
     * on the first unprocessed curve of the other path and check the curve time.
     *
     * Suppose the current node positions are denoted by P; one possible location of
     * the nearest point (N) to the next node is:
     *
     * ----P------------------N--X---- paths[!leg]
     * --------P--------------X------- paths[leg] (we've stepped forward from P to X)
     *
     * We detect this situation when we find that the curve time of N is < 1.0.
     * We then create a trimmed version of the top curve so that it corresponds to
     * the current bottom curve:
     *
     * ----P----------------------N--X---------- paths[!leg]
     *         [------------------]              trimmed curve
     * --------P------------------X------------- paths[leg]
     *
     * Using isNear(), we can compare the trimmed curve to the front curve (P--X) on
     * paths[leg]; if they are indeed near, then pos[leg] can be incremented.
     *
     * Another possibility is that we overstep the end of the other curve:
     *
     * ----P-----------------X------------------ paths[!leg]
     *                       N
     * --------P------------------X------------- paths[leg]
     *
     * so the nearest point N now coincides with a node on the top path. We detect
     * this situation by observing that the curve time of N is close to 1. In case
     * of such overstep, we change legs by flipping the `leg` variable:
     *
     * ----P-----------------X------------------ paths[leg]
     * --------P------------------X------------- paths[!leg]
     *
     * We can now continue the stepping procedure, but the next step will be taken on
     * the path `paths[leg]`, so it should be a shorter step (if it isn't, the paths
     * must have diverged and we're done):
     *
     * ----P-----------------X------------------ paths[leg]
     * --------P-------------N----X------------- paths[!leg]
     *
     * Another piece of data we hold on to are the curve times on the current curves
     * up until which the paths have been found to coincide. In other words, at every
     * step of the algorithm we know that the curves agree up to the path-times
     * PathTime(pos[i], curve_times[i]).
     *
     * In the situation mentioned just above, the times (T) will be as follows:
     *
     * ----P---T-------------X------------------ paths[leg]
     *
     * --------P-------------N----X------------- paths[!leg]
     *         T
     *
     * In this example, the time on top path is > 0.0, since the T mark is further
     * ahead than P on that path. This value of the curve time is needed to correctly
     * crop the top curve for the purpose of the isNear() comparison:
     *
     * ----P---T-------------X---------- paths[leg]
     *         [-------------]           comparison curve (cropped from paths[leg])
     *         [-------------]           comparison curve (cropped from paths[!leg])
     * --------P-------------N----X----- paths[!leg]
     *         T
     *
     * In fact, the lower end of the curve time range for cropping is always
     * given by curve_times[i].
     *
     * The iteration ends when we find that the two paths have diverged or when we
     * reach the end. When that happens, the positions and curve times will be
     * the PathTime components of the actual point of divergence on both paths.
     */

    /// A closure to crop and compare the curve pieces ([----] in the diagrams above).
    auto const pieces_agree = [&](Coord time_on_other) -> bool {
        Curve *pieces[2];
        // The leg-side curve is always cropped to the end:
        pieces[ leg] = paths[ leg]->at(pos[ leg]).portion(curve_times[ leg], 1.0);
        // The other one is cropped to a variable curve time:
        pieces[!leg] = paths[!leg]->at(pos[!leg]).portion(curve_times[!leg], time_on_other);
        bool ret = pieces[0]->isNear(*pieces[1], precision);
        delete pieces[0];
        delete pieces[1];
        return ret;
    };

    /// A closure to skip degenerate curves; returns true if we reached the end.
    auto const skip_degenerates = [&](size_t which) -> bool {
        while (paths[which]->at(pos[which]).isDegenerate()) {
            ++pos[which];
            curve_times[which] = 0.0;
            if (pos[which] == curve_count[which]) {
                return true; // We've reached the end
            }
        }
        return false;
    };

    // Main loop of the ladder algorithm.
    while (pos[0] < curve_count[0] && pos[1] < curve_count[1]) {
        // Skip degenerate curves if any.
        if (skip_degenerates(0)) {
            break;
        }
        if (skip_degenerates(1)) {
            break;
        }

        // Try to step to the next node with the current leg and see what happens.
        Coord forward_coord = (Coord)(pos[leg] + 1);
        if (forward_coord > max_time[leg]) {
            forward_coord = max_time[leg];
        }
        auto const step_point = paths[leg]->pointAt(forward_coord);
        auto const time_on_other = paths[!leg]->at(pos[!leg]).nearestTime(step_point);

        if (are_near(time_on_other, 1.0, precision) &&
            are_near(step_point, paths[!leg]->pointAt(pos[!leg] + 1), precision))
        { // The step took us very near to the first uncertified node on the other path.
            just_changed_legs = false;
            //
            // -------PT-----------------X---------- paths[!leg]
            // --P-----T-----------------X---------- paths[leg]
            //                           ^
            //                            endpoints (almost) coincide
            //
            // We should compare the curves cropped to the end:
            //
            // --------T-----------------X---------- paths[!leg]
            //         [-----------------]
            //         [-----------------]
            // --------T-----------------X---------- paths[leg]
            if (pieces_agree(1.0)) {
                // The curves are nearly identical, so we advance both positions
                // and zero out the forward curve times.
                for (size_t i = 0; i < 2; i++) {
                    pos[i]++;
                    curve_times[i] = 0.0;
                }
            } else { // We've diverged.
                break;
            }
        } else if (time_on_other < 1.0 - precision) {
            just_changed_legs = false;

            // The other curve is longer than our step! We trim the other curve to the point
            // nearest to the step point and compare the resulting pieces.
            //
            // --------T-----------------N--------X---- paths[!leg]
            //         [-----------------]
            //         [-----------------]
            // --------T-----------------X------------- paths[leg]
            //
            if (pieces_agree(time_on_other)) { // The curve pieces are near to one another!
                // We can advance our position and zero out the curve time:
                pos[leg]++;
                curve_times[leg] = 0.0;
                // But on the other path, we can only advance the time, not the curve index:
                curve_times[!leg] = time_on_other;
            } else { // We've diverged.
                break;
            }
        } else {
            // The other curve is shorter than ours, which means that we've overstepped.
            // We change legs and try to take a shorter step in the next iteration.
            if (just_changed_legs) {
                // We already changed legs before and it didn't help, i.e., we've diverged.
                break;
            } else {
                leg = !leg;
                just_changed_legs = true;
            }
        }
    }

    // Compute the parting time on both paths
    PathTime path_times[2];
    for (size_t i = 0; i < 2; i++) {
        path_times[i] = (pos[i] == curve_count[i]) ? PathTime(curve_count[i] - 1, 1.0)
                                                   : PathTime(pos[i], curve_times[i]);
    }

    // Get the parting point from the numerically nicest source
    Point parting_pt;
    if (curve_times[0] == 0.0) {
        parting_pt = paths[0]->pointAt(path_times[0]);
    } else if (curve_times[1] == 0.0) {
        parting_pt = paths[1]->pointAt(path_times[1]);
    } else {
        parting_pt = middle_point(first.pointAt(path_times[0]), second.pointAt(path_times[1]));
    }

    return PathIntersection(path_times[0], path_times[1], std::move(parting_pt));
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
