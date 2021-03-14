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

using coordinate_type = double;

struct Point {
    cInt a;
    cInt b;
    Point(cInt x, cInt y)
        : a(x)
        , b(y)
    {
    }
};

struct Segment {
    Point p0;
    Point p1;
    int id;
    Segment(cInt x1, cInt y1, cInt x2, cInt y2, int id = -1)
        : p0(x1, y1)
        , p1(x2, y2)
        , id(id)
    {
    }
};

namespace boost {
namespace polygon {

    template <>
    struct geometry_concept<Point> {
        typedef point_concept type;
    };

    template <>
    struct point_traits<Point> {
        typedef int coordinate_type;
        static inline coordinate_type get(const Point& point, orientation_2d orient)
        {
            return (orient == HORIZONTAL) ? point.a : point.b;
        }
    };

    template <>
    struct geometry_concept<Segment> {
        typedef segment_concept type;
    };

    template <>
    struct segment_traits<Segment> {
        typedef int coordinate_type;
        typedef Point point_type;
        static inline point_type get(const Segment& segment, direction_1d dir)
        {
            return dir.to_int() ? segment.p1 : segment.p0;
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

    int id = 0;
    //add line segments to diagram
    msg = tr("Calc BOOST Voronoi");

    size_t max {};
    for (const Paths& paths : m_groupedPss)
        for (const Path& path : paths)
            max += path.size();
    max *= 1.5;
    setMax(max);
    setCurrent();
    std::vector<Segment> srcSegments;
    srcSegments.reserve(max);

    for (const Paths& paths : m_groupedPss) {
        ++id;
        for (const Path& path : paths) {
            for (size_t i = 0; i < path.size(); ++i) {
                incCurrent();
                getCancelThrow();
                const IntPoint& point = path[i];

                !i ? srcSegments.emplace_back(path.back().X, path.back().Y, point.X, point.Y, id)
                   : srcSegments.emplace_back(path[i - 1].X, path[i - 1].Y, point.X, point.Y, id);

                maxX = std::max(maxX, point.X);
                maxY = std::max(maxY, point.Y);
                minX = std::min(minX, point.X);
                minY = std::min(minY, point.Y);
            }
        }
    }
    const cInt kx = (maxX - minX) * 2;
    const cInt ky = (maxY - minY) * 2;
    srcSegments.emplace_back(maxX + kx, minY - ky, maxX + kx, maxY + ky, ++id);
    srcSegments.emplace_back(maxX + kx, minY - ky, minX - kx, minY - ky, id);
    srcSegments.emplace_back(minX - kx, maxY + ky, maxX + kx, maxY + ky, id);
    srcSegments.emplace_back(minX - kx, minY - ky, minX - kx, maxY + ky, id);

    qDebug() << "max id:" << id;
    //    const cInt kx = (maxX - minX) * 2;
    //    const cInt ky = (maxY - minY) * 2;

    Paths segments;
    {
        voronoi_diagram<double> vd;
        construct_voronoi(srcSegments.begin(), srcSegments.end(), &vd);

        for (auto& cell : vd.cells()) {
            cell.color(srcSegments[cell.source_index()].id);
        }

        std::set<Path> set;

        for (auto& edge : vd.edges()) {
            auto v0 = edge.vertex0();
            auto v1 = edge.vertex1();

            auto cell1 = edge.cell();
            auto cell2 = edge.twin()->cell();

            auto color1 = cell1->color();
            auto color2 = cell2->color();

            if (v0 && v1) {
                IntPoint p0 { static_cast<cInt>(v0->x()), static_cast<cInt>(v0->y()) };
                IntPoint p1 { static_cast<cInt>(v1->x()), static_cast<cInt>(v1->y()) };
                if (color1 != color2 && color1 && color2) {
                    if (auto [it, b] = set.emplace(Path { p0, p1 }); b) {
                        if (auto [it, b] = set.emplace(Path { p1, p0 }); b) {
                            segments.emplace_back(Path { p0, p1 });
                        }
                    }
                }
            }
        }
    }
    mergeSegments(segments, 0.005 * uScale);

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

    m_returnPs = segments;
    m_returnPs.push_back(frame);
}
}
