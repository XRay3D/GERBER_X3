/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "voronoi_boost.h"
#include "types.h"

#if __has_include(<boost/polygon/voronoi.hpp>)

    #include <cstdio>
    #include <set>
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
using boost::polygon::voronoi_diagram;

// Boost.Polygon строит диаграмму ТОЛЬКО по целочисленному входу:
// construct_voronoi -- это voronoi_builder<int32>, и любой double на входе
// молча обрезается до целого. В миллиметрах это значит «до целых
// миллиметров»: пады слипаются в точки, и диаграмма выходит мусорной --
// рёбра режут тела насквозь вместо бисектрис между ними. Прежний
// Clipper2-слой давал целые сам собой (Point64 в единицах uScale), тут
// масштабируем явно, тем же 1e5 (10 нм), и разжимаем на выходе.
constexpr double kScale = 1e5;
inline std::int32_t toUnits(double mm) { return static_cast<std::int32_t>(std::llround(mm * kScale)); }
inline double fromUnits(double units) { return units / kScale; }

using coordinate_type = std::int32_t;
using point_type = boost::polygon::point_data<coordinate_type>;
using segment_type = boost::polygon::segment_data<coordinate_type>;
using VD = voronoi_diagram<double>;
using cell_type = VD::cell_type;
using source_index_type = VD::cell_type::source_index_type;
using source_category_type = VD::cell_type::source_category_type;
using edge_type = VD::edge_type;

segment_type retrieve_segment(std::vector<segment_type>& segment_data_, const cell_type& cell) {
    source_index_type index = cell.source_index();
    return segment_data_[index];
}

point_type retrieve_point(std::vector<segment_type>& segment_data_, const cell_type& cell) {
    source_index_type index = cell.source_index();
    source_category_type category = cell.source_category();
    return category == boost::polygon::SOURCE_CATEGORY_SEGMENT_START_POINT ? low(segment_data_[index]) : high(segment_data_[index]);
}

// Дискретизация параболического (кривого) ребра диаграммы -- ребра между
// точкой и отрезком. Шаг задаётся в мм и переводится в единицы диаграммы.
Geo::Polyline sample_curved_edge(std::vector<segment_type>& segment_data_, const edge_type& edge, double maxDistMm) {
    using dpoint = boost::polygon::point_data<double>;
    std::vector sampled_edge{
        dpoint{edge.vertex0()->x(), edge.vertex0()->y()},
        dpoint{edge.vertex1()->x(), edge.vertex1()->y()}
    };

    const point_type ipoint = edge.cell()->contains_point() ? retrieve_point(segment_data_, *edge.cell()) : retrieve_point(segment_data_, *edge.twin()->cell());
    const segment_type isegment = edge.cell()->contains_point() ? retrieve_segment(segment_data_, *edge.twin()->cell()) : retrieve_segment(segment_data_, *edge.cell());
    const dpoint point{static_cast<double>(ipoint.x()), static_cast<double>(ipoint.y())};
    const boost::polygon::segment_data<double> segment{
        dpoint{static_cast<double>(isegment.low().x()), static_cast<double>(isegment.low().y())},
        dpoint{static_cast<double>(isegment.high().x()), static_cast<double>(isegment.high().y())}};
    boost::polygon::voronoi_visual_utils<double>::discretize(point, segment, maxDistMm * kScale, &sampled_edge);

    Geo::Polyline path;
    path.reserve(sampled_edge.size());
    for(const auto& p: sampled_edge)
        path.emplace_back(fromUnits(p.x()), fromUnits(p.y()));
    return path;
}

