// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "voronoi_cgal.h"
#if __has_include(<CGAL/Algebraic_structure_traits_.h>)
#include <CGAL/Algebraic_structure_traits.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Segment_Delaunay_graph_2.h>
#include <CGAL/Segment_Delaunay_graph_filtered_traits_2.h>
#include <CGAL/Segment_Delaunay_graph_storage_traits_with_info_2.h>
#include <CGAL/Segment_Delaunay_graph_traits_2.h>
#include <ranges>

struct convert_info {
    using Info = int;
    using result_type = const Info&;
    inline const Info& operator()(const Info& info0, bool) const { return info0; }
    inline const Info& operator()(const Info& info0, const Info&, bool) const { return info0; }
};

struct merge_info {
    using Info = int;
    using result_type = Info;
    inline Info operator()(const Info& info0, const Info& info1) const {
        if (info0 == info1)
            return info0;
        return info0;
    }
};

typedef CGAL::Simple_cartesian<double> K;
// using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using GT = CGAL::Segment_Delaunay_graph_filtered_traits_2<K>;
using ST = CGAL::Segment_Delaunay_graph_storage_traits_with_info_2<GT, int, convert_info, merge_info>;
using DS = CGAL::Triangulation_data_structure_2<CGAL::Segment_Delaunay_graph_vertex_base_2<ST>, CGAL::Segment_Delaunay_graph_face_base_2<GT>>;
using SDG2 = CGAL::Segment_Delaunay_graph_2<GT, ST, DS>;

inline auto toPoint(const CGAL::Point_2<K>& point) { return Point {static_cast<Point::Type>(point.x()), static_cast<Point::Type>(point.y())}; }

///////////////////////////////////

#include "mvector.h"
#include <cstdio>

struct Segment {
    Point p0;
    Point p1;
    Segment(Point::Type x1, Point::Type y1, Point::Type x2, Point::Type y2)
        : p0(x1, y1)
        , p1(x2, y2) {
    }
    Segment(const Point& p0_, const Point& p1_)
        : p0(p0_)
        , p1(p1_) {
    }
};

namespace ClipperLib {
inline size_t qHash(const Point& key, uint /*seed*/ = 0) { return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(Point))); }

} // namespace ClipperLib

