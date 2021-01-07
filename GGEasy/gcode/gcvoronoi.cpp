// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcvoronoi.h"
#include "gcfile.h"
#include "gcvoronoi.h"

#include "voroni/jc_voronoi.h"
#ifdef _USE_CGAL_
#include <CGAL/Algebraic_structure_traits.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Segment_Delaunay_graph_2.h>
#include <CGAL/Segment_Delaunay_graph_filtered_traits_2.h>
#include <CGAL/Segment_Delaunay_graph_storage_traits_with_info_2.h>
#include <CGAL/Segment_Delaunay_graph_traits_2.h>

#include "leakdetector.h"

struct convert_info {
    using Info = int;
    using result_type = const Info&;
    inline const Info& operator()(const Info& info0, bool) const { return info0; }
    inline const Info& operator()(const Info& info0, const Info&, bool) const { return info0; }
};

struct merge_info {
    using Info = int;
    using result_type = Info;
    inline Info operator()(const Info& info0, const Info& info1) const
    {
        if (info0 == info1)
            return info0;
        return info0;
    }
};

typedef CGAL::Simple_cartesian<double> K;
//using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using GT = CGAL::Segment_Delaunay_graph_filtered_traits_2<K>;
using ST = CGAL::Segment_Delaunay_graph_storage_traits_with_info_2<GT, int, convert_info, merge_info>;
using DS = CGAL::Triangulation_data_structure_2<CGAL::Segment_Delaunay_graph_vertex_base_2<ST>, CGAL::Segment_Delaunay_graph_face_base_2<GT>>;
using SDG2 = CGAL::Segment_Delaunay_graph_2<GT, ST, DS>;

inline IntPoint toIntPoint(const CGAL::Point_2<K>& point)
{
    return IntPoint { static_cast<cInt>(point.x()), static_cast<cInt>(point.y()) };
}

///////////////////////////////////

#include "mvector.h"
#include <cstdio>

#include <boost/polygon/voronoi.hpp>
using boost::polygon::high;
using boost::polygon::low;
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;

//#include "voronoi_visual_utils.hpp"

struct Segment {
    IntPoint p0;
    IntPoint p1;
    Segment(cInt x1, cInt y1, cInt x2, cInt y2)
        : p0(x1, y1)
        , p1(x2, y2)
    {
    }
    Segment(const IntPoint& p0_, const IntPoint& p1_)
        : p0(p0_)
        , p1(p1_)
    {
    }
};

namespace boost {
namespace polygon {

    template <>
    struct geometry_concept<IntPoint> {
        typedef point_concept type;
    };

    template <>
    struct point_traits<IntPoint> {
        typedef int coordinate_type;

        static inline coordinate_type get(
            const IntPoint& point, orientation_2d orient)
        {
            return (orient == HORIZONTAL) ? point.X : point.Y;
        }
    };

    template <>
    struct geometry_concept<Segment> {
        typedef segment_concept type;
    };

    template <>
    struct segment_traits<Segment> {
        typedef int coordinate_type;
        typedef IntPoint point_type;

        static inline point_type get(const Segment& segment, direction_1d dir)
        {
            return dir.to_int() ? segment.p1 : segment.p0;
        }
    };
} // polygon
} // boost

// Traversing Voronoi edges using edge iterator.
int iterate_primary_edges1(const voronoi_diagram<double>& vd)
{
    int result = 0;
    for (voronoi_diagram<double>::const_edge_iterator it = vd.edges().begin();
         it != vd.edges().end(); ++it) {
        if (it->is_primary())
            ++result;
    }
    return result;
}

// Traversing Voronoi edges using cell iterator.
int iterate_primary_edges2(const voronoi_diagram<double>& vd)
{
    int result = 0;
    for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin();
         it != vd.cells().end(); ++it) {
        const voronoi_diagram<double>::cell_type& cell = *it;
        const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
        // This is convenient way to iterate edges around Voronoi cell.
        do {
            if (edge->is_primary())
                ++result;
            edge = edge->next();
        } while (edge != cell.incident_edge());
    }
    return result;
}

