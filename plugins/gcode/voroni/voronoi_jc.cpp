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

size_t qHash(const Point64& key, uint /*seed*/ = 0) { return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(Point64))); }

namespace Voronoi {

void VoronoiJc::jcVoronoi() {
    const auto tolerance = gcp.params[Tolerance].toDouble();

    mvector<jcv_point> points;
    points.reserve(100000);
    CleanPaths(closedSrcPaths, tolerance * 0.1 * uScale);
    groupedPaths(GCode::Grouping::Copper);
    int32_t id{};
    auto condei = [&points, tolerance, &id](Point64 tmp, Point64 point) { // split long segments
        QLineF line{~tmp, ~point};
        if(line.length() > tolerance) {
            for(size_t i = 1, total = static_cast<int>(line.length() / tolerance); i < total; ++i) {
                line.setLength(i * tolerance);
                Point64 pt{~line.p2()};
                points.push_back({static_cast<jcv_real>(pt.x), static_cast<jcv_real>(pt.y), id});
            }
        }
    };

    for(const Paths64& paths: groupedPss) {
        for(const Geo::Polyline& path: paths) {
            Point64 tmp(path.front());
            for(const Point64& point: path) {
                condei(tmp, point);
                points.push_back({static_cast<jcv_real>(point.x), static_cast<jcv_real>(point.y), id});
                tmp = point;
            }
            condei(tmp, path.front());
        }
        ++id;
    }

    for(const Geo::Polyline& path: openSrcPaths) {
        Point64 tmp(path.front());
        for(const Point64& point: path) {
            condei(tmp, point);
            points.push_back({static_cast<jcv_real>(point.x), static_cast<jcv_real>(point.y), id});
            tmp = point;
        }
        condei(tmp, path.front());
        ++id;
    }

    cl::Clipper64 clipper;
    for(const Paths64& paths: groupedPss)
        clipper.AddClip(paths);
    clipper.AddClip(openSrcPaths);
    const Rect r(/*GetBounds(groupedPss) +*/ GetBounds(openSrcPaths)); // FIXME
    std::map<int, Pairs> edges;
    Pairs frame;
    {
        const /*PType*/ int32_t fo = gcp.params[FrameOffset].toDouble() * uScale;
        jcv_rect bounding_box = {
            {static_cast<jcv_real>(r.left - fo),  static_cast<jcv_real>(r.top - fo)   },
            {static_cast<jcv_real>(r.right + fo), static_cast<jcv_real>(r.bottom + fo)}
        };
        jcv_diagram diagram;
        jcv_diagragenerate_(points.size(), points.data(), &bounding_box, nullptr, &diagram);
        auto toPoint = [](const jcv_edge* edge, int num) -> const Point64 {
            return {static_cast</*PType*/ int32_t>(edge->pos[num].x), static_cast</*PType*/ int32_t>(edge->pos[num].y)};
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
    mergePaths(returnPs, 0.005 * uScale);
    returnPs.append_range(toPath(frame));
    for(size_t i{}; i < returnPs.size(); ++i) // remove verry short paths
        if(returnPs[i].size() < 4 && distTo(returnPs[i].front(), returnPs[i].back()) < tolerance * 0.5 * uScale)
            returnPs -= i--;
}

Paths64 VoronoiJc::toPath(const Pairs& pairs) {
    msg = QObject::tr("Merge Segments");

    mvector<Pair> pairsVec;
    pairsVec.reserve(pairs.size());
    for(auto&& pair: pairs)
        pairsVec.push_back(pair);

    r::sort(pairsVec, {}, [](const Pair& a) { return (a.first.y + a.second.y) / 2; });

    mvector<OrdPath> holder(pairsVec.size() * 2);
    QList<OrdPath*> merge;
    OrdPath* it = holder.data();
    for(auto&& [p1, p2, id]: pairsVec) {
        Q_UNUSED(id)
        OrdPath* ordPath1 = it++;
        OrdPath* ordPath2 = it++;
        ordPath1->Pt = p1;
        ordPath1->Next = ordPath2;
        ordPath1->Last = ordPath2;
        ordPath2->Pt = p2;
        ordPath2->Prev = ordPath1;
        merge.push_back(ordPath1);
    }

    const int max = merge.size();
    for(int i{}; i < merge.size(); ++i) {
        setMax(max);
        setCurrent(max - merge.size());
        throwIfCancel();
        for(int j{}; j < merge.size(); ++j) {
            if(i == j)
                continue;
            if(merge[i]->Last->Pt == merge[j]->Pt) {
                merge[i]->push_back(merge[j]->Next);
                merge.removeAt(j--);
                continue;
            } else if(merge[i]->Pt == merge[j]->Last->Pt) {
                merge[j]->push_back(merge[i]->Next);
                merge.removeAt(i--);
                break;
            }
        }
    }

    Paths64 paths;
    paths.reserve(merge.size());
    for(auto&& path: merge)
        paths.emplace_back(path->toPath());

    mergePaths(paths, 0.005 * uScale);

    auto clean = [this, kAngle = 2.0](Geo::Polyline& path) {
        for(size_t i = 1; i < path.size() - 2; ++i) {
            QLineF line{~path[i], ~path[i + 1]};
            if(line.length() < gcp.params[Tolerance].toDouble()) {
                path[i] = ~line.center();
                path -= i + 1;
                --i;
            }
        }
        for(size_t i = 1; i < path.size() - 1; ++i) {
            const double a1 = angleTo(path[i - 1], path[i]);
            const double a2 = angleTo(path[i], path[i + 1]);
            if(abs(a1 - a2) < kAngle)
                path -= i--;
        }
    };
    r::for_each(paths, clean);

    return paths;
}

} // namespace Voronoi
