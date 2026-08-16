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
#include <cmath>
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

namespace {

// Ближайшая к point точка сегмента from -> to (с прогибом from.bulge):
// параметр t и сама точка.
std::pair<double, QPointF> closestOnSegment(const Geo::Vertex& from, const Geo::Vertex& to, QPointF point) {
    if(const auto arc = Geo::arcOf(from, to, from.bulge)) {
        // Угол точки от центра, отсчитанный от начала дуги ПО ХОДУ дуги: попал
        // в размах -- ближайшая точка на самой окружности, нет -- один из концов.
        const double a = std::atan2(point.y() - arc->center.y(), point.x() - arc->center.x());
        const double sweep = std::abs(arc->theta);
        double rel = std::fmod((a - arc->startAngle) * (arc->theta < 0.0 ? -1.0 : 1.0), 2.0 * pi);
        if(rel < 0.0) rel += 2.0 * pi;
        if(rel <= sweep) {
            const double t = rel / sweep;
            return {t, arc->pointAt(t)};
        }
        return Geo::distance(point, from) <= Geo::distance(point, to)
            ? std::pair{0.0, QPointF{from}}
            : std::pair{1.0, QPointF{to}};
    }
    const QPointF d = to - from;
    const double len2 = QPointF::dotProduct(d, d);
    const double t = len2 > 0.0 ? std::clamp(QPointF::dotProduct(point - from, d) / len2, 0.0, 1.0) : 0.0;
    return {t, from + d * t};
}

} // namespace

ClosestPoint closestPoint(const Geo::Polyline& polyline, QPointF point) {
    ClosestPoint best;
    std::size_t i{};
    for(const auto& [from, to]: Geo::segments(polyline)) {
        const auto [t, pt] = closestOnSegment(from, to, point);
        if(const double d = Geo::distance(point, pt); d < best.distance)
            best = {i, t, pt, d};
        ++i;
    }
    // Одинокая вершина сегментов не даёт, но точкой всё же является.
    if(polyline.size() == 1)
        best = {0, 0.0, polyline.front(), Geo::distance(point, polyline.front())};
    return best;
}

void rotateToClosest(Geo::Polyline& polyline, QPointF point) {
    if(!polyline.closed || polyline.size() < 2) return;
    const ClosestPoint cp = closestPoint(polyline, point);
    const std::size_t n = polyline.size();

    // Окружность -- две половины с прогибом +-1, и такой её пишет одной
    // командой «G2/G3 I J» вывод УП (GCode::File::savePath). Резать её на
    // четверть и три четверти незачем: половины просто пересобираются от новой
    // точки, вторая вершина -- диаметрально противоположная.
    if(n == 2 && polyline.front().bulge == polyline.back().bulge
        && std::abs(std::abs(polyline.front().bulge) - 1.0) < 1e-9) {
        if(const auto arc = Geo::arcOf(polyline.front(), polyline.back(), polyline.front().bulge)) {
            const double bulge = polyline.front().bulge;
            const QPointF opposite = arc->center * 2.0 - cp.point;
            polyline[0] = Geo::Vertex{cp.point, bulge};
            polyline[1] = Geo::Vertex{opposite, bulge};
            return;
        }
    }

    // Разрез у самого конца сегмента -- это его вершина, вставлять нечего:
    // вершина в допуске сварки на выводе всё равно схлопнется в предыдущую.
    const Geo::Vertex& from = polyline[cp.segment];
    const Geo::Vertex& to = polyline[(cp.segment + 1) % n];
    const double len = Geo::segmentLength(from, to);
    std::size_t start = cp.segment;
    if(cp.t * len >= Geo::exitWeldTolerance && (1.0 - cp.t) * len >= Geo::exitWeldTolerance) {
        // Вершина реза: у головы прогиб её доли дуги, у хвоста -- остатка.
        Geo::Vertex cut{cp.point};
        if(const auto arc = Geo::arcOf(from, to, from.bulge)) {
            polyline[cp.segment].bulge = Geo::bulgeOf(arc->theta * cp.t);
            cut.bulge = Geo::bulgeOf(arc->theta * (1.0 - cp.t));
        }
        polyline.insert(polyline.begin() + static_cast<std::ptrdiff_t>(cp.segment) + 1, cut);
        start = cp.segment + 1;
    } else if((1.0 - cp.t) * len < Geo::exitWeldTolerance) {
        start = (cp.segment + 1) % n;
    }
    std::rotate(polyline.begin(), polyline.begin() + static_cast<std::ptrdiff_t>(start), polyline.end());
}

} // namespace GCode
