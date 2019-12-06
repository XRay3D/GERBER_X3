#include "gcvoronoi.h"
#include "gcfile.h"
#include "gcvoronoi.h"

#ifdef JCV
#include "voroni/jc_voronoi.h"
#endif

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Segment_Delaunay_graph_2.h>
#include <CGAL/Segment_Delaunay_graph_filtered_traits_2.h>
#include <CGAL/Segment_Delaunay_graph_storage_traits_with_info_2.h>

// an enum representing the color
enum /*Red_blue*/ {
    RED = 1,
    BLUE = 2,
    PURPLE = 3
};

using Red_blue = int;

// functor that defines how to convert color info when:
// 1. constructing the storage site of an endpoint of a segment
// 2. a segment site is split into two sub-segments
struct Red_blue_convert_info {
    using Info = Red_blue;
    using result_type = const Info&;

    inline const Info& operator()(const Info& info0, bool) const
    {
        // just return the info of the supporting segment
        return info0;
    }

    inline const Info& operator()(const Info& info0, const Info&, bool) const
    {
        // just return the info of the supporting segment
        return info0;
    }
};
// functor that defines how to merge color info when a site (either
// point or segment) corresponds to point(s) on plane belonging to
// more than one input site
struct Red_blue_merge_info {
    using Info = Red_blue;
    using result_type = Info;

    inline Info operator()(const Info& info0, const Info& info1) const
    {
        // if the two sites defining the new site have the same info, keep this common info
        if (info0 == info1) {
            return info0;
        }
        // otherwise the new site should be purple
        return PURPLE;
    }
};

using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Gt = CGAL::Segment_Delaunay_graph_filtered_traits_2<K>;
// define the storage traits with info
using ST = CGAL::Segment_Delaunay_graph_storage_traits_with_info_2<Gt, Red_blue, Red_blue_convert_info, Red_blue_merge_info>;
using SDG2 = CGAL::Segment_Delaunay_graph_2<Gt, ST>;

//// includes for defining the Voronoi diagram adaptor
//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Segment_2.h>
//#include <CGAL/Segment_Delaunay_graph_2.h>
//#include <CGAL/Segment_Delaunay_graph_adaptation_policies_2.h>
//#include <CGAL/Segment_Delaunay_graph_adaptation_traits_2.h>
//#include <CGAL/Segment_Delaunay_graph_storage_traits_with_info_2.h>
//#include <CGAL/Segment_Delaunay_graph_traits_2.h>
//#include <CGAL/Voronoi_diagram_2.h>

//// an enum representing the color
//enum Red_blue {
//    RED = 1,
//    BLUE = 2,
//    PURPLE = 3
//};

//// functor that defines how to convert color info when:
//// 1. constructing the storage site of an endpoint of a segment
//// 2. a segment site is split into two sub-segments
//struct Red_blue_convert_info {
//    typedef Red_blue Info;
//    typedef const Info& result_type;

//    inline const Info& operator()(const Info& info0, bool) const
//    {
//        // just return the info of the supporting segment
//        return info0;
//    }

//    inline const Info& operator()(const Info& info0, const Info&, bool) const
//    {
//        // just return the info of the supporting segment
//        return info0;
//    }
//};

//// functor that defines how to merge color info when a site (either
//// point or segment) corresponds to point(s) on plane belonging to
//// more than one input site
//struct Red_blue_merge_info {
//    typedef Red_blue Info;
//    typedef Info result_type;

//    inline Info operator()(const Info& info0, const Info& info1) const
//    {
//        // if the two sites defining the new site have the same info, keep
//        // this common info
//        if (info0 == info1) {
//            return info0;
//        }
//        // otherwise the new site should be purple
//        return PURPLE;
//    }
//};

//using K = CGAL::Exact_predicates_inexact_constructions_kernel;
//using Gt = CGAL::Segment_Delaunay_graph_traits_2<K>;

//using ST = CGAL::Segment_Delaunay_graph_storage_traits_with_info_2<Gt,
//    Red_blue,
//    Red_blue_convert_info,
//    Red_blue_merge_info>;

