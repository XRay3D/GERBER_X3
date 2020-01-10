#include "gcvoronoi.h"
#include "gcfile.h"
#include "gcvoronoi.h"
#include "voroni/jc_voronoi.h"
#include <type_traits>
#ifdef _USE_CGAL_
#include <CGAL/Algebraic_structure_traits.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Segment_Delaunay_graph_2.h>
#include <CGAL/Segment_Delaunay_graph_filtered_traits_2.h>
#include <CGAL/Segment_Delaunay_graph_storage_traits_with_info_2.h>
#include <CGAL/Segment_Delaunay_graph_traits_2.h>

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
using Gt = CGAL::Segment_Delaunay_graph_filtered_traits_2<K>;
using ST = CGAL::Segment_Delaunay_graph_storage_traits_with_info_2<Gt, int, convert_info, merge_info>;
using D_S = CGAL::Triangulation_data_structure_2<CGAL::Segment_Delaunay_graph_vertex_base_2<ST>, CGAL::Segment_Delaunay_graph_face_base_2<Gt>>;
using SDG2 = CGAL::Segment_Delaunay_graph_2<Gt, ST, D_S>;

inline IntPoint toIntPoint(const CGAL::Point_2<K>& point)
{
    return IntPoint{ static_cast<cInt>(point.x()), static_cast<cInt>(point.y()) };
}
#endif
inline uint qHash(const IntPoint& key, uint /*seed*/ = 0)
{
    return qHash(QByteArray(reinterpret_cast<const char*>(&key), sizeof(IntPoint)));
}

namespace GCode {

inline uint qHash(const GCode::VoronoiCreator::Pair& tag, uint = 0)
{
    return ::qHash(tag.first.X ^ tag.second.X) ^ ::qHash(tag.first.Y ^ tag.second.Y);
}

void VoronoiCreator::create()
{
    self = this;

    const auto& tool = m_gcp.tool.first();
    const auto depth = m_gcp.dParam[Depth].toDouble();
    const auto width = m_gcp.dParam[Width].toDouble();
    const auto vorType = m_gcp.dParam[VorT].toInt();

    groupedPaths(CopperPaths);
    if (vorType) {
        cgalVoronoi();
    } else {
        jcVoronoi();
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
    m_stepOver = static_cast<cInt>(tool.stepover() * uScale);
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

void VoronoiCreator::mergePaths(QList<Path>& paths, const double dist)
{
    msg = tr("Merge Paths");
    auto mmm = [](Path& dst, Path& src) {
        if (src.size() == 2)
            dst.append(std::move(src.last()));
        else {
            src.takeFirst();
            dst.append(std::move(src));
        }
    };
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
                    mmm(paths[j], paths[i]);
                    paths.removeAt(i--);
                    break;
                } else if (paths[i].last() == paths[j].last()) {
                    ReversePath(paths[j]);
                    mmm(paths[i], paths[j]);
                    paths.removeAt(j--);
                    if (j >= i)
                        continue;
                    else
                        break;
                } else if (paths[i].first() == paths[j].last()) {
                    mmm(paths[j], paths[i]);
                    paths.removeAt(i--);
                    break;
                } else if (paths[j].first() == paths[i].last()) {
                    mmm(paths[i], paths[j]);
                    paths.removeAt(j--);
                    if (j >= i)
                        continue;
                    else
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
                        mmm(paths[i], paths[j]);
                        paths.removeAt(j--);
                        if (j >= i)
                            continue;
                        else
                            break;
                    } else if (Length(paths[i].last(), paths[j].first()) < dist) {
                        mmm(paths[i], paths[j]);
                        paths.removeAt(j--);
                        if (j >= i)
                            continue;
                        else
                            break;
                    } else if (Length(paths[i].first(), paths[j].last()) < dist) {
                        mmm(paths[j], paths[i]);
                        paths.removeAt(i--);
                        break;
                    } else if (Length(paths[i].first(), paths[j].first()) < dist) {
                        ReversePath(paths[j]);
                        mmm(paths[j], paths[i]);
                        paths.removeAt(i--);
                        break;
                    }
                }
            }
        }
    } while (max != paths.size());
}

