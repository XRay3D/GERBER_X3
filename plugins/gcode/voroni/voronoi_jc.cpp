// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "voronoi_jc.h"

#undef _USE_CGAL_
#include "jc_voronoi.h"

namespace ClipperLib {
inline size_t qHash(const IntPoint& key, uint /*seed*/ = 0) { return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(IntPoint))); }

} // namespace ClipperLib

namespace GCode {

inline size_t qHash(const VoronoiJc::Pair& tag, uint = 0) { return ::qHash(tag.first.X ^ tag.second.X) ^ ::qHash(tag.first.Y ^ tag.second.Y); }

void VoronoiJc::jcVoronoi() {
    const auto tolerance = gcp_.params[GCodeParams::Tolerance].toDouble();

    mvector<jcv_point> points;
    points.reserve(100000);
    CleanPolygons(workingPs, tolerance * 0.1 * uScale);
    groupedPaths(CopperPaths);
    int id = 0;
    auto condei = [&points, tolerance, &id](IntPoint tmp, IntPoint point) { // split long segments
        QLineF line(tmp, point);
        if (line.length() > tolerance) {
            for (size_t i = 1, total = static_cast<int>(line.length() / tolerance); i < total; ++i) {
                line.setLength(i * tolerance);
                IntPoint pt((line.p2()));
                points.push_back({static_cast<jcv_real>(pt.X), static_cast<jcv_real>(pt.Y), id});
            }
        }
    };
    // PROG //PROG .3setProgMaxAndVal(7, 1); // progress
    for (const Paths& paths : groupedPss) {
        for (const Path& path : paths) {
            IntPoint tmp(path.front());
            for (const IntPoint& point : path) {
                condei(tmp, point);
                points.push_back({static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id});
                tmp = point;
            }
            condei(tmp, path.front());
        }
        ++id;
    }
    // PROG //PROG .3setProgMaxAndVal(7, 2); // progress
    for (const Path& path : workingRawPs) {
        IntPoint tmp(path.front());
        for (const IntPoint& point : path) {
            condei(tmp, point);
            points.push_back({static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id});
            tmp = point;
        }
        condei(tmp, path.front());
        ++id;
    }

    Clipper clipper;
    for (const Paths& paths : groupedPss) {
        clipper.AddPaths(paths, ptClip, true);
    }
    clipper.AddPaths(workingRawPs, ptClip, true);
    const IntRect r(clipper.GetBounds());
    std::map<int, Pairs> edges;
    Pairs frame;
    {
        const cInt fo = gcp_.params[GCodeParams::FrameOffset].toDouble() * uScale;
        jcv_rect bounding_box = {
            { static_cast<jcv_real>(r.left - fo),    static_cast<jcv_real>(r.top - fo)},
            {static_cast<jcv_real>(r.right + fo), static_cast<jcv_real>(r.bottom + fo)}
        };
        jcv_diagram diagram;
        jcv_diagragenerate_(points.size(), points.data(), &bounding_box, nullptr, &diagram);
        auto toIntPoint = [](const jcv_edge* edge, int num) -> const IntPoint {
            return {static_cast<cInt>(edge->pos[num].x), static_cast<cInt>(edge->pos[num].y)};
        };
        const jcv_site* sites = jcv_diagraget_sites_(&diagram);
        for (int i = 0; i < diagram.numsites; i++) {
            jcv_graphedge* graph_edge = sites[i].edges;
            while (graph_edge) {
                const jcv_edge* edge = graph_edge->edge;
                const Pair pair {toIntPoint(edge, 0), toIntPoint(edge, 1), sites[i].p.id};
                if (edge->sites[0] == nullptr || edge->sites[1] == nullptr)
                    frame.insert(pair); // frame
                else if (edge->sites[0]->p.id != edge->sites[1]->p.id)
                    edges[edge->sites[0]->p.id ^ edge->sites[1]->p.id].insert(pair); // other
                graph_edge = graph_edge->next;
            }
        }
        jcv_diagrafree_(&diagram);
    }

    for (const auto& [key, edge] : edges) {
        returnPs.append(toPath(edge));
        // PROG //PROG .3setProgMaxAndVal(edges.size(), returnPs_.size()); // progress
    }
    mergePaths(returnPs, 0.005 * uScale);
    returnPs.append(toPath(frame));
    for (size_t i = 0; i < returnPs.size(); ++i) { // remove verry short paths
        if (returnPs[i].size() < 4 && returnPs[i].front().distTo(returnPs[i].back()) < tolerance * 0.5 * uScale)
            returnPs.remove(i--);
    }
}

Paths VoronoiJc::toPath(const Pairs& pairs) {
    msg = tr("Merge Segments");

    mvector<Pair> pairsVec;
    pairsVec.reserve(pairs.size());
    for (auto&& pair : pairs)
        pairsVec.push_back(pair);

    std::ranges::sort(pairsVec, {}, [](const Pair& a) { return (a.first.Y + a.second.Y) / 2; });

    mvector<OrdPath> holder(pairsVec.size() * 2);
    QList<OrdPath*> merge;
    OrdPath* it = holder.data();
    for (auto&& [p1, p2, id] : pairsVec) {
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
    for (int i = 0; i < merge.size(); ++i) {
        setMax(max);
        setCurrent(max - merge.size());
        ifCancelThenThrow();
        for (int j = 0; j < merge.size(); ++j) {
            if (i == j)
                continue;
            if (merge[i]->Last->Pt == merge[j]->Pt) {
                merge[i]->push_back(merge[j]->Next);
                merge.removeAt(j--);
                continue;
            } else if (merge[i]->Pt == merge[j]->Last->Pt) {
                merge[j]->push_back(merge[i]->Next);
                merge.removeAt(i--);
                break;
            }
        }
    }

    Paths paths;
    paths.reserve(merge.size());
    for (auto&& path : merge)
        paths.emplace_back(path->toPath());

    mergePaths(paths, 0.005 * uScale);

    auto clean = [this, kAngle = 2.0](Path& path) {
        for (size_t i = 1; i < path.size() - 2; ++i) {
            QLineF line(path[i], path[i + 1]);
            if (line.length() < gcp_.params[GCodeParams::Tolerance].toDouble()) {
                path[i] = (line.center());
                path.remove(i + 1);
                --i;
            }
        }
        for (size_t i = 1; i < path.size() - 1; ++i) {
            const double a1 = path[i - 1].angleTo(path[i]);
            const double a2 = path[i].angleTo(path[i + 1]);
            if (abs(a1 - a2) < kAngle) {
                path.remove(i--);
            }
        }
    };
    std::ranges::for_each(paths, clean);

    return paths;
}

} // namespace GCode