// Traversing Voronoi edges using vertex iterator.
// As opposite to the above two functions this one will not iterate through
// edges without finite endpoints and will iterate only once through edges
// with single finite endpoint.
int iterate_primary_edges3(const voronoi_diagram<double>& vd)
{
    int result = 0;
    for (voronoi_diagram<double>::const_vertex_iterator it = vd.vertices().begin(); it != vd.vertices().end(); ++it) {
        const voronoi_diagram<double>::vertex_type& vertex = *it;
        const voronoi_diagram<double>::edge_type* edge = vertex.incident_edge();
        // This is convenient way to iterate edges around Voronoi vertex.
        do {
            if (edge->is_primary())
                ++result;
            edge = edge->rot_next();
        } while (edge != vertex.incident_edge());
    }
    return result;
}
#endif
inline uint qHash(const Point64& key, uint /*seed*/ = 0)
{
    return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(Point64)));
}

namespace GCode {

inline uint qHash(const GCode::VoronoiCreator::Pair& tag, uint = 0)
{
    return ::qHash(tag.first.X ^ tag.second.X) ^ ::qHash(tag.first.Y ^ tag.second.Y);
}

void VoronoiCreator::create()
{
    createVoronoi();
}

void VoronoiCreator::createVoronoi()
{
    const auto& tool = m_gcp.tools.first();
    const auto depth = m_gcp.params[GCodeParams::Depth].toDouble();
    const auto width = m_gcp.params[GCodeParams::Width].toDouble();

    groupedPaths(CopperPaths);
    switch (m_gcp.params[GCodeParams::VorT].toInt()) {
    case 0:
        jcVoronoi();
        break;
    case 1:
        cgalVoronoi();
        break;
    case 2:
        boostVoronoi();
        /**************/ return;
        break;
    }

    if (width < tool.getDiameter(depth)) {
        m_gcp.gcType = Voronoi;
        m_file = new File({ sortBE(m_returnPs) }, m_gcp);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    } else {
        Paths copy { m_returnPs };
        createOffset(tool, depth, width);
        m_gcp.gcType = Voronoi;
        { // создание пермычек.
            Clipper clipper;
            clipper.AddPaths(m_workingRawPs, ptClip, true);
            clipper.AddPaths(copy, ptSubject, false);
            clipper.Execute(ctDifference, copy, pftNonZero);
            sortBE(copy);
            for (auto&& p : copy)
                m_returnPss.push_back({ p });
        }
        { // создание заливки.
            ClipperOffset offset(uScale);
            offset.AddPaths(m_workingRawPs, jtRound, etClosedPolygon);
            offset.AddPaths(copy, jtRound, etOpenRound);
            offset.Execute(m_workingRawPs, m_dOffset + 10);
        }
        m_file = new File(m_returnPss, m_gcp, m_workingRawPs);
        m_file->setFileName(tool.nameEnc());
        emit fileReady(m_file);
    }
}

void VoronoiCreator::createOffset(const Tool& tool, double depth, const double width)
{
    msg = tr("Create Offset");
    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover() * uScale;
    const Path frame(m_returnPs.takeLast());
    { // create offset
        ClipperOffset offset;
        offset.AddPaths(m_returnPs, jtRound /*jtMiter*/, etOpenRound);
        offset.Execute(m_returnPs, width * uScale * 0.5);
    }
    { // fit offset to copper
        Clipper clipper;
        clipper.AddPaths(m_returnPs, ptSubject, true);
        for (const Paths& paths : m_groupedPss)
            clipper.AddPaths(paths, ptClip, true);
        clipper.Execute(ctDifference, m_returnPs, pftPositive, pftNegative);
    }
    { // cut to copper rect
        Clipper clipper;
        clipper.AddPaths(m_returnPs, ptSubject, true);
        clipper.AddPath(frame, ptClip, true);
        clipper.Execute(ctIntersection, m_returnPs, pftNonZero);
        CleanPolygons(m_returnPs, 0.001 * uScale);
    }
    { // create pocket
        ClipperOffset offset(uScale);
        offset.AddPaths(m_returnPs, jtRound, etClosedPolygon);
        Paths tmpPaths1;
        offset.Execute(tmpPaths1, -m_dOffset);
        m_workingRawPs = tmpPaths1;
        Paths tmpPaths;
        do {
            tmpPaths.push_back(tmpPaths1);
            offset.Clear();
            offset.AddPaths(tmpPaths1, jtMiter, etClosedPolygon);
            offset.Execute(tmpPaths1, -m_stepOver);
        } while (tmpPaths1.size());
        m_returnPs = tmpPaths;
    }
    if (m_returnPs.isEmpty()) {
        emit fileReady(nullptr);
        return;
    }
    stacking(m_returnPs);
    m_returnPss.push_back({ frame });
}

void VoronoiCreator::mergePaths(Paths& paths, const double dist)
{
    msg = tr("Merge Paths");
    sortBE(paths);
    size_t max;
    do {
        max = paths.size();
        for (size_t i = 0; i < paths.size(); ++i) {
            setMax(max);
            setCurrent(max - paths.size());
            getCancelThrow();

            for (size_t j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                else if (paths[i].first() == paths[j].first()) {
                    ReversePath(paths[j]);
                    paths[j].push_back(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                } else if (paths[i].back() == paths[j].back()) {
                    ReversePath(paths[j]);
                    paths[i].push_back(paths[j].mid(1));
                    paths.remove(j--);
                    break;
                } else if (paths[i].first() == paths[j].back()) {
                    paths[j].push_back(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                } else if (paths[j].first() == paths[i].back()) {
                    paths[i].push_back(paths[j].mid(1));
                    paths.remove(j--);
                    break;
                } else if (dist != 0.0) {
                    /*  */ if (Length(paths[i].back(), paths[j].back()) < dist) {
                        ReversePath(paths[j]);
                        paths[i].push_back(paths[j].mid(1));
                        paths.remove(j--);
                        break; //
                    } else if (Length(paths[i].back(), paths[j].first()) < dist) {
                        paths[i].push_back(paths[j].mid(1));
                        paths.remove(j--);
                        break; //
                    } else if (Length(paths[i].first(), paths[j].back()) < dist) {
                        paths[j].push_back(paths[i].mid(1));
                        paths.remove(i--);
                        break;
                    } else if (Length(paths[i].first(), paths[j].first()) < dist) {
                        ReversePath(paths[j]);
                        paths[j].push_back(paths[i].mid(1));
                        paths.remove(i--);
                        break;
                    }
                }
            }
        }
    } while (max != paths.size());
}

void VoronoiCreator::clean(Path& path)
{
    for (size_t i = 1; i < path.size() - 2; ++i) {
        QLineF line(path[i], path[i + 1]);
        if (line.length() < m_gcp.params[GCodeParams::Tolerance].toDouble()) {
            path[i] = (line.center());
            path.remove(i + 1);
            --i;
        }
    }
    const double kAngle = 1; //0.2;
    for (size_t i = 1; i < path.size() - 1; ++i) {
        const double a1 = Angle(path[i - 1], path[i]);
        const double a2 = Angle(path[i], path[i + 1]);
        if (abs(a1 - a2) < kAngle) {
            path.remove(i--);
        }
    }
}

void VoronoiCreator::cgalVoronoi()
{
#ifdef _USE_CGAL_
    cInt minX = std::numeric_limits<cInt>::max(),
         minY = std::numeric_limits<cInt>::max(),
         maxX = std::numeric_limits<cInt>::min(),
         maxY = std::numeric_limits<cInt>::min();
    progress(4, 0);
    SDG2 sdg;
    int id = 0;
    //add line segments to diagram
    int c = 0;
    msg = tr("Calc Voronoi");
    for (const Paths& paths : m_groupedPss) {
        progress(m_groupedPss.size(), ++c);
        for (const Path& path : paths) {
            for (size_t i = 0; i < path.size(); ++i) {
                const IntPoint& point = path[i];
                const SDG2::Site_2 site = !i ? SDG2::Site_2::construct_site_2({ path.last().X, path.last().Y }, { point.X, point.Y })
                                             : SDG2::Site_2::construct_site_2({ path[i - 1].X, path[i - 1].Y }, { point.X, point.Y });
                sdg.insert(site, id);
                maxX = std::max(maxX, point.X);
                maxY = std::max(maxY, point.Y);
                minX = std::min(minX, point.X);
                minY = std::min(minY, point.Y);
            }
        }
        ++id;
    }
    const cInt kx = (maxX - minX) * 2;
    const cInt ky = (maxY - minY) * 2;
    sdg.insert(SDG2::Site_2::construct_site_2({ maxX + kx, minY - ky }, { maxX + kx, maxY + ky }), id);
    sdg.insert(SDG2::Site_2::construct_site_2({ maxX + kx, minY - ky }, { minX - kx, minY - ky }), id);
    sdg.insert(SDG2::Site_2::construct_site_2({ minX - kx, maxY + ky }, { maxX + kx, maxY + ky }), id);
    sdg.insert(SDG2::Site_2::construct_site_2({ minX - kx, minY - ky }, { minX - kx, maxY + ky }), id);
    assert(sdg.is_valid(true, 1));
    Paths segments;
    segments.reserve(id);
    {
        QMap<int, Paths> pathPairs;
        for (SDG2::Finite_edges_iterator eit = sdg.finite_edges_begin(); eit != sdg.finite_edges_end(); ++eit) {
            const SDG2::Edge e = *eit;
            CGAL_precondition(!sdg.is_infinite(e));
            if (e.first->vertex(sdg.cw(e.second))->storage_site().info() == e.first->vertex(sdg.ccw(e.second))->storage_site().info())
                continue;
            const int idIdx = e.first->vertex(sdg.cw(e.second))->storage_site().info() ^ e.first->vertex(sdg.ccw(e.second))->storage_site().info();
            CGAL::Object o = sdg.primal(e);
            /*  */ if (SDG2::Geom_traits::Line_2 sdgLine; CGAL::assign(sdgLine, o)) {
                pathPairs[idIdx].push_back(Path { toIntPoint(sdgLine.point(0)), toIntPoint(sdgLine.point(1)) });
            } else if (SDG2::Geom_traits::Ray_2 sdgRay; CGAL::assign(sdgRay, o)) {
                pathPairs[idIdx].push_back(Path { toIntPoint(sdgRay.point(0)), toIntPoint(sdgRay.point(1)) });
            } else if (SDG2::Geom_traits::Segment_2 sdgSegment; CGAL::assign(sdgSegment, o) && !sdgSegment.is_degenerate()) {
                pathPairs[idIdx].push_back(Path { toIntPoint(sdgSegment.point(0)), toIntPoint(sdgSegment.point(1)) });
            } else if (CGAL::Parabola_segment_2<GT> cgalParabola; CGAL::assign(cgalParabola, o)) {
                mvector<SDG2::Point_2> points;
                cgalParabola.generate_points(points, 5);
                Path path;
                path.reserve(static_cast<int>(points.size()));
                for (const SDG2::Point_2& pt : points)
                    path.push_back(toIntPoint(pt));
                pathPairs[idIdx].push_back(path);
            }
        }
        for (Paths& edge : pathPairs) {
            mergePaths(edge);
            mergePaths(edge, 0.005 * uScale);
            segments.push_back(edge);
        }
    }
    const cInt fo = m_gcp.params[GCodeParams::FrameOffset].toDouble() * uScale;
    Path frame {
        { minX - fo, minY - fo },
        { minX - fo, maxY + fo },
        { maxX + fo, maxY + fo },
        { maxX + fo, minY - fo },
        { minX - fo, minY - fo },
    };
    {
        Clipper clipper;
        clipper.AddPaths(segments, ptSubject, false);
        clipper.AddPath(frame, ptClip, true);
        clipper.Execute(ctIntersection, segments, pftNonZero);
    }
    mergePaths(segments, 0.02 * uScale);
    m_returnPs = segments;
    m_returnPs.push_back(frame);
#endif
}

void VoronoiCreator::jcVoronoi()
{
    const auto tolerance = m_gcp.params[GCodeParams::Tolerance].toDouble();

    mvector<jcv_point> points;
    points.reserve(100000);
    CleanPolygons(m_workingPs, tolerance * 0.1 * uScale);
    groupedPaths(CopperPaths);
    int id = 0;
    auto condei = [&points, tolerance, &id](Point64 tmp, Point64 point) { // split long segments
        QLineF line(tmp, point);
        if (line.length() > tolerance) {
            for (size_t i = 1, total = static_cast<int>(line.length() / tolerance); i < total; ++i) {
                line.setLength(i * tolerance);
                Point64 pt((line.p2()));
                points.push_back({ static_cast<jcv_real>(pt.X), static_cast<jcv_real>(pt.Y), id });
            }
        }
    };
    //PROG //PROG .3setProgMaxAndVal(7, 1); // progress
    for (const Paths& paths : m_groupedPss) {
        for (const Path& path : paths) {
            Point64 tmp(path.first());
            for (const Point64& point : path) {
                condei(tmp, point);
                points.push_back({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                tmp = point;
            }
            condei(tmp, path.first());
        }
        ++id;
    }
    //PROG //PROG .3setProgMaxAndVal(7, 2); // progress
    for (const Path& path : m_workingRawPs) {
        Point64 tmp(path.first());
        for (const Point64& point : path) {
            condei(tmp, point);
            points.push_back({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
            tmp = point;
        }
        condei(tmp, path.first());
        ++id;
    }

    Clipper clipper;
    for (const Paths& paths : m_groupedPss) {
        clipper.AddPaths(paths, ptClip, true);
    }
    clipper.AddPaths(m_workingRawPs, ptClip, true);
    const IntRect r(clipper.GetBounds());
    QMap<int, Pairs> edges;
    Pairs frame;
    {
        const cInt fo = m_gcp.params[GCodeParams::FrameOffset].toDouble() * uScale;
        jcv_rect bounding_box = {
            { static_cast<jcv_real>(r.left - fo), static_cast<jcv_real>(r.top - fo) },
            { static_cast<jcv_real>(r.right + fo), static_cast<jcv_real>(r.bottom + fo) }
        };
        jcv_diagram diagram;
        jcv_diagram_generate(points.size(), points.data(), &bounding_box, &diagram);
        auto toIntPoint = [](const jcv_edge* edge, int num) -> const Point64 {
            return { static_cast<cInt>(edge->pos[num].x), static_cast<cInt>(edge->pos[num].y) };
        };
        const jcv_site* sites = jcv_diagram_get_sites(&diagram);
        for (int i = 0; i < diagram.numsites; i++) {
            jcv_graphedge* graph_edge = sites[i].edges;
            while (graph_edge) {
                const jcv_edge* edge = graph_edge->edge;
                const Pair pair { toIntPoint(edge, 0), toIntPoint(edge, 1), sites[i].p.id };
                if (edge->sites[0] == nullptr || edge->sites[1] == nullptr)
                    frame.insert(pair); // frame
                else if (edge->sites[0]->p.id != edge->sites[1]->p.id)
                    edges[edge->sites[0]->p.id ^ edge->sites[1]->p.id].insert(pair); // other
                graph_edge = graph_edge->next;
            }
        }
        jcv_diagram_free(&diagram);
    }

    for (const Pairs& edge : edges) {
        m_returnPs.push_back(toPath(edge));
        //PROG //PROG .3setProgMaxAndVal(edges.size(), m_returnPs.size()); // progress
    }
    mergePaths(m_returnPs, 0.005 * uScale);
    m_returnPs.push_back(toPath(frame));
    for (size_t i = 0; i < m_returnPs.size(); ++i) { // remove verry short paths
        if (m_returnPs[i].size() < 4 && Length(m_returnPs[i].first(), m_returnPs[i].back()) < tolerance * 0.5 * uScale)
            m_returnPs.remove(i--);
    }
}

void VoronoiCreator::boostVoronoi()
{
#ifdef _USE_CGAL_
    // Preparing Input Geometries.
    mvector<IntPoint> points;
    mvector<Segment> segments;
    segments.push_back(Segment(-4, 5, 5, -1));
    segments.push_back(Segment(3, -11, 13, -1));

    cInt minX = std::numeric_limits<cInt>::max(),
         minY = std::numeric_limits<cInt>::max(),
         maxX = std::numeric_limits<cInt>::min(),
         maxY = std::numeric_limits<cInt>::min();
    progress(4, 0);
    int id = 0;
    //add line segments to diagram
    int c = 0;
    msg = tr("Calc Voronoi");
    for (const Paths& paths : m_groupedPss) {
        progress(m_groupedPss.size(), ++c);
        for (const Path& path : paths) {
            for (size_t i = 0; i < path.size(); ++i) {
                const IntPoint& point = path[i];
                !i ? segments.push_back(Segment { path.last(), point })
                   : segments.push_back(Segment { path[i - 1], point });
                maxX = std::max(maxX, point.X);
                maxY = std::max(maxY, point.Y);
                minX = std::min(minX, point.X);
                minY = std::min(minY, point.Y);
            }
        }
        ++id;
    }
    const cInt kx = (maxX - minX) * 2;
    const cInt ky = (maxY - minY) * 2;

    segments.push_back(Segment({ maxX + kx, minY - ky }, { maxX + kx, maxY + ky }));
    segments.push_back(Segment({ maxX + kx, minY - ky }, { minX - kx, minY - ky }));
    segments.push_back(Segment({ minX - kx, maxY + ky }, { maxX + kx, maxY + ky }));
    segments.push_back(Segment({ minX - kx, minY - ky }, { minX - kx, maxY + ky }));

    Paths segments_;

    segments_.reserve(id);

    // Construction of the Voronoi Diagram.
    voronoi_diagram<double> vd;
    construct_voronoi(points.begin(), points.end(), segments.begin(), segments.end(), &vd);

    // Traversing Voronoi Graph.
    {
        printf("Traversing Voronoi graph.\n");
        printf("Number of visited primary edges using edge iterator: %d\n", iterate_primary_edges1(vd));
        printf("Number of visited primary edges using cell iterator: %d\n", iterate_primary_edges2(vd));
        printf("Number of visited primary edges using vertex iterator: %d\n", iterate_primary_edges3(vd));
        printf("\n");
    }

    // Using color member of the Voronoi primitives to store the average number
    // of edges around each cell (including secondary edges).
    {
        printf("Number of edges (including secondary) around the Voronoi cells:\n");
        for (voronoi_diagram<double>::const_edge_iterator it = vd.edges().begin();
             it != vd.edges().end(); ++it) {
            std::size_t cnt = it->cell()->color();
            it->cell()->color(cnt + 1);
        }
        for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin();
             it != vd.cells().end(); ++it) {
            printf("%lu ", it->color());
        }
        printf("\n");
        printf("\n");
    }

    // Linking Voronoi cells with input geometries.
    QMap<std::size_t, QPair<IntPoint, IntPoint>> m;
    {
        unsigned int cell_index = 0;
        for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin();
             it != vd.cells().end(); ++it) {
            if (it->contains_point()) {
                if (it->source_category() == boost::polygon::SOURCE_CATEGORY_SINGLE_POINT) {
                    std::size_t index = it->source_index();
                    IntPoint p = points[index];
                    printf("Cell #%u contains a point: (%d, %d).\n", cell_index, x(p), y(p));
                } else if (it->source_category() == boost::polygon::SOURCE_CATEGORY_SEGMENT_START_POINT) {
                    std::size_t index = it->source_index() - points.size();
                    IntPoint p0 = low(segments[index]);
                    printf("Cell #%u contains segment start point: (%d, %d).\n", cell_index, x(p0), y(p0));
                    m[index].first = p0;
                } else if (it->source_category() == boost::polygon::SOURCE_CATEGORY_SEGMENT_END_POINT) {
                    std::size_t index = it->source_index() - points.size();
                    IntPoint p1 = high(segments[index]);
                    printf("Cell #%u contains segment end point: (%d, %d).\n", cell_index, x(p1), y(p1));
                    m[index].first = p1;
                }
            } else {
                std::size_t index = it->source_index() - points.size();
                IntPoint p0 = low(segments[index]);
                IntPoint p1 = high(segments[index]);
                printf("Cell #%u contains a segment: ((%d, %d), (%d, %d)). \n", cell_index, x(p0), y(p0), x(p1), y(p1));
            }
            ++cell_index;
        }
    }

    for (auto& pair : m) {
        segments_.push_back({ pair.first, pair.second });
    }

    m_gcp.gcType = Voronoi;
    m_file = new File({ sortBE(segments_) }, m_gcp);
    m_file->setFileName(Tool {}.nameEnc());
    emit fileReady(m_file);

#endif
}

Paths VoronoiCreator::toPath(const Pairs& pairs)
{
    msg = tr("Merge Segments");
    Paths paths;
    Pairs tmp = pairs;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QList<Pair> tmp2(tmp.toList());
#else
    QList<Pair> tmp2(tmp.values());
#endif

    std::sort(tmp2.begin(), tmp2.end(), [](const Pair& a, const Pair& b) { return a.id > b.id; });
    {
        QList<OrdPath*> merge;
        mvector<OrdPath> holder(tmp2.size() * 2);
        OrdPath* it = holder.data();
        for (auto [p1, p2, _skip] : tmp2) {
            Q_UNUSED(_skip)
            OrdPath* pt1 = it++;
            OrdPath* pt2 = it++;
            pt1->Pt = p1;
            pt1->Next = pt2;
            pt1->Last = pt2;
            pt2->Pt = p2;
            pt2->Prev = pt1;
            merge.push_back(pt1);
        }

        const int max = merge.size();
        for (int i = 0; i < merge.size(); ++i) {
            setMax(max);
            setCurrent(max - merge.size());
            getCancelThrow();
            for (int j = 0; j < merge.size(); ++j) {
                if (i == j)
                    continue;
                if (merge[i]->Last->Pt == merge[j]->Pt) {
                    merge[i]->push_back(merge[j]->Next);
                    merge.removeAt(j--);
                    continue;
                }
                if (merge[i]->Pt == merge[j]->Last->Pt) {
                    merge[j]->push_back(merge[i]->Next);
                    merge.removeAt(i--);
                    break;
                }
            }
        }

        paths.resize(merge.size());
        for (int i = 0; i < merge.size(); ++i) {
            paths[i] = merge[i]->toPath();
        }
    }

    mergePaths(paths, 0.005 * uScale);
    for (Path& path : paths)
        clean(path);

    return paths;
}
}
