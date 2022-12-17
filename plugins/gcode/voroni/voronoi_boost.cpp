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
#include "voronoi_boost.h"

#if __has_include(<boost/polygon/voronoi.hpp>)
#include "mvector.h"

#include <cstdio>
#include <vector>

#pragma warning(push)
#pragma warning(disable : 5055)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-float-conversion"
// Your function
#include "voronoi_visual_utils.h"
#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>

#pragma GCC diagnostic pop
#pragma warning(pop)

using boost::polygon::high;
using boost::polygon::low;
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;

using coordinate_type = double;
using point_type = boost::polygon::point_data<coordinate_type>;
using segment_type = boost::polygon::segment_data<coordinate_type>;
typedef boost::polygon::rectangle_data<coordinate_type> rect_type;
typedef voronoi_builder<int> VB;
typedef voronoi_diagram<coordinate_type> VD;
typedef VD::cell_type cell_type;
typedef VD::cell_type::source_index_type source_index_type;
typedef VD::cell_type::source_category_type source_category_type;
typedef VD::edge_type edge_type;
typedef VD::cell_container_type cell_container_type;
typedef VD::cell_container_type vertex_container_type;
typedef VD::edge_container_type edge_container_type;
typedef VD::const_cell_iterator const_cell_iterator;
typedef VD::const_vertex_iterator const_vertex_iterator;
typedef VD::const_edge_iterator const_edge_iterator;

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

    std::vector<point_type> sampled_edge {
        point_type {edge.vertex0()->x(), edge.vertex0()->y()},
        point_type {edge.vertex1()->x(), edge.vertex1()->y()}
    };
    coordinate_type max_dist = uScale * 0.00001; //* 1E-3* (xh(brect_) - xl(brect_));

    point_type point = edge.cell()->contains_point() ? retrieve_point(segment_data_, *edge.cell()) : retrieve_point(segment_data_, *edge.twin()->cell());

    segment_type segment = edge.cell()->contains_point() ? retrieve_segment(segment_data_, *edge.twin()->cell()) : retrieve_segment(segment_data_, *edge.cell());

    boost::polygon::voronoi_visual_utils<coordinate_type>::discretize(point, segment, max_dist, &sampled_edge);

    //    qDebug() << "discretize" << sampled_edge.size();
    Path path;
    path.reserve(sampled_edge.size());
    for (const auto& p : sampled_edge)
        path.emplace_back(static_cast<cInt>(p.x()), static_cast<cInt>(p.y()));
    return path;
}
namespace GCode {

void VoronoiBoost::boostVoronoi() {
    const double tolerance = gcp_.params[GCodeParams::Tolerance].toDouble() * uScale;

    cInt minX = std::numeric_limits<cInt>::max(),
         minY = std::numeric_limits<cInt>::max(),
         maxX = std::numeric_limits<cInt>::min(),
         maxY = std::numeric_limits<cInt>::min();

    int id = 0, id2 = 0;
    // add line segments to diagram
    msg = tr("Calc BOOST Voronoi");

    size_t max {};
    for (const Paths& paths : groupedPss)
        for (const Path& path : paths)
            max += path.size();
    max *= 1.5;
    setMax(max);
    setCurrent();
    std::vector<segment_type> srcSegments;
    srcSegments.reserve(max);
    std::vector<int> vecId;
    srcSegments.reserve(max);

    for (const Paths& paths : groupedPss) {
        ++id;
        for (const Path& path : paths) {
            for (size_t i = 0; i < path.size(); ++i) {
                incCurrent();
                ifCancelThenThrow();
                const IntPoint& point = path[i];
                vecId.emplace_back(id);
                //                !i ? srcSegments.emplace_back(path.back().X, path.back().Y, point.X, point.Y /*, id, id2++*/)
                //                   : srcSegments.emplace_back(path[i - 1].X, path[i - 1].Y, point.X, point.Y /*, id, id2++*/);
                !i ? srcSegments.emplace_back(
                    point_type {static_cast<coordinate_type>(path.back().X), static_cast<coordinate_type>(path.back().Y)},
                    point_type {static_cast<coordinate_type>(point.X), static_cast<coordinate_type>(point.Y)}) :
                     srcSegments.emplace_back(
                         point_type {static_cast<coordinate_type>(path[i - 1].X), static_cast<coordinate_type>(path[i - 1].Y)},
                         point_type {static_cast<coordinate_type>(point.X), static_cast<coordinate_type>(point.Y)});
                maxX = std::max(maxX, point.X);
                maxY = std::max(maxY, point.Y);
                minX = std::min(minX, point.X);
                minY = std::min(minY, point.Y);
            }
        }
    }
    const cInt kx = (maxX - minX) * 2;
    const cInt ky = (maxY - minY) * 2;
    //    srcSegments.emplace_back(maxX + kx, minY - ky, maxX + kx, maxY + ky, ++id);
    //    srcSegments.emplace_back(maxX + kx, minY - ky, minX - kx, minY - ky, id);
    //    srcSegments.emplace_back(minX - kx, maxY + ky, maxX + kx, maxY + ky, id);
    //    srcSegments.emplace_back(minX - kx, minY - ky, minX - kx, maxY + ky, id);
    vecId.emplace_back(++id);
    srcSegments.emplace_back(
        point_type {static_cast<coordinate_type>(maxX + kx), static_cast<coordinate_type>(minY - ky)},
        point_type {static_cast<coordinate_type>(maxX + kx), static_cast<coordinate_type>(maxY + ky)});
    vecId.emplace_back(id);
    srcSegments.emplace_back(
        point_type {static_cast<coordinate_type>(maxX + kx), static_cast<coordinate_type>(minY - ky)},
        point_type {static_cast<coordinate_type>(minX - kx), static_cast<coordinate_type>(minY - ky)});
    vecId.emplace_back(id);
    srcSegments.emplace_back(
        point_type {static_cast<coordinate_type>(minX - kx), static_cast<coordinate_type>(maxY + ky)},
        point_type {static_cast<coordinate_type>(maxX + kx), static_cast<coordinate_type>(maxY + ky)});
    vecId.emplace_back(id);
    srcSegments.emplace_back(
        point_type {static_cast<coordinate_type>(minX - kx), static_cast<coordinate_type>(minY - ky)},
        point_type {static_cast<coordinate_type>(minX - kx), static_cast<coordinate_type>(maxY + ky)});

    qDebug() << "max id:" << id;
    //    const cInt kx = (maxX - minX) * 2;
    //    const cInt ky = (maxY - minY) * 2;

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

        for (auto& edge : vd.edges()) {
            auto v0 = edge.vertex0();
            auto v1 = edge.vertex1();

            //            auto cell1 = edge.cell();
            //            auto cell2 = edge.twin()->cell();

            auto color1 = id0(edge);
            auto color2 = id1(edge);

            if (v0 && v1) {
                IntPoint p0 {static_cast<cInt>(v0->x()), static_cast<cInt>(v0->y())};
                IntPoint p1 {static_cast<cInt>(v1->x()), static_cast<cInt>(v1->y())};
                if (color1 != color2 && color1 && color2) {
                    if (set.emplace(Path {p0, p1}).second && set.emplace(Path {p1, p0}).second) {
                        if (edge.is_curved() && p0.distTo(p1) < tolerance) {
                            segments.emplace_back(sample_curved_edge(srcSegments, edge));
                            // segments.emplace_back(Path { p0, p1 });
                        } else {
                            segments.emplace_back(Path {p0, p1});
                        }
                    }
                }
            }
        }
    }
    mergeSegments(segments, 0.005 * uScale);

    const cInt fo = gcp_.params[GCodeParams::FrameOffset].toDouble() * uScale;
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
        clipper.Execute(ctIntersection, segments, pftNonZero);
    }

    //    dbgPaths(segments, "segments");
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
    std::ranges::for_each(segments, clean);

    returnPs = segments;
    returnPs.push_back(frame);
}

} // namespace GCode
#else
void GCode::VoronoiBoost::boostVoronoi() {
}
#endif
