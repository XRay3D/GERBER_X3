#include "gcvoronoi.h"
#include "gcfile.h"
#include "gcvoronoi.h"

//#include "voroni/jc_voronoi.h"
#include <QElapsedTimer>
#include <myclipper.h>
#include <tooldatabase/tool.h>

// includes for defining the Voronoi diagram adaptor
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_adaptation_policies_2.h>
#include <CGAL/Delaunay_triangulation_adaptation_traits_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Voronoi_diagram_2.h>
// typedefs for defining the adaptor
using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using DT = CGAL::Delaunay_triangulation_2<K>;
using AT = CGAL::Delaunay_triangulation_adaptation_traits_2<DT>;
using AP = CGAL::Delaunay_triangulation_caching_degeneracy_removal_policy_2<DT>;
using VD = CGAL::Voronoi_diagram_2<DT, AT, AP>;
// typedef for the result type of the point location
using Site_2 = AT::Site_2;
using Point_2 = AT::Point_2;
using Locate_result = VD::Locate_result;
using Vertex_handle = VD::Vertex_handle;
using Face_handle = VD::Face_handle;
using Halfedge_handle = VD::Halfedge_handle;
using Ccb_halfedge_circulator = VD::Ccb_halfedge_circulator;

//void print_endpoint(Halfedge_handle e, bool is_src)
//{
//    qDebug() << "\t";
//    if (is_src) {
//        if (e->has_source())
//            qDebug() << e->source()->point().x() << e->source()->point().y();
//        else
//            qDebug() << "point at infinity";
//    } else {
//        if (e->has_target())
//            qDebug() << e->target()->point().x() << e->target()->point().y();
//        else
//            qDebug() << "point at infinity";
//    }
//}

namespace GCode {

inline uint qHash(const GCode::VoronoiCreator::Pair& tag, uint seed = 0) { return ::qHash(tag.first.X * tag.second.X, seed ^ 0xa317a317) ^ ::qHash(tag.first.Y * tag.second.Y, seed ^ 0x17a317a3); }

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

    VD vd;

    //    for (Face_iterator f = VD.faces_begin(); f != VD.faces_end(); f++) {
    //        Ccb_halfedge_circulator ec_start = (f)->ccb();
    //        Ccb_halfedge_circulator ec = ec_start;
    //        do {
    //            if (!ec->has_source()) {
    //            } else
    //                QpolyF << QPointF(((Halfedge_handle)ec)->source()->point().x(), ((Halfedge_handle)ec)->source()->point().y());
    //        } while (++ec != ec_start);
    //        VectPolygon.push_back(QpolyF);
    //        QpolyF.clear();
    //    }

    for (const Paths& paths : m_groupedPss) {
        for (const Path& path : paths) {
            for (int i = 0; i < path.size(); ++i) {
                const IntPoint& point = path[i];
                //for (const IntPoint& point : path) {
                //                Site_2 t;
                //                if (point == path.first()) {
                //                    t << { path.last().X, path.last().Y };
                //                    t << { point.X, point.Y };
                //                } else {
                //                    t << { path[i - 1].X, path[i - 1].Y };
                //                    t << { point.X, point.Y };
                //                }
                //                vd.insert(t);
                vd.insert({ point.X, point.Y });
            }
        }
    }

    m_returnPss.clear();
    m_returnPss.resize(1);

    //    std::for_each(vd.edges_begin(), vd.edges_end(), [this](VD::Halfedge_handle e) {
    //        qDebug() << e->is_valid();
    //        //        m_returnPss.first().append(Path{
    //        //            IntPoint{ static_cast<cInt>(e->right()->point().x()), static_cast<cInt>(e->right()->point().y()) },
    //        //            IntPoint{ static_cast<cInt>(e->left()->point().x()), static_cast<cInt>(e->left()->point().y()) } });
    //    });
    for (VD::Face_iterator f = vd.faces_begin(); f != vd.faces_end(); f++) {
        Ccb_halfedge_circulator ec_start = f->ccb();
        Ccb_halfedge_circulator ec = ec_start;
        Path path;
        do {
            if (ec->has_source())
                path << IntPoint{
                    static_cast<cInt>(((Halfedge_handle)ec)->source()->point().x()),
                    static_cast<cInt>(((Halfedge_handle)ec)->source()->point().y())
                }; //((Halfedge_handle)ec)->source()->point().x(), ((Halfedge_handle)ec)->source()->point().y());
        } while (++ec != ec_start);
        m_returnPss.first().append(path);
    }

    m_file = new File(m_returnPss, tool, depth, Voronoi, m_workingRawPs);
    m_file->setFileName(tool.name());
    emit fileReady(m_file);

    //    std::ifstream ifs("data/data1.dt.cin");
    //    assert(ifs);
    //    ifs.close();
    //    assert(vd.is_valid());
    //    std::ifstream ifq("data/queries1.dt.cin");
    //    assert(ifq);
    //    Point_2 p;
    //    while (ifq >> p) {
    //        qDebug() << "Query point (" << p.x() << "," << p.y() << ") lies on a Voronoi ";
    //        Locate_result lr = vd.locate(p);
    //        if (Vertex_handle* v = boost::get<Vertex_handle>(&lr)) {
    //            qDebug() << "vertex.";
    //            qDebug() << "The Voronoi vertex is:";
    //            qDebug() << "\t" << (*v)->point().x() << (*v)->point().y();
    //        } else if (Halfedge_handle* e = boost::get<Halfedge_handle>(&lr)) {
    //            qDebug() << "edge.";
    //            qDebug() << "The source and target vertices "
    //                     << "of the Voronoi edge are:";
    //            print_endpoint(*e, true);
    //            print_endpoint(*e, false);
    //        } else if (Face_handle* f = boost::get<Face_handle>(&lr)) {
    //            qDebug() << "face.";
    //            qDebug() << "The vertices of the Voronoi face are"
    //                     << " (in counterclockwise order):";
    //            Ccb_halfedge_circulator ec_start = (*f)->ccb();
    //            Ccb_halfedge_circulator ec = ec_start;
    //            do {
    //                print_endpoint(ec, false);
    //            } while (++ec != ec_start);
    //        }
    //        qDebug();
    //    }
    //    ifq.close();

