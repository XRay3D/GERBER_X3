/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "voronoi_boost.h"
#include "types.h"

#if 0 // __has_include(<boost/polygon/voronoi.hpp>)
#include "mvector.h"

#include <cstdio>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 5055)
#elif defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wdeprecated-enum-float-conversion"
#endif /* _MSC_VER, __GNUC__ */

#include "voronoi_visual_utils.h"
#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic pop
#endif /* _MSC_VER, __GNUC__ */

using boost::polygon::high;
using boost::polygon::low;
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;

using coordinate_type = double;
using point_type = boost::polygon::point_data<coordinate_type>;
using segment_type = boost::polygon::segment_data<coordinate_type>;
using rect_type = boost::polygon::rectangle_data<coordinate_type>;
using VB = voronoi_builder<int>;
using VD = voronoi_diagram<coordinate_type>;
using cell_type = VD::cell_type;
using source_index_type = VD::cell_type::source_index_type;
using source_category_type = VD::cell_type::source_category_type;
using edge_type = VD::edge_type;
using cell_container_type = VD::cell_container_type;
using vertex_container_type = VD::cell_container_type;
using edge_container_type = VD::edge_container_type;
using const_cell_iterator = VD::const_cell_iterator;
using const_vertex_iterator = VD::const_vertex_iterator;
using const_edge_iterator = VD::const_edge_iterator;

segment_type retrieve_segment(std::vector<segment_type>& segment_data_, const cell_type& cell) {
    source_index_type index = cell.source_index(); // - point_data_.size();
    return segment_data_[index];
}

point_type retrieve_point(std::vector<segment_type>& segment_data_, const cell_type& cell) {
    source_index_type index = cell.source_index();
    source_category_type category = cell.source_category();
    //    if (category == boost::polygon::SOURCE_CATEGORY_SINGLE_POINT) {
    //        return point_data_[index];
    //    }
    //    index -= point_data_.size();
    return category == boost::polygon::SOURCE_CATEGORY_SEGMENT_START_POINT ? low(segment_data_[index]) : high(segment_data_[index]);
}

