/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gc_pathutils.h"

#include "geo/boolean.h"

#include "geo/polygon.h"
#include "geo/util.h"

#include <QRectF>
#include <algorithm>
#include <limits>

namespace GCode {

namespace {

// Внутри ли контур `inner` контура `outer`. Контуры не пересекаются, поэтому
// одной точки достаточно: либо весь контур внутри, либо весь снаружи.
bool isInside(const Geo::Polyline& inner, const Geo::Polygon& outer) {
    return !inner.empty() && outer.contains(inner.front());
}

} // namespace

Geo::Polyline boundingFrame(QRectF rect, double margin) {
    rect.adjust(-margin, -margin, margin, margin);
    return Geo::rectangle(rect.width(), rect.height(), rect.center());
}

Geo::Polyline boundingFrame(const Geo::Polylines& polylines, double margin) {
    QRectF rect;
    for(const Geo::Polyline& polyline: polylines)
        rect = rect.isNull() ? polyline.boundingRect() : rect.united(polyline.boundingRect());
    return boundingFrame(rect, margin);
}

NestingForest nestingForest(const Geo::Polylines& contours) {
    const std::size_t count = contours.size();
    NestingForest forest;
    forest.children.resize(count);
    forest.depth.assign(count, 0);

    // Полигон на контур строится один раз: точная проверка принадлежности
    // стоит куда дороже самого перебора пар.
    std::vector<Geo::Polygon> bodies;
    bodies.reserve(count);
    for(const Geo::Polyline& contour: contours)
        bodies.emplace_back(contour);

    // Родитель -- самый глубокий из объемлющих, поэтому сперва считаем
    // глубины, а уже потом выбираем по ним родителя.
    std::vector<std::vector<std::size_t>> parents(count);
    for(std::size_t i{}; i < count; ++i)
        for(std::size_t j{}; j < count; ++j)
            if(i != j && isInside(contours[i], bodies[j])) {
                parents[i].push_back(j);
                ++forest.depth[i];
            }

    for(std::size_t i{}; i < count; ++i) {
        if(parents[i].empty()) {
            forest.roots.push_back(i);
            continue;
        }
        const auto parent = *std::ranges::max_element(parents[i],
            {}, [&](std::size_t idx) { return forest.depth[idx]; });
        forest.children[parent].push_back(i);
    }

    return forest;
}

void mergePolylines(Geo::Polylines& polylines, double maxDist) { Geo::stitch(polylines, maxDist); }

void sortByProximity(Geo::Polyline& points, QPointF start) {
    for(std::size_t first{}; first < points.size(); ++first) {
        std::size_t nearest = first;
        double best = std::numeric_limits<double>::max();
        for(std::size_t i = first; i < points.size(); ++i)
            if(const double d = Geo::distance(start, points[i]); d < best)
                best = d, nearest = i;
        start = points[nearest];
        if(nearest != first) std::swap(points[first], points[nearest]);
    }
}

void sortByProximity(Geo::Polylines& polylines, QPointF start) {
    std::erase_if(polylines, [](const Geo::Polyline& p) { return p.empty(); });

    for(std::size_t first{}; first < polylines.size(); ++first) {
        std::size_t nearest = first;
        double best = std::numeric_limits<double>::max();
        for(std::size_t i = first; i < polylines.size(); ++i)
            if(const double d = Geo::distance(start, polylines[i].front()); d < best)
                best = d, nearest = i;
        start = polylines[nearest].back();
        if(nearest != first) std::swap(polylines[first], polylines[nearest]);
    }
}

void sortByProximity(std::vector<Geo::Polylines>& groups, QPointF start) {
    std::erase_if(groups, [](const Geo::Polylines& g) { return g.empty(); });

    for(std::size_t first{}; first < groups.size(); ++first) {
        std::size_t nearest = first;
        double best = std::numeric_limits<double>::max();
        for(std::size_t i = first; i < groups.size(); ++i) {
            if(groups[i].front().empty()) continue;
            if(const double d = Geo::distance(start, groups[i].front().front()); d < best)
                best = d, nearest = i;
        }
        if(!groups[nearest].back().empty())
            start = groups[nearest].back().back();
        if(nearest != first) std::swap(groups[first], groups[nearest]);
    }
}

void rotateToNearest(Geo::Polyline& polyline, QPointF point) {
    if(!polyline.closed || polyline.size() < 2) return;
    const auto nearest = std::ranges::min_element(polyline,
        {}, [point](const Geo::Vertex& v) { return Geo::distance(point, v); });
    std::rotate(polyline.begin(), nearest, polyline.end());
}

} // namespace GCode
