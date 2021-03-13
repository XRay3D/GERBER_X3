// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcvoronoiboost.h"
#include "mvector.h"

#include <cstdio>
#include <vector>

#include "voronoi_visual_utils.hpp"
#include <boost/polygon/polygon.hpp>
#include <boost/polygon/voronoi.hpp>

using boost::polygon::high;
using boost::polygon::low;
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;
using boost::polygon::x;
using boost::polygon::y;

using coordinate_type2 = cInt;

//struct Point {
//    coordinate_type2 a;
//    coordinate_type2 b;
//    Point(cInt x, cInt y)
//        : a(x)
//        , b(y)
//    {
//    }
//};

//struct Segment {
//    Point p0;
//    Point p1;
//    int id;
//    Segment(coordinate_type2 x1, coordinate_type2 y1, coordinate_type2 x2, coordinate_type2 y2, int id = -1)
//        : p0(x1, y1)
//        , p1(x2, y2)
//        , id(id)
//    {
//    }
//};
using namespace boost::polygon;

using coordinate_type = double;
using point_type = point_data<coordinate_type>;
using segment_type = segment_data<coordinate_type>;
using rect_type = rectangle_data<coordinate_type>;
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

using Point = point_type;
using Segment = segment_type;

namespace boost {
namespace polygon {

    template <>
    struct geometry_concept<Point> {
        using type = point_concept;
    };

    template <>
    struct point_traits<Point> {
        using coordinate_type = int;
        static inline coordinate_type get(const Point& point, orientation_2d orient)
        {
            return (orient == HORIZONTAL) ? point.x() : point.y();
            // return (orient == HORIZONTAL) ? point.a : point.b;
        }
    };

    template <>
    struct geometry_concept<Segment> {
        using type = segment_concept;
    };

    template <>
    struct segment_traits<Segment> {
        using coordinate_type = int;
        using point_type = Point;
        static inline point_type get(const Segment& segment, direction_1d dir)
        {
            return dir.to_int() ? segment.high() : segment.low();
            // return dir.to_int() ? segment.p1 : segment.p0;
        }
    };
} // polygon
} // boost

