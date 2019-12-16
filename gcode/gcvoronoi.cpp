#include "gcvoronoi.h"
#include "gcfile.h"
#include "gcvoronoi.h"

#include "voroni/jc_voronoi.h"

#include <CGAL/Algebraic_structure_traits.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Segment_Delaunay_graph_2.h>
#include <CGAL/Segment_Delaunay_graph_filtered_traits_2.h>
#include <CGAL/Segment_Delaunay_graph_storage_traits_with_info_2.h>
#include <CGAL/Segment_Delaunay_graph_traits_2.h>

// an enum representing the color
enum /*Red_blue*/ {
    RED = 1,
    BLUE = 2,
    PURPLE = 3
};

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
        return info0; /*PURPLE*/
    }
};

typedef CGAL::Simple_cartesian<double> K;
//using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Gt = CGAL::Segment_Delaunay_graph_filtered_traits_2<K>;
using ST = CGAL::Segment_Delaunay_graph_storage_traits_with_info_2<Gt, int, convert_info, merge_info>;
using D_S = CGAL::Triangulation_data_structure_2<CGAL::Segment_Delaunay_graph_vertex_base_2<ST>, CGAL::Segment_Delaunay_graph_face_base_2<Gt>>;
using SDG2 = CGAL::Segment_Delaunay_graph_2<Gt, ST, D_S>;

inline IntPoint toIntPoint(const CGAL::Point_2<K>& point)
{
    return IntPoint{ static_cast<cInt>(point.x()), static_cast<cInt>(point.y()) };
}

inline uint qHash(const IntPoint& key, uint /*seed*/ = 0)
{
    return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(IntPoint)));
}