namespace Voronoi {

// Скелет Вороного МЕЖДУ разными телами меди -- рёбра диаграммы, у которых
// два прилегающих сайта принадлежат разным группам (color1 != color2). Это
// ровно то, что нужно для изоляционной фрезеровки: разрез посередине между
// соседними проводниками.
void VoronoiBoost::boostVoronoi() {
    const double tolerance = gcp.params[Tolerance].toDouble();

    double minX = std::numeric_limits<double>::max(),
           minY = std::numeric_limits<double>::max(),
           maxX = std::numeric_limits<double>::lowest(),
           maxY = std::numeric_limits<double>::lowest();

    int32_t id{};
    setMsg(QObject::tr("Calc BOOST Voronoi"));

    std::vector<Geo::Polylines> bodies;
    bodies.reserve(groupedPss.size());
    size_t max{};
    for(const Geo::Polygon& body: groupedPss) {
        bodies.push_back(body.contours());
        for(const Geo::Polyline& contour: bodies.back())
            max += contour.size();
    }

    max = max * 3 / 2;
    setMax(max);
    setCurrent();
    std::vector<segment_type> srcSegments;
    srcSegments.reserve(max);
    std::vector<int> vecId;
    vecId.reserve(max);

    // Отрезок в диаграмму. Короче 10 нм -- для диаграммы это точка, не
    // отрезок, такой пропускается.
    auto addSegment = [&](QPointF from, QPointF to) {
        const point_type a{toUnits(from.x()), toUnits(from.y())};
        const point_type b{toUnits(to.x()), toUnits(to.y())};
        if(a == b) return;
        vecId.emplace_back(id);
        srcSegments.emplace_back(a, b);
        maxX = std::max(maxX, to.x());
        maxY = std::max(maxY, to.y());
        minX = std::min(minX, to.x());
        minY = std::min(minY, to.y());
    };

    for(const Geo::Polylines& contours: bodies) {
        ++id; // тег -- тело целиком: между внешним контуром и его же дыркой резать нечего
        for(const Geo::Polyline& path: contours) {
            for(auto&& [from, to]: Geo::segments(path)) {
                incCurrent();
                Geo::checkCancelled();
                // Дуга -- хордами: диаграмма понимает только отрезки. Шаг --
                // по стреле прогиба, чтобы круглый пад остался круглым.
                if(const auto arc = Geo::arcOf(from, to, from.bulge)) {
                    const int steps = arcChordSteps(*arc);
                    QPointF prev = from;
                    for(int i = 1; i <= steps; ++i) {
                        const QPointF pt = i == steps ? QPointF{to} : arc->pointAt(static_cast<double>(i) / steps);
                        addSegment(prev, pt);
                        prev = pt;
                    }
                } else
                    addSegment(from, to);
            }
        }
    }

    const double kx = (maxX - minX) * 2;
    const double ky = (maxY - minY) * 2;
    // Огромная охватывающая рамка-сайт: без неё лучи диаграммы на границе
    // области уходят в бесконечность и вершины (vertex0/vertex1) у их рёбер
    // отсутствуют вовсе.
    const point_type fLB{toUnits(minX - kx), toUnits(minY - ky)};
    const point_type fRB{toUnits(maxX + kx), toUnits(minY - ky)};
    const point_type fRT{toUnits(maxX + kx), toUnits(maxY + ky)};
    const point_type fLT{toUnits(minX - kx), toUnits(maxY + ky)};
    vecId.emplace_back(++id);
    srcSegments.emplace_back(fRB, fRT);
    vecId.emplace_back(id);
    srcSegments.emplace_back(fRB, fLB);
    vecId.emplace_back(id);
    srcSegments.emplace_back(fLT, fRT);
    vecId.emplace_back(id);
    srcSegments.emplace_back(fLB, fLT);

    Geo::Polylines segments;
    {
        VD vd;
        construct_voronoi(srcSegments.begin(), srcSegments.end(), &vd);

        auto id0 = [&](auto edge) { return vecId[edge.cell()->source_index()]; };
        auto id1 = [&](auto edge) { return vecId[edge.twin()->cell()->source_index()]; };

        struct EdgeKey {
            double x0, y0, x1, y1;
            auto operator<=>(const EdgeKey&) const = default;
        };
        std::set<EdgeKey> seen;

        for(auto& edge: vd.edges()) {
            auto v0 = edge.vertex0();
            auto v1 = edge.vertex1();

            const int32_t color1 = id0(edge);
            const int32_t color2 = id1(edge);

            if(v0 && v1) {
                const QPointF p0{fromUnits(v0->x()), fromUnits(v0->y())};
                const QPointF p1{fromUnits(v1->x()), fromUnits(v1->y())};
                if(color1 != color2 && color1 && color2) {
                    if(seen.emplace(p0.x(), p0.y(), p1.x(), p1.y()).second && seen.emplace(p1.x(), p1.y(), p0.x(), p0.y()).second) {
                        // Кривое ребро (между точкой и отрезком -- парабола)
                        // дискретизируется всегда: одной хордой оно
                        // выражается лишь когда короче допуска, а это
                        // discretize сделает и сам.
                        if(edge.is_curved() && Geo::distance(p0, p1) >= tolerance)
                            segments.push_back(sample_curved_edge(srcSegments, edge, tolerance));
                        else
                            segments.push_back(Geo::Polyline{Geo::Vertex{p0}, Geo::Vertex{p1}});
                    }
                }
            }
        }
    }

    segments = chainDiagramEdges(std::move(segments));

    // Обход строго против часовой стрелки: Geo::Polygons трактует контур с
    // отрицательной площадью как пустоту, а не как тело (в отличие от
    // Clipper2, которому направление рамки-клипа было всё равно).
    const double fo = gcp.params[FrameOffset].toDouble();
    Geo::Polyline frame{
        Geo::Vertex{minX - fo, minY - fo},
        Geo::Vertex{maxX + fo, minY - fo},
        Geo::Vertex{maxX + fo, maxY + fo},
        Geo::Vertex{minX - fo, maxY + fo},
    };
    frame.closed = true;
    segments = Geo::clipOpen(Geo::ClipType_::Intersection, segments, Geo::Polygons{Geo::Polylines{frame}});

    auto clean = [kAngle = 2.0](Geo::Polyline& path) {
        for(size_t i = 1; i + 1 < path.size(); ++i) {
            const double a1 = Geo::angleTo(path[i - 1], path[i + 0]);
            const double a2 = Geo::angleTo(path[i + 0], path[i + 1]);
            if(std::abs(a1 - a2) < kAngle) {
                path.erase(path.begin() + static_cast<std::ptrdiff_t>(i));
                --i;
            }
        }
    };

    r::for_each(segments, clean);
    r::for_each(segments, clean);

    returnPs = std::move(segments);
    returnPs.push_back(frame);
}

} // namespace Voronoi
#else
namespace Voronoi {
void VoronoiBoost::boostVoronoi() { }
} // namespace Voronoi
#endif