void VoronoiCreator::clean(Path& path)
{
    const auto tolerance = m_gcp.dParam[Tolerance].toDouble();
    for (int i = 1; i < path.size() - 2; ++i) {
        QLineF line(toQPointF(path[i]), toQPointF(path[i + 1]));
        if (line.length() < tolerance) {
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

void VoronoiCreator::cgalVoronoi()
{
#ifdef _USE_CGAL_
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
    QList<Path> segments;
    segments.reserve(id * 2);

    for (auto eit = sdg.finite_edges_begin(); eit != sdg.finite_edges_end(); ++eit) {
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
            segments.append({ toIntPoint(sdgLine.point(0)), toIntPoint(sdgLine.point(1)) });
        } else if (CGAL::assign(sdgRay, o)) {
            segments.append({ toIntPoint(sdgRay.point(0)), toIntPoint(sdgRay.point(1)) });
        } else if (CGAL::assign(sdgSegment, o) && !sdgSegment.is_degenerate()) {
            segments.append({ toIntPoint(sdgSegment.point(0)), toIntPoint(sdgSegment.point(1)) });
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

    const cInt offset = static_cast<cInt>(m_gcp.dParam[FrameOffset].toDouble() * uScale);
    Path frame{
        { minX - offset, minY - offset },
        { minX - offset, maxY + offset },
        { maxX + offset, maxY + offset },
        { maxX + offset, minY - offset },
        { minX - offset, minY - offset },
    };
    mergePaths(segments, 0.02 * uScale);
    m_returnPs = segments.toVector();
    {
        Clipper clipper;
        clipper.AddPaths(m_returnPs, ptSubject, false);
        clipper.AddPath(frame, ptClip, true);
        clipper.Execute(ctIntersection, m_returnPs, pftNonZero);
    }
    m_returnPs.append(frame);
#endif
}

void VoronoiCreator::jcVoronoi()
{
    const auto tolerance = m_gcp.dParam[Tolerance].toDouble();

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
        const cInt offset = static_cast<cInt>(m_gcp.dParam[FrameOffset].toDouble() * uScale);
        jcv_rect bounding_box = {
            { static_cast<jcv_real>(r.left - offset), static_cast<jcv_real>(r.top - offset) },
            { static_cast<jcv_real>(r.right + offset), static_cast<jcv_real>(r.bottom + offset) }
        };
        jcv_diagram diagram;
        jcv_diagram_generate(points.size(), points.data(), &bounding_box, &diagram);
        auto toIntPoint = [](const jcv_edge* edge, int num) -> const IntPoint {
            return { static_cast<cInt>(edge->pos[num].x), static_cast<cInt>(edge->pos[num].y) };
        };
        const jcv_site* sites = jcv_diagram_get_sites(&diagram);
        for (int i = 0; i < diagram.numsites; i++) {
            progress(diagram.numsites, i); // progress
            jcv_graphedge* graph_edge = sites[i].edges;
            while (graph_edge) {
                const jcv_edge* edge = graph_edge->edge;
                const Pair pair{ toIntPoint(edge, 0), toIntPoint(edge, 1), sites[i].p.id };
                if (edge->sites[0] == nullptr || edge->sites[1] == nullptr)
                    frame.insert(pair); // frame
                else if (edge->sites[0]->p.id != edge->sites[1]->p.id)
                    edges[edge->sites[0]->p.id ^ edge->sites[1]->p.id].insert(pair); // other
                graph_edge = graph_edge->next;
            }
        }
        jcv_diagram_free(&diagram);
    }

    QList<Path> segments;
    for (auto&& edge : edges) {
        segments.append(toPaths(std::move(edge), true));
        progress(edges.size(), m_returnPs.size()); // progress
    }
    mergePaths(segments, 0.005 * uScale);
    m_returnPs = segments.toVector();
    m_returnPs.append(toPaths(std::move(frame)).toVector());
    for (int i = 0; i < m_returnPs.size(); ++i) { // remove verry short paths
        if (m_returnPs[i].size() < 4 && Length(m_returnPs[i].first(), m_returnPs[i].last()) < tolerance * 0.5 * uScale)
            m_returnPs.remove(i--);
    }
}

QList<Path> VoronoiCreator::toPaths(Pairs&& pairs, bool bClean)
{
    msg = tr("Merge Segments");
    QList<Path> paths;
    Pairs tmp = pairs;
    QList<Pair> tmp2(tmp.toList());
    std::sort(tmp2.begin(), tmp2.end(), [](const Pair& a, const Pair& b) { return a.id > b.id; });

    {
        QVector<OrdPath> holder(tmp2.count() * 2);
        QList<OrdPath*> merge;
        OrdPath* it = holder.data();
        for (const Pair& path : tmp2) {
            OrdPath* pt1 = it++;
            OrdPath* pt2 = it++;
            pt1->Pt = path.first;
            pt1->Next = pt2;
            pt1->Last = pt2;
            pt2->Pt = path.second;
            pt2->Prev = pt1;
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
        paths.reserve(merge.size());
        for (auto& var : merge) {
            paths.append(var->toPath());
        }
    }

    mergePaths(paths, 0.005 * uScale);
    if (bClean)
        for (Path& path : paths)
            clean(path);

    return paths;
}
}