namespace GCode {

inline uint qHash(const GCode::VoronoiCreator::Pair& tag, uint = 0)
{
    return ::qHash(tag.first.X ^ tag.second.X /*, seed ^ 0xa317a317*/) ^ ::qHash(tag.first.Y ^ tag.second.Y /*, seed ^ 0x17a317a3*/);
}

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
    groupedPaths(CopperPaths);
    if (static_cast<int>(m_gcp.dParam[VorT])) {
        cInt minX = std::numeric_limits<cInt>::max(),
             minY = minX,
             maxX = std::numeric_limits<cInt>::min(),
             maxY = maxX;
        progress(4, 0);
        SDG2 sdg;
        int id = 0;
        //add line segments to diagram
        int c = 0;
        msg = tr("Calc Voronoi");
        for (const Paths& paths : m_groupedPss) {
            progress(m_groupedPss.size(), ++c);
            for (const Path& path : paths) {
                for (int i = 0; i < path.size(); ++i) {
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
        segments.reserve(id * 2);
        for (SDG2::Finite_edges_iterator eit = sdg.finite_edges_begin(); eit != sdg.finite_edges_end(); ++eit) {
            const SDG2::Edge e = *eit;
            CGAL_precondition(!sdg.is_infinite(e));
            if (e.first->vertex(sdg.cw(e.second))->storage_site().info() == e.first->vertex(sdg.ccw(e.second))->storage_site().info())
                continue;
            SDG2::Geom_traits::Line_2 sdgLine;
            SDG2::Geom_traits::Segment_2 sdgSegment;
            SDG2::Geom_traits::Ray_2 sdgRay;
            CGAL::Parabola_segment_2<Gt> cgalParabola;

            CGAL::Object o = sdg.primal(e);

            if (CGAL::assign(sdgLine, o)) {
                Path path{ toIntPoint(sdgLine.point(0)), toIntPoint(sdgLine.point(1)) };
                segments.append(path);
            } else if (CGAL::assign(sdgRay, o)) {
                Path path{ toIntPoint(sdgRay.point(0)), toIntPoint(sdgRay.point(1)) };
                segments.append(path);
            } else if (CGAL::assign(sdgSegment, o) && !sdgSegment.is_degenerate()) {
                Path path{ toIntPoint(sdgSegment.point(0)), toIntPoint(sdgSegment.point(1)) };
                segments.append(path);
            } else if (CGAL::assign(cgalParabola, o)) {
                std::vector<SDG2::Point_2> points;
                cgalParabola.generate_points(points, 5);
                segments.append(Path());
                Path& path = segments.last();
                path.reserve(static_cast<int>(points.size()));
                for (const SDG2::Point_2& pt : points)
                    path << toIntPoint(pt);
            }
        }
        Path frame{
            { minX - uScale, minY - uScale },
            { minX - uScale, maxY + uScale },
            { maxX + uScale, maxY + uScale },
            { maxX + uScale, minY - uScale },
            { minX - uScale, minY - uScale },
        };
        {
            Clipper clipper;
            clipper.AddPaths(segments, ptSubject, false);
            clipper.AddPath(frame, ptClip, true);
            clipper.Execute(ctIntersection, segments, pftNonZero);
        }
        mergePaths(segments, 0.02 * uScale);
        m_returnPs = segments;
        m_returnPs.append(frame);
    } else {
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
        {
            jcv_rect bounding_box = {
                { static_cast<jcv_real>(r.left - uScale), static_cast<jcv_real>(r.top - uScale) },
                { static_cast<jcv_real>(r.right + uScale), static_cast<jcv_real>(r.bottom + uScale) }
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
                    const Pair pair{ toIntPoint(edge, 0), toIntPoint(edge, 1), sites[i].p.id };
                    if (edge->sites[0] == nullptr || edge->sites[1] == nullptr)
                        frame.insert(pair); // frame
                    else if (edge->sites[0]->p.id != edge->sites[1]->p.id)
                        edges[edge->sites[0]->p.id * edge->sites[0]->p.id ^ edge->sites[1]->p.id * edge->sites[1]->p.id].insert(pair); // other
                    graph_edge = graph_edge->next;
                }
            }
            jcv_diagram_free(&diagram);
        }

        for (const Pairs& edge : edges) {
            m_returnPs.append(toPath(edge));
            progress(edges.size(), m_returnPs.size()); // progress
        }
        mergePaths(m_returnPs, 0.005 * uScale);
        m_returnPs.append(toPath(frame));
        for (int i = 0; i < m_returnPs.size(); ++i) { // remove verry short paths
            if (m_returnPs[i].size() < 4 && Length(m_returnPs[i].first(), m_returnPs[i].last()) < tolerance * 0.5 * uScale)
                m_returnPs.remove(i--);
        }
    }
    qDebug() << "m_returnPs" << m_returnPs.size();
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

void VoronoiCreator::mergePaths(Paths& paths, const double dist)
{
    msg = tr("Merge Paths");
    sortBE(paths);
    int max;
    do {
        max = paths.size();
        for (int i = 0; i < paths.size(); ++i) {
            progress(max, max - paths.size()); // progress
            for (int j = 0; j < paths.size(); ++j) {
                if (i == j)
                    continue;
                else if (paths[i].first() == paths[j].first()) {
                    ReversePath(paths[j]);
                    paths[j].append(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                } else if (paths[i].last() == paths[j].last()) {
                    ReversePath(paths[j]);
                    paths[i].append(paths[j].mid(1));
                    paths.remove(j--);
                    break;
                } else if (paths[i].first() == paths[j].last()) {
                    paths[j].append(paths[i].mid(1));
                    paths.remove(i--);
                    break;
                } else if (paths[j].first() == paths[i].last()) {
                    paths[i].append(paths[j].mid(1));
                    paths.remove(j--);
                    break;
                }
            }
        }
    } while (max != paths.size());
    do {
        max = paths.size();
        for (int k = 0; k < 10; ++k) {
            for (int i = 0; i < paths.size(); ++i) {
                progress(max, max - paths.size()); // progress
                for (int j = 0; j < paths.size(); ++j) {
                    if (i == j)
                        continue;
                    else if (Length(paths[i].last(), paths[j].last()) < dist) {
                        ReversePath(paths[j]);
                        paths[i].append(paths[j].mid(1));
                        paths.remove(j--);
                        continue; // break;
                    } else if (Length(paths[i].last(), paths[j].first()) < dist) {
                        paths[i].append(paths[j].mid(1));
                        paths.remove(j--);
                        continue; // break;
                    } else if (Length(paths[i].first(), paths[j].last()) < dist) {
                        paths[j].append(paths[i].mid(1));
                        paths.remove(i--);
                        break;
                    } else if (Length(paths[i].first(), paths[j].first()) < dist) {
                        ReversePath(paths[j]);
                        paths[j].append(paths[i].mid(1));
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

Paths VoronoiCreator::toPath(const Pairs& pairs)
{
    msg = tr("Merge Segments");
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

    const int max = merge.size();
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

    paths.resize(merge.size());
    for (int i = 0; i < merge.size(); ++i) {
        paths[i] = merge[i]->toPath();
    }

    mergePaths(paths, 0.005 * uScale);
    for (Path& path : paths)
        clean(path);

    return paths;
}
}