//using DT = CGAL::Segment_Delaunay_graph_2<Gt, ST>;
//using AT = CGAL::Segment_Delaunay_graph_adaptation_traits_2<DT>;
//using AP = CGAL::Segment_Delaunay_graph_degeneracy_removal_policy_2<DT>;
//using VD = CGAL::Voronoi_diagram_2<DT, AT, AP>;
//using Site_2 = AT::Site_2;
//using Face_handle = VD::Face_handle;

//inline bool qMapLessThanKey(const IntPoint& key1, const IntPoint& key2)
//{
//    return qHash(QByteArray(reinterpret_cast<const char*>(&key1), sizeof(IntPoint))) < qHash(QByteArray(reinterpret_cast<const char*>(&key2), sizeof(IntPoint)));
//}
//inline bool qMapLessThanKey(const Site_2& key1, const Site_2& key2)
//{
//    return qHash(QByteArray(reinterpret_cast<const char*>(&key1), sizeof(Site_2))) < qHash(QByteArray(reinterpret_cast<const char*>(&key2), sizeof(Site_2)));
//}

namespace GCode {

#ifdef JCV
inline uint qHash(const GCode::VoronoiCreator::Pair& tag, uint seed = 0)
{
    return ::qHash(tag.first.X * tag.second.X, seed ^ 0xa317a317) ^ ::qHash(tag.first.Y * tag.second.Y, seed ^ 0x17a317a3);
}
#endif

void VoronoiCreator::create(const GCodeParams& gcp)
{
    m_gcp = gcp;
    createVoronoi(m_gcp.tool.first(),
        m_gcp.dParam[Depth],
        m_gcp.dParam[Tolerance],
        m_gcp.dParam[Width]);
}

void VoronoiCreator::createVoronoi(const Tool& tool, double depth, const double tolerance, const double width)
{
    self = this;

    CleanPolygons(m_workingPs, tolerance * 0.1 * uScale);
    groupedPaths(CopperPaths);

    //    VD vd;

    //    int id = 0;

    //    cInt minX = std::numeric_limits<cInt>::max(), minY = minX, maxX = std::numeric_limits<cInt>::min(), maxY = maxX;

    //    //    QMap<IntPoint, int> map;
    //    QMap<Site_2, int> map;
    //    for (const Paths& paths : m_groupedPss) {
    //        for (const Path& path : paths) {
    //            for (int i = 0; i < path.size(); ++i) {
    //                const IntPoint& point = path[i];
    //                //                map[point] = id;

    //                Site_2 site;
    //                site = !i ? CGAL::Segment_Delaunay_graph_site_2<K>::construct_site_2({ path.last().X, path.last().Y }, { point.X, point.Y }, id)
    //                          : CGAL::Segment_Delaunay_graph_site_2<K>::construct_site_2({ path[i - 1].X, path[i - 1].Y }, { point.X, point.Y }, id);
    //                vd.insert(site);
    //                map[site] = id;
    //                maxX = std::max(maxX, point.X);
    //                maxY = std::max(maxY, point.Y);
    //                minX = std::min(minX, point.X);
    //                minY = std::min(minY, point.Y);
    //            }
    //            ++id;
    //        }
    //    }
    //    Paths line;
    //    assert(vd.is_valid());
    //    qDebug() << vd.number_of_faces();
    //    VD::Face_iterator it = vd.faces_begin(), beyond = vd.faces_end();
    //    for (int f = 0; it != beyond; ++f, ++it) {
    //        qDebug() << "Face" << f << ":";
    //        VD::Ccb_halfedge_circulator hec = it->ccb();
    //        Path path;
    //        do {
    //            VD::Halfedge_handle heh = static_cast<VD::Halfedge_handle>(hec);
    //            if (heh->has_target()) {
    //                if (!heh->has_source() || !heh->has_target()) {
    //                    qDebug("continue 1");
    //                    continue;
    //                }
    //                //                if (map[heh->left()->face()->vertex()] == map[heh->right()->site()]) {
    //                //                    qDebug() << "continue 2" << map[heh->left()->site()] << map[heh->right()->site()];
    //                //                    continue;
    //                //                }
    //                // qDebug() <<heh->left()->face();
    //                path.append(IntPoint {
    //                    static_cast<cInt>(heh->target()->point().x()),
    //                    static_cast<cInt>(heh->target()->point().y()) });
    //            } else {
    //                qDebug("point at infinity");
    //            }
    //        } while (++hec != it->ccb());
    //        line.append(path);
    //    }

    SDG2 sdg;

    cInt minX = std::numeric_limits<cInt>::max(), minY = minX, maxX = std::numeric_limits<cInt>::min(), maxY = maxX;
    //add line segments to diagram
    int id = 0;
    for (const Paths& paths : m_groupedPss) {
        for (const Path& path : paths) {
            for (int i = 0; i < path.size(); ++i) {
                const IntPoint& point = path[i];
                SDG2::Site_2 site;
                site = !i ? CGAL::Segment_Delaunay_graph_site_2<K>::construct_site_2({ path.last().X, path.last().Y }, { point.X, point.Y }, id)
                          : CGAL::Segment_Delaunay_graph_site_2<K>::construct_site_2({ path[i - 1].X, path[i - 1].Y }, { point.X, point.Y }, id);
                sdg.insert(site, id);
                maxX = std::max(maxX, point.X);
                maxY = std::max(maxY, point.Y);
                minX = std::min(minX, point.X);
                minY = std::min(minY, point.Y);
            }
            ++id;
        }
    }
    assert(sdg.is_valid(true, 1));
    //    std::vector<SDG2::Site_2> sites;
    //    sdg.insert(sites.begin(), sites.end(), CGAL::Tag_true());

    //save voronoi edges to a file
    //    sdg.draw_dual(ofs);

    Paths line;
    Paths segment;
    Paths ray;
    Paths parabolaSegment;

    //    using FVIT = SDG2::Finite_vertices_iterator;
    //    for (FVIT it = sdg.finite_vertices_begin(); it != sdg.finite_vertices_end(); ++it) {
    //        SDG2::Site_2 site = it->site();
    //        line std::cout << "\t" << it->storage_site().info();
    //    }

    SDG2::Finite_edges_iterator eit = sdg.finite_edges_begin();
    for (; eit != sdg.finite_edges_end(); ++eit) {
        const SDG2::Edge e = *eit;
        CGAL_precondition(!sdg.is_infinite(e));
        typename SDG2::Geom_traits::Line_2 l;
        typename SDG2::Geom_traits::Segment_2 s;
        typename SDG2::Geom_traits::Ray_2 r;

        using LocalGt = CGAL::Segment_Delaunay_graph_filtered_traits_2<K>;
        CGAL::Parabola_segment_2<LocalGt> ps;

        CGAL::Object o = sdg.primal(e);

        if (CGAL::assign(l, o)) {
            //qDebug("Line");
            line.append({
                IntPoint { static_cast<cInt>(l.point(0).x()), static_cast<cInt>(l.point(0).y()) },
                IntPoint { static_cast<cInt>(l.point(1).x()), static_cast<cInt>(l.point(1).y()) },
            });
        }
        if (CGAL::assign(s, o) && !s.is_degenerate()) {
            //qDebug("Segment");
            segment.append({
                IntPoint { static_cast<cInt>(s.point(0).x()), static_cast<cInt>(s.point(0).y()) },
                IntPoint { static_cast<cInt>(s.point(1).x()), static_cast<cInt>(s.point(1).y()) },
            });
        }
        if (CGAL::assign(r, o)) {
            //qDebug("Ray");
            ray.append({
                IntPoint { static_cast<cInt>(r.point(0).x()), static_cast<cInt>(r.point(0).y()) },
                IntPoint { static_cast<cInt>(r.point(1).x()), static_cast<cInt>(r.point(1).y()) },
            });
        }
        if (CGAL::assign(ps, o)) {
            //qDebug("Parabola_segment");
            std::vector<CGAL::Point_2<K>> points;
            ps.generate_points(points);
            qDebug() << "points" << points.size();
            Path path;
            path.reserve(static_cast<int>(points.size()));
            for (const CGAL::Point_2<K>& pt : points) {
                path << IntPoint { static_cast<cInt>(pt.x()), static_cast<cInt>(pt.y()) };
            }
            clean(path);
            parabolaSegment.append(path);
        }
    }

    auto generateFile = [this, tool, depth](const QString& name, Paths& paths) {
        m_file = new File({ paths }, tool, depth, Voronoi, m_workingRawPs);
        m_file->setFileName(name);
        emit fileReady(m_file);
    };

    generateFile("line", line);
    generateFile("segment", segment);
    //    generateFile("ray", ray);
    generateFile("parabolaSegment", parabolaSegment);

#ifdef JCV
    QVector<jcv_point> points;
    points.reserve(100000);
    CleanPolygons(m_workingPs, tolerance * 0.1 * uScale);
    groupedPaths(CopperPaths);
    int id = 0;
    auto condei = [&points, tolerance, &id](IntPoint tmp, IntPoint point) { // split long segments
        QLineF line(toQPointF(tmp), toQPointF(point));
        if (line.length() > tolerance) {
            for (int i = 1, total = static_cast<int>(line.length() / tolerance); i < total; ++i) {
                line.setLength(i * tolerance);
                IntPoint point(toIntPoint(line.p2()));
                points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
            }
        }
    };
    progress(7, 1); // progress
    for (const Paths& paths : m_groupedPss) {
        for (const Path& path : paths) {

            IntPoint tmp(path.first());
            for (const IntPoint& point : path) {
                condei(tmp, point);
                points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                tmp = point;
            }
            condei(tmp, path.first());
        }
        ++id;
    }
    progress(7, 2); // progress
    for (const Path& path : m_workingRawPs) {

        IntPoint tmp(path.first());
        for (const IntPoint& point : path) {
            condei(tmp, point);
            points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
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
    //progressOrCancel(7, 3); // progress
    {
        jcv_rect bounding_box = {
            { static_cast<jcv_real>(r.left - uScale * 1.1), static_cast<jcv_real>(r.top - uScale * 1.1) },
            { static_cast<jcv_real>(r.right + uScale * 1.1), static_cast<jcv_real>(r.bottom + uScale * 1.1) }
        };
        jcv_diagram diagram;
        jcv_diagram_generate(points.size(), points.data(), &bounding_box, &diagram);
        auto toIntPoint = [](const jcv_edge* edge, int num) -> const IntPoint {
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
                    edges[edge->sites[0]->p.id * edge->sites[0]->p.id ^ edge->sites[1]->p.id * edge->sites[1]->p.id].insert(pair); // other
                graph_edge = graph_edge->next;
            }
        }
        jcv_diagram_free(&diagram);
    }

    //progressOrCancel(7, 5); // progress
    for (const Pairs& edge : edges) {
        m_returnPs.append(toPath(edge));
        progress(edges.size(), m_returnPs.size()); // progress
    }
    mergePaths(m_returnPs);
    m_returnPs.append(toPath(frame));
    //progressOrCancel(7, 6); // progress
    for (int i = 0; i < m_returnPs.size(); ++i) { // remove verry short paths
        if (m_returnPs[i].size() < 4 && Length(m_returnPs[i].first(), m_returnPs[i].last()) < tolerance * 0.5 * uScale)
            m_returnPs.remove(i--);
    }
    //progressOrCancel(7, 7); // progress
    if (width < tool.getDiameter(depth)) {
        m_file = new File({ sortBE(m_returnPs) }, tool, depth, Voronoi);
        m_file->setFileName(tool.name());
        emit fileReady(m_file);
    } else {
        createOffset(tool, depth, width);
        m_file = new File(m_returnPss, tool, depth, Voronoi, m_workingRawPs);
        m_file->setFileName(tool.name());
        emit fileReady(m_file);
    }
#endif
}

void VoronoiCreator::createOffset(const Tool& tool, double depth, const double width)
{
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
            tmpPaths.append(tmpPaths1);
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
    m_returnPss.append({ frame });
}

void VoronoiCreator::mergePaths(Paths& paths)
{
    const double dist = 0.005 * uScale; // mm // 0.001;
    sortBE(paths);
    const int max = paths.size();
    for (int k = 0; k < 10; ++k) {
        for (int i = 0; i < paths.size(); ++i) {
            progress(max, max - paths.size()); // progress
            for (int j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                if (paths[i].first() == paths[j].first()) {
                    ReversePath(paths[j]);
                    paths[j].append(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                }
                if (paths[i].last() == paths[j].last()) {
                    ReversePath(paths[j]);
                    paths[i].append(paths[j].mid(1));
                    paths.remove(j--);
                    break;
                }
                if (Length(paths[i].last(), paths[j].last()) < dist) {
                    ReversePath(paths[j]);
                    paths[i].append(paths[j].mid(1));
                    paths.remove(j--);
                    continue; // break;
                }
                if (Length(paths[i].last(), paths[j].first()) < dist) {
                    paths[i].append(paths[j].mid(1));
                    paths.remove(j--);
                    continue; // break;
                }
                if (Length(paths[i].first(), paths[j].last()) < dist) {
                    paths[j].append(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                }
                if (Length(paths[i].first(), paths[j].first()) < dist) {
                    ReversePath(paths[j]);
                    paths[j].append(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                }
            }
        }
    }
}

void VoronoiCreator::clean(Path& path)
{
    for (int i = 1; i < path.size() - 2; ++i) {
        QLineF line(toQPointF(path[i]), toQPointF(path[i + 1]));
        if (line.length() < m_gcp.dParam[Tolerance]) {
            path[i] = toIntPoint(line.center());
            path.remove(i + 1);
            --i;
        }
    }
    const double kAngle = 1; //0.2;
    for (int i = 1; i < path.size() - 1; ++i) {
        const double a1 = Angle(path[i - 1], path[i]);
        const double a2 = Angle(path[i], path[i + 1]);
        if (abs(a1 - a2) < kAngle) {
            path.remove(i--);
        }
    }
}

void VoronoiCreator::mergeSegments(QList<OrdPath*>& merge)
{
    const int max = merge.size();
    //const double dist = 0.01 * uScale; // mm // 0.001;
    for (int i = 0; i < merge.size(); ++i) {
        progress(max, max - merge.size()); // progress
        for (int j = 0; j < merge.size(); ++j) {
            if (i == j)
                continue;
            if (merge[i]->Last->Pt == merge[j]->Pt) {
                merge[i]->append(merge[j]->Next);
                merge.removeAt(j--);
                continue;
            }
            if (merge[i]->Pt == merge[j]->Last->Pt) {
                merge[j]->append(merge[i]->Next);
                merge.removeAt(i--);
                break;
            }
        }
    }
}

Paths VoronoiCreator::toPath(const Pairs& pairs)
{
    Paths paths;
    Pairs tmp = pairs;
    QList<Pair> tmp2(tmp.toList());
    std::sort(tmp2.begin(), tmp2.end(), [](const Pair& a, const Pair& b) { return a.id > b.id; });
    QList<QSharedPointer<OrdPath>> holder;
    QList<OrdPath*> merge;
    for (const Pair& path : tmp2) {
        OrdPath* pt1 = new OrdPath;
        OrdPath* pt2 = new OrdPath;
        pt1->Pt = path.first;
        pt1->Next = pt2;
        pt1->Last = pt2;
        pt2->Pt = path.second;
        pt2->Prev = pt1;
        holder.append(QSharedPointer<OrdPath>(pt1));
        holder.append(QSharedPointer<OrdPath>(pt2));
        merge.append(pt1);
    }
    mergeSegments(merge);
    paths.resize(merge.size());
    for (int i = 0; i < merge.size(); ++i) {
        paths[i] = merge[i]->toPath();
    }
    //qDebug() << "";
    //const int s = paths.size();
    mergePaths(paths);
    //qDebug() << "paths" << paths.size() << (paths.size() - s);
    for (Path& path : paths) {
        //const int s = path.size();
        clean(path);
        //qDebug() << "path" << path.size() << (path.size() - s);
    }
    return paths;
}
}