namespace GCode {

void VoronoiBoost::boostVoronoi()
{

    cInt minX = std::numeric_limits<cInt>::max(),
         minY = std::numeric_limits<cInt>::max(),
         maxX = std::numeric_limits<cInt>::min(),
         maxY = std::numeric_limits<cInt>::min();

    int idCtr = 0;
    //add line segments to diagram
    msg = tr("Calc BOOST Voronoi");

    size_t max {};
    for (const Paths& paths : m_groupedPss)
        for (const Path& path : paths)
            max += path.size();
    max *= 1.5;
    setMax(max);
    setCurrent();

    std::vector<int> id;
    id.reserve(max);

    std::vector<segment_type> segments;
    segments.reserve(max);

    std::set<point_type> set;

    {
        size_t srcCtr {};
        for (const Paths& paths : m_groupedPss) {
            ++idCtr;
            for (const Path& path : paths) {
                for (size_t i = 0; i < path.size(); ++i) {
                    incCurrent();
                    getCancelThrow();
                    const IntPoint& point = path[i];

                    set.emplace(point);
                    id[srcCtr++] = idCtr;

                    !i ? segments.emplace_back(path.back().X, path.back().Y, point.X, point.Y)
                       : segments.emplace_back(path[i - 1].X, path[i - 1].Y, point.X, point.Y);

                    maxX = std::max(maxX, point.X);
                    maxY = std::max(maxY, point.Y);
                    minX = std::min(minX, point.X);
                    minY = std::min(minY, point.Y);
                }
            }
        }
    }
    qDebug() << "max id:" << idCtr;
    const cInt kx = (maxX - minX) * 2;
    const cInt ky = (maxY - minY) * 2;

    // Construction of the Voronoi Diagram.
    voronoi_diagram<double> vd;
    construct_voronoi(segments.begin(), segments.end(), &vd);

    // Using color member of the Voronoi primitives to store the average number
    // of edges around each cell (including secondary edges).
    Paths paths;
    {
        for (auto& cell : vd.cells()) {
            cell.color(id[cell.source_index()]);
        }

        using point_type = boost::polygon::point_data<coordinate_type>;
        std::vector<point_type> sampled_edge;

        auto retrieve_point = [&](auto& cell) {
            auto index = cell.source_index();
            auto category = cell.source_category();
            //            if (category == boost::polygon::SOURCE_CATEGORY_SINGLE_POINT) {
            //                return point_data_[index];
            //            }
            //            index -= point_data_.size();
            if (category == boost::polygon::SOURCE_CATEGORY_SEGMENT_START_POINT) {
                return low(segments[index]);
            } else {
                return high(segments[index]);
            }
        };

        auto retrieve_segment = [&](auto& cell) {
            auto index = cell.source_index();
            return segments[index];
        };

        qDebug("Number of edges (including secondary) around the Voronoi cells:\n");
        for (auto& edge : vd.edges()) {
            auto v0 = edge.vertex0();
            auto v1 = edge.vertex1();
            auto c1 = edge.cell()->color();
            auto c2 = edge.twin()->cell()->color();
            if (v0 && v1) {
                if (c1 != c2 && c1 && c2) {
                    if (edge.is_curved()) {
                        //                        //coordinate_type max_dist = 1E-3 * (xh(brect_) - xl(brect_));
                        //                        auto point = edge.cell()->contains_point() ? retrieve_point(*edge.cell()) : retrieve_point(*edge.twin()->cell());
                        //                        auto segment = edge.cell()->contains_point() ? retrieve_segment(*edge.twin()->cell()) : retrieve_segment(*edge.cell());
                        //                        boost::polygon::voronoi_visual_utils<coordinate_type>::discretize(point, segment, /*max_dist*/ 0.1, sampled_edge);
                        IntPoint p0 { static_cast<cInt>(v0->x()), static_cast<cInt>(v0->y()) };
                        IntPoint p1 { static_cast<cInt>(v1->x()), static_cast<cInt>(v1->y()) };
                        paths.emplace_back(Path { p0, p1 });
                    } else {
                        IntPoint p0 { static_cast<cInt>(v0->x()), static_cast<cInt>(v0->y()) };
                        IntPoint p1 { static_cast<cInt>(v1->x()), static_cast<cInt>(v1->y()) };
                        paths.emplace_back(Path { p0, p1 });
                    }
                }
            }
        }
        mergeSegments(paths);
        mergeSegments(paths, 0.01 * uScale);
        sortBE(paths);
        dbgPaths(paths, "edges");

        //        for (auto& cell : vd.cells()) {
        //            qDebug("%llu ", cell.color());
        //        }
        //        qDebug("\n");
        //        qDebug("\n");
    }

    // Linking Voronoi cells with input geometries.
    //    {
    //        unsigned int cell_index = 0;
    //        for (auto& cell : vd.cells()) {
    //            size_t index = cell.source_index() - points.size();
    //            Point p0 = low(segments[index]);
    //            Point p1 = high(segments[index]);
    //            switch (cell.source_category()) {
    //            case boost::polygon::SOURCE_CATEGORY_SEGMENT_START_POINT:
    //                qDebug("Cell #%ud contains segment start point: (%d, %d).\n", cell_index, x(p0), y(p0));
    //                break;
    //            case boost::polygon::SOURCE_CATEGORY_SEGMENT_END_POINT:
    //                qDebug("Cell #%ud contains segment end point: (%d, %d).\n", cell_index, x(p0), y(p0));
    //                break;
    //            default:
    //                qDebug("Cell #%ud contains a segment: ((%d, %d), (%d, %d)). \n", cell_index, x(p0), y(p0), x(p1), y(p1));
    //                break;
    //            }
    //            ++cell_index;
    //        }
    //    }
}
}
