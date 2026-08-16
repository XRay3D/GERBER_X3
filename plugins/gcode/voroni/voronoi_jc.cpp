/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "voronoi_jc.h"
#include "jc_voronoi.h"
#include "types.h"

#include <QLineF>
#include <map>

namespace Voronoi {

void VoronoiJc::jcVoronoi() {
    const auto tolerance = gcp.params[Tolerance].toDouble();

    std::vector<jcv_point> points;
    points.reserve(100000);
    groupedPaths(GCode::Grouping::Copper);
    int32_t id{};

    // Дробит длинный отрезок на точки не реже tolerance: jc_voronoi строит
    // диаграмму по точкам, а не по отрезкам, и без дробления длинная прямая
    // грань меди осталась бы одним-единственным сайтом.
    auto condei = [&points, tolerance, &id](QPointF tmp, QPointF point) {
        QLineF line{tmp, point};
        if(line.length() > tolerance) {
            const auto total = static_cast<size_t>(line.length() / tolerance);
            for(size_t i = 1; i < total; ++i) {
                line.setLength(static_cast<double>(i) * tolerance);
                const QPointF pt = line.p2();
                points.push_back({static_cast<jcv_real>(pt.x()), static_cast<jcv_real>(pt.y()), id});
            }
        }
    };

    // Полилиния -- точками. Дуга крошится хордами по стреле прогиба, чтобы
    // круглый пад остался круглым; прямые куски дробит condei.
    auto addPolyline = [&](const Geo::Polyline& path) {
        if(path.empty()) return;
        const QPointF first = path.front();
        auto addPoint = [&](QPointF pt) {
            if(pt == first) return; // замыкающий сегмент приводит к началу -- оно уже добавлено
            points.push_back({static_cast<jcv_real>(pt.x()), static_cast<jcv_real>(pt.y()), id});
        };
        points.push_back({static_cast<jcv_real>(first.x()), static_cast<jcv_real>(first.y()), id});
        QPointF tmp = first;
        for(auto&& [from, to]: Geo::segments(path)) {
            if(const auto arc = Geo::arcOf(from, to, from.bulge)) {
                const int steps = arcChordSteps(*arc);
                for(int i = 1; i <= steps; ++i) {
                    const QPointF pt = i == steps ? QPointF{to} : arc->pointAt(static_cast<double>(i) / steps);
                    condei(tmp, pt);
                    addPoint(pt);
                    tmp = pt;
                }
            } else {
                condei(tmp, to);
                addPoint(to);
                tmp = to;
            }
        }
    };

    for(const Geo::Polygon& body: groupedPss) {
        ++id; // тег -- тело целиком: между внешним контуром и его же дыркой резать нечего
        for(const Geo::Polyline& contour: body.contours())
            addPolyline(contour);
    }

    for(const Geo::Polyline& path: openSrcPaths) {
        ++id;
        addPolyline(path);
    }

    QRectF bounds = Geo::Polygons{std::span<const Geo::Polygon>{groupedPss}}.boundingRect();
    if(!openSrcPaths.empty())
        bounds |= Geo::Polygons{openSrcPaths}.boundingRect();

    std::map<int, Pairs> edges;
    Pairs frame;
    {
        const double fo = gcp.params[FrameOffset].toDouble();
        jcv_rect bounding_box = {
            {static_cast<jcv_real>(bounds.left() - fo),  static_cast<jcv_real>(bounds.top() - fo)   },
            {static_cast<jcv_real>(bounds.right() + fo), static_cast<jcv_real>(bounds.bottom() + fo)}
        };
        jcv_diagram diagram;
        jcv_diagragenerate_(static_cast<int>(points.size()), points.data(), &bounding_box, nullptr, &diagram);
        auto toPoint = [](const jcv_edge* edge, int num) -> QPointF {
            return {edge->pos[num].x, edge->pos[num].y};
        };
        const jcv_site* sites = jcv_diagraget_sites_(&diagram);
        for(int i{}; i < diagram.numsites; i++) {
            jcv_graphedge* graph_edge = sites[i].edges;
            while(graph_edge) {
                const jcv_edge* edge = graph_edge->edge;
                const Pair pair{toPoint(edge, 0), toPoint(edge, 1), sites[i].p.id};
                if(edge->sites[0] == nullptr || edge->sites[1] == nullptr)
                    frame.insert(pair); // frame
                else if(edge->sites[0]->p.id != edge->sites[1]->p.id)
                    edges[edge->sites[0]->p.id ^ edge->sites[1]->p.id].insert(pair); // other
                graph_edge = graph_edge->next;
            }
        }
        jcv_diagrafree_(&diagram);
    }

    for(const auto& [key, edge]: edges)
        returnPs.append_range(toPath(edge));
    // Сшивка групп между собой -- тем же O(n)-склеем по точному совпадению
    // концов, а не Geo::stitch: тот кубичен по числу обрывков.
    returnPs = chainDiagramEdges(std::move(returnPs));
    returnPs.append_range(toPath(frame));
    for(size_t i{}; i < returnPs.size();) // remove very short paths
        if(returnPs[i].size() < 4 && Geo::distance(returnPs[i].front(), returnPs[i].back()) < tolerance * 0.5)
            returnPs.erase(returnPs.begin() + static_cast<std::ptrdiff_t>(i));
        else
            ++i;
}

Geo::Polylines VoronoiJc::toPath(const Pairs& pairs) {
    setMsg(QObject::tr("Merge Segments"));

    Geo::Polylines segments;
    segments.reserve(pairs.size());
    for(auto&& [p1, p2, id]: pairs) {
        Q_UNUSED(id)
        segments.push_back(Geo::Polyline{Geo::Vertex{p1}, Geo::Vertex{p2}});
    }

    Geo::Polylines paths = chainDiagramEdges(std::move(segments));

    auto clean = [this, kAngle = 2.0](Geo::Polyline& path) {
        for(size_t i = 1; i + 2 < path.size(); ++i) {
            QLineF line{path[i], path[i + 1]};
            if(line.length() < gcp.params[Tolerance].toDouble()) {
                path[i] = Geo::Vertex{line.center()};
                path.erase(path.begin() + static_cast<std::ptrdiff_t>(i + 1));
                --i;
            }
        }
        for(size_t i = 1; i + 1 < path.size(); ++i) {
            const double a1 = Geo::angleTo(path[i - 1], path[i]);
            const double a2 = Geo::angleTo(path[i], path[i + 1]);
            if(std::abs(a1 - a2) < kAngle) {
                path.erase(path.begin() + static_cast<std::ptrdiff_t>(i));
                --i;
            }
        }
    };
    r::for_each(paths, clean);

    return paths;
}

} // namespace Voronoi