    // }

    //    QVector<jcv_point> points;
    //    points.reserve(100000);
    //    CleanPolygons(m_workingPs, tolerance * 0.1 * uScale);
    //    groupedPaths(CopperPaths);
    //    int id = 0;
    //    auto condei = [&points, tolerance, &id](IntPoint tmp, IntPoint point) { // split long segments
    //        QLineF line(toQPointF(tmp), toQPointF(point));
    //        if (line.length() > tolerance) {
    //            for (int i = 1, total = static_cast<int>(line.length() / tolerance); i < total; ++i) {
    //                line.setLength(i * tolerance);
    //                IntPoint point(toIntPoint(line.p2()));
    //                points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
    //            }
    //        }
    //    };
    //    progress(7, 1); // progress
    //    for (const Paths& paths : m_groupedPss) {
    //        for (const Path& path : paths) {

    //            IntPoint tmp(path.first());
    //            for (const IntPoint& point : path) {
    //                condei(tmp, point);
    //                points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
    //                tmp = point;
    //            }
    //            condei(tmp, path.first());
    //        }
    //        ++id;
    //    }
    //    progress(7, 2); // progress
    //    for (const Path& path : m_workingRawPs) {

    //        IntPoint tmp(path.first());
    //        for (const IntPoint& point : path) {
    //            condei(tmp, point);
    //            points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
    //            tmp = point;
    //        }
    //        condei(tmp, path.first());
    //        ++id;
    //    }

    //    Clipper clipper;
    //    for (const Paths& paths : m_groupedPss) {
    //        clipper.AddPaths(paths, ptClip, true);
    //    }
    //    clipper.AddPaths(m_workingRawPs, ptClip, true);
    //    const IntRect r(clipper.GetBounds());
    //    QMap<int, Pairs> edges;
    //    Pairs frame;
    //    //progressOrCancel(7, 3); // progress
    //    {
    //        jcv_rect bounding_box = {
    //            { static_cast<jcv_real>(r.left - uScale * 1.1), static_cast<jcv_real>(r.top - uScale * 1.1) },
    //            { static_cast<jcv_real>(r.right + uScale * 1.1), static_cast<jcv_real>(r.bottom + uScale * 1.1) }
    //        };
    //        jcv_diagram diagram;
    //        jcv_diagram_generate(points.size(), points.data(), &bounding_box, &diagram);
    //        auto toIntPoint = [](const jcv_edge* edge, int num) -> const IntPoint {
    //            return { static_cast<cInt>(edge->pos[num].x), static_cast<cInt>(edge->pos[num].y) };
    //        };
    //        const jcv_site* sites = jcv_diagram_get_sites(&diagram);
    //        for (int i = 0; i < diagram.numsites; i++) {

    //            jcv_graphedge* graph_edge = sites[i].edges;
    //            while (graph_edge) {
    //                const jcv_edge* edge = graph_edge->edge;
    //                const Pair pair { toIntPoint(edge, 0), toIntPoint(edge, 1), sites[i].p.id };
    //                if (edge->sites[0] == nullptr || edge->sites[1] == nullptr)
    //                    frame.insert(pair); // frame
    //                else if (edge->sites[0]->p.id != edge->sites[1]->p.id)
    //                    edges[edge->sites[0]->p.id * edge->sites[0]->p.id ^ edge->sites[1]->p.id * edge->sites[1]->p.id].insert(pair); // other
    //                graph_edge = graph_edge->next;
    //            }
    //        }
    //        jcv_diagram_free(&diagram);
    //    }

    //    //progressOrCancel(7, 5); // progress
    //    for (const Pairs& edge : edges) {
    //        m_returnPs.append(toPath(edge));
    //        progress(edges.size(), m_returnPs.size()); // progress
    //    }
    //    mergePaths(m_returnPs);
    //    m_returnPs.append(toPath(frame));
    //    //progressOrCancel(7, 6); // progress
    //    for (int i = 0; i < m_returnPs.size(); ++i) { // remove verry short paths
    //        if (m_returnPs[i].size() < 4 && Length(m_returnPs[i].first(), m_returnPs[i].last()) < tolerance * 0.5 * uScale)
    //            m_returnPs.remove(i--);
    //    }
    //    //progressOrCancel(7, 7); // progress
    //    if (width < tool.getDiameter(depth)) {
    //        m_file = new File({ sortBE(m_returnPs) }, tool, depth, Voronoi);
    //        m_file->setFileName(tool.name());
    //        emit fileReady(m_file);
    //    } else {
    //        createOffset(tool, depth, width);
    //        m_file = new File(m_returnPss, tool, depth, Voronoi, m_workingRawPs);
    //        m_file->setFileName(tool.name());
    //        emit fileReady(m_file);
    //    }
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