namespace GCode {

void VoronoiCgal::cgalVoronoi() {
    Point::Type minX = std::numeric_limits<Point::Type>::max(),
                minY = std::numeric_limits<Point::Type>::max(),
                maxX = std::numeric_limits<Point::Type>::min(),
                maxY = std::numeric_limits<Point::Type>::min();
    //    progress(4, 0);
    SDG2 sdg;
    int id = 0;
    // add line segments to diagram
    msg = tr("Calc CGAL Voronoi");

    size_t max {};
    for (const Paths& paths : groupedPss_)
        for (const Path& path : paths)
            max += path.size();

    setMax(max);
    setCurrent();

    for (const Paths& paths : groupedPss_) {
        for (const Path& path : paths) {
            for (size_t i = 0; i < path.size(); ++i) {
                incCurrent();
                getCancelThrow();
                const Point& point = path[i];
                const SDG2::Site_2 site = !i ? SDG2::Site_2::construct_site_2({path.back().x, path.back().y}, {point.x, point.y}) : SDG2::Site_2::construct_site_2({path[i - 1].x, path[i - 1].y}, {point.x, point.y});
                sdg.insert(site, id);
                maxX = std::max(maxX, point.x);
                maxY = std::max(maxY, point.y);
                minX = std::min(minX, point.x);
                minY = std::min(minY, point.y);
            }
        }
        ++id;
    }
    const Point::Type kx = (maxX - minX) * 2;
    const Point::Type ky = (maxY - minY) * 2;
    sdg.insert(SDG2::Site_2::construct_site_2({maxX + kx, minY - ky}, {maxX + kx, maxY + ky}), id);
    sdg.insert(SDG2::Site_2::construct_site_2({maxX + kx, minY - ky}, {minX - kx, minY - ky}), id);
    sdg.insert(SDG2::Site_2::construct_site_2({minX - kx, maxY + ky}, {maxX + kx, maxY + ky}), id);
    sdg.insert(SDG2::Site_2::construct_site_2({minX - kx, minY - ky}, {minX - kx, maxY + ky}), id);
    assert(sdg.is_valid(true, 1));
    Paths segments;
    segments.reserve(id);
    {
        std::map<int, Paths> pathPairs;
        for (auto eit = sdg.finite_edges_begin(); eit != sdg.finite_edges_end(); ++eit) {
            const SDG2::Edge e = *eit;
            CGAL_precondition(!sdg.is_infinite(e));
            if (e.first->vertex(sdg.cw(e.second))->storage_site().info() == e.first->vertex(sdg.ccw(e.second))->storage_site().info())
                continue;
            const int idIdx = e.first->vertex(sdg.cw(e.second))->storage_site().info() ^ e.first->vertex(sdg.ccw(e.second))->storage_site().info();
            CGAL::Object o = sdg.primal(e);
            /*  */ if (SDG2::Geotraits_::Line_2 sdgLine; CGAL::assign(sdgLine, o)) {
                pathPairs[idIdx].push_back(Path {toPoint(sdgLine.point(0)), toPoint(sdgLine.point(1))});
            } else if (SDG2::Geotraits_::Ray_2 sdgRay; CGAL::assign(sdgRay, o)) {
                pathPairs[idIdx].push_back(Path {toPoint(sdgRay.point(0)), toPoint(sdgRay.point(1))});
            } else if (SDG2::Geotraits_::Segment_2 sdgSegment; CGAL::assign(sdgSegment, o) && !sdgSegment.is_degenerate()) {
                pathPairs[idIdx].push_back(Path {toPoint(sdgSegment.point(0)), toPoint(sdgSegment.point(1))});
            } else if (CGAL::Parabola_segment_2<GT> cgalParabola; CGAL::assign(cgalParabola, o)) {
                mvector<SDG2::Point_2> points;
                cgalParabola.generate_points(points, 0.1 * uScale);
                Path path;
                path.reserve(static_cast<int>(points.size()));
                for (const SDG2::Point_2& pt : points)
                    path.push_back(toPoint(pt));
                pathPairs[idIdx].push_back(path);
            }
        }
        for (auto& [key, edge] : pathPairs) {
            mergePaths(edge);
            mergePaths(edge, 0.005 * uScale);
            segments.append(edge);
        }
    }
    const Point::Type fo = gcp_.params[GCodeParams::FrameOffset].toDouble() * uScale;
    Path frame {
        {minX - fo, minY - fo},
        {minX - fo, maxY + fo},
        {maxX + fo, maxY + fo},
        {maxX + fo, minY - fo},
        {minX - fo, minY - fo},
    };
    {
        Clipper clipper;
        clipper.AddPaths(segments, ptSubject, false);
        clipper.AddPath(frame, ptClip, true);
        clipper.Execute(ClipType::Intersection, segments, FillRule::NonZero);
    }

    std::ranges::sort(segments, {}, [](const Path& path) { return (path.front().y + path.back().y); });
    //    mergePaths(segments, 0.005 * uScale);
    mergeSegments(segments, 0.005 * uScale);

    auto clean = [kAngle = 2.0](Path& path) {
        for (size_t i = 1; i < path.size() - 1; ++i) {
            const double a1 = path[i - 1].angleTo(path[i + 0]);
            const double a2 = path[i + 0].angleTo(path[i + 1]);
            if (abs(a1 - a2) < kAngle) {
                path.remove(i--);
            }
        }
    };
    std::ranges::for_each(segments, clean);

    returnPs_ = segments;
    returnPs_.push_back(frame);
}

} // namespace GCode
#else
namespace GCode {
void VoronoiCgal::cgalVoronoi() { }

} // namespace GCode
#endif