Path sample_curved_edge(std::vector<segment_type>& segment_data_, const edge_type& edge) {
    std::vector sampled_edge{
        point_type{edge.vertex0()->x(), edge.vertex0()->y()},
        point_type{edge.vertex1()->x(), edge.vertex1()->y()}
    };

    coordinate_type max_dist = uScale * 0.00001; //* 1E-3* (xh(brect_) - xl(brect_));
    point_type point = edge.cell()->contains_point() ? retrieve_point(segment_data_, *edge.cell()) : retrieve_point(segment_data_, *edge.twin()->cell());
    segment_type segment = edge.cell()->contains_point() ? retrieve_segment(segment_data_, *edge.twin()->cell()) : retrieve_segment(segment_data_, *edge.cell());
    boost::polygon::voronoi_visual_utils<coordinate_type>::discretize(point, segment, max_dist, &sampled_edge);

    Path path;
    path.reserve(sampled_edge.size());
    for (const auto& p: sampled_edge)
        path.emplace_back(static_cast</*Point::Type*/ int32_t>(p.x()), static_cast</*Point::Type*/ int32_t>(p.y()));
    return path;
}
namespace Voronoi {

void VoronoiBoost::boostVoronoi() {
    const double tolerance = gcp_.params[Tolerance].toDouble() * uScale;

    /*Point::Type*/ int32_t minX = std::numeric_limits</*Point::Type*/ int32_t>::max(),
                            minY = std::numeric_limits</*Point::Type*/ int32_t>::max(),
                            maxX = std::numeric_limits</*Point::Type*/ int32_t>::min(),
                            maxY = std::numeric_limits</*Point::Type*/ int32_t>::min();

    int32_t id = 0, id2 = 0;
    // add line segments to diagram
    msg = QObject::tr("Calc BOOST Voronoi");

    size_t max{};

    for (const Path& path: std::views::join(groupedPss))
        max += path.size();

    max *= 1.5;
    setMax(max);
    setCurrent();
    std::vector<segment_type> srcSegments;
    srcSegments.reserve(max);
    std::vector<int> vecId;
    srcSegments.reserve(max);

    for (const Paths& paths: groupedPss) {
        ++id;
        for (const Path& path: paths) {
            for (size_t i = 0; i < path.size(); ++i) {
                incCurrent();
                throwIfCancel();
                const Point& point = path[i];
                vecId.emplace_back(id);
                //                !i ? srcSegments.emplace_back(path.back().x, path.back().y, point.x, point.y /*, id, id2++*/)
                //                   : srcSegments.emplace_back(path[i - 1].x, path[i - 1].y, point.x, point.y /*, id, id2++*/);
                !i ? srcSegments.emplace_back(
                    point_type{static_cast<coordinate_type>(path.back().x), static_cast<coordinate_type>(path.back().y)},
                    point_type{static_cast<coordinate_type>(point.x), static_cast<coordinate_type>(point.y)})
                   : srcSegments.emplace_back(
                       point_type{static_cast<coordinate_type>(path[i - 1].x), static_cast<coordinate_type>(path[i - 1].y)},
                       point_type{static_cast<coordinate_type>(point.x), static_cast<coordinate_type>(point.y)});
                maxX = std::max<int32_t>(maxX, point.x);
                maxY = std::max<int32_t>(maxY, point.y);
                minX = std::min<int32_t>(minX, point.x);
                minY = std::min<int32_t>(minY, point.y);
            }
        }
    }
    const /*Point::Type*/ int32_t kx = (maxX - minX) * 2;
    const /*Point::Type*/ int32_t ky = (maxY - minY) * 2;
    //    srcSegments.emplace_back(maxX + kx, minY - ky, maxX + kx, maxY + ky, ++id);
    //    srcSegments.emplace_back(maxX + kx, minY - ky, minX - kx, minY - ky, id);
    //    srcSegments.emplace_back(minX - kx, maxY + ky, maxX + kx, maxY + ky, id);
    //    srcSegments.emplace_back(minX - kx, minY - ky, minX - kx, maxY + ky, id);
    vecId.emplace_back(++id);
    srcSegments.emplace_back(
        point_type{static_cast<coordinate_type>(maxX + kx), static_cast<coordinate_type>(minY - ky)},
        point_type{static_cast<coordinate_type>(maxX + kx), static_cast<coordinate_type>(maxY + ky)});
    vecId.emplace_back(id);
    srcSegments.emplace_back(
        point_type{static_cast<coordinate_type>(maxX + kx), static_cast<coordinate_type>(minY - ky)},
        point_type{static_cast<coordinate_type>(minX - kx), static_cast<coordinate_type>(minY - ky)});
    vecId.emplace_back(id);
    srcSegments.emplace_back(
        point_type{static_cast<coordinate_type>(minX - kx), static_cast<coordinate_type>(maxY + ky)},
        point_type{static_cast<coordinate_type>(maxX + kx), static_cast<coordinate_type>(maxY + ky)});
    vecId.emplace_back(id);
    srcSegments.emplace_back(
        point_type{static_cast<coordinate_type>(minX - kx), static_cast<coordinate_type>(minY - ky)},
        point_type{static_cast<coordinate_type>(minX - kx), static_cast<coordinate_type>(maxY + ky)});

    qDebug() << "max id:" << id;
    //    const /*Point::Type*/int32_t kx = (maxX - minX) * 2;
    //    const /*Point::Type*/int32_t ky = (maxY - minY) * 2;

    Paths segments;
    {
        voronoi_diagram<double> vd;
        construct_voronoi(srcSegments.begin(), srcSegments.end(), &vd);

        auto id0 = [&](auto edge) { return vecId[edge.cell()->source_index()]; };
        //        auto id2 = [&](auto edge) { return srcSegments[edge->cell()->source_index()].id2; };
        auto id1 = [&](auto edge) { return vecId[edge.twin()->cell()->source_index()]; };
        //        auto id2_ = [&](auto edge) { return srcSegments[edge.twin()->cell()->source_index()].id2; };

        //        for (auto& cell : vd.cells()) {
        //            cell.color(srcSegments[cell.source_index()].id);
        //        }

        std::set<Path> set;

        for (auto& edge: vd.edges()) {
            auto v0 = edge.vertex0();
            auto v1 = edge.vertex1();

            //            auto cell1 = edge.cell();
            //            auto cell2 = edge.twin()->cell();

            auto color1 = id0(edge);
            auto color2 = id1(edge);

            if (v0 && v1) {
                Point p0{static_cast</*Point::Type*/ int32_t>(v0->x()), static_cast</*Point::Type*/ int32_t>(v0->y())};
                Point p1{static_cast</*Point::Type*/ int32_t>(v1->x()), static_cast</*Point::Type*/ int32_t>(v1->y())};
                if (color1 != color2 && color1 && color2) {
                    if (set.emplace(Path{p0, p1}).second && set.emplace(Path{p1, p0}).second) {
                        if (edge.is_curved() && distTo(p0, p1) < tolerance) {
                            segments.emplace_back(sample_curved_edge(srcSegments, edge));
                            // segments.emplace_back(Path { p0, p1 });
                        } else {
                            segments.emplace_back(Path{p0, p1});
                        }
                    }
                }
            }
        }
    }
    mergeSegments(segments, 0.005 * uScale);

    const /*Point::Type*/ int32_t fo = gcp_.params[FrameOffset].toDouble() * uScale;
    Path frame{
        {minX - fo, minY - fo},
        {minX - fo, maxY + fo},
        {maxX + fo, maxY + fo},
        {maxX + fo, minY - fo},
        {minX - fo, minY - fo},
    };
    {
        Clipper clipper;
        clipper.AddOpenSubject(segments);
        clipper.AddClip({frame});
        clipper.Execute(ClipType::Intersection, FillRule::NonZero, segments, segments);
    }

    //    dbgPaths(segments, "segments");
    auto clean = [kAngle = 2.0](Path& path) {
        for (size_t i = 1; i < path.size() - 1; ++i) {
            const double a1 = angleTo(path[i - 1],path[i + 0]);
            const double a2 = angleTo(path[i + 0],path[i + 1]);
            if (abs(a1 - a2) < kAngle)
                path -= i--;
        }
    };

    std::ranges::for_each(segments, clean);
    std::ranges::for_each(segments, clean);

    returnPs = segments;
    returnPs.push_back(frame);
}

} // namespace Voronoi
#else
namespace Voronoi {
void VoronoiBoost::boostVoronoi() { }
} // namespace Voronoi
#endif
