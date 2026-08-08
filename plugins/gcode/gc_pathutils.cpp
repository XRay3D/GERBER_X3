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

void mergePolylines(Geo::Polylines& polylines, double maxDist) {
    auto joinable = [maxDist](QPointF a, QPointF b) { return Geo::distance(a, b) <= maxDist; };

    // Приклеивает `src` к хвосту `dst`, не дублируя общую точку стыка.
    auto append = [](Geo::Polyline& dst, const Geo::Polyline& src) {
        dst.insert(dst.end(), src.begin() + 1, src.end());
    };

    bool merged;
    do {
        merged = false;
        for(std::size_t i{}; i < polylines.size() && !merged; ++i) {
            if(polylines[i].closed || polylines[i].size() < 2) continue;
            for(std::size_t j{}; j < polylines.size(); ++j) {
                if(i == j || polylines[j].closed || polylines[j].size() < 2) continue;

                Geo::Polyline& a = polylines[i];
                Geo::Polyline b = polylines[j];

                if(joinable(a.back(), b.front())) {
                } else if(joinable(a.back(), b.back())) {
                    b.reverse();
                } else if(joinable(a.front(), b.back())) {
                    a.reverse(), b.reverse();
                } else if(joinable(a.front(), b.front())) {
                    a.reverse();
                } else
                    continue;

                append(a, b);
                polylines.erase(polylines.begin() + j);
                merged = true;
                break;
            }
        }
    } while(merged);

    // Замкнувшийся сам на себя обрывок -- это контур, а не линия.
    for(Geo::Polyline& polyline: polylines)
        if(polyline.size() > 3 && joinable(polyline.front(), polyline.back()))
            polyline.close();
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
