// Бенчмарк кармана «снаружи» без GUI: синтетическое поле «рамка минус
// сетка падов» гоняется через Geo::InflatePasses в форме fieldLoops из
// pocketoffset -- первый виток точный, последующие черновые. Детерминирован
// нацело (никакого рандома), поэтому годится для A/B: одна и та же
// геометрия до вершины, меняется только время.
//
// Аргументы: [падов-на-сторону = 12] [прогонов = 5]. Первый прогон греет
// кэши и в медиану не входит, если прогонов больше двух.

#include "geo/boolean.h"
#include "geo/inflatepasses.h"
#include "geo/phasestats.h"

#include <QPointF>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <numbers>
#include <vector>

using namespace Geo;

namespace {

Polyline rectangle(double x0, double y0, double x1, double y1) {
    Polyline poly{Vertex(x0, y0), Vertex(x1, y0), Vertex(x1, y1), Vertex(x0, y1)};
    poly.closed = true;
    return poly;
}

// Зубчатое кольцо из test_inflate/test_inflatepasses: мелочь границы, на
// которой прореживание и принятие базы отрабатывают в полную силу.
Polyline toothedRing(double cx, double cy, double radius, double tooth, int teeth = 24) {
    Polyline ring;
    ring.closed = true;
    for(int k = 0; k < teeth; ++k) {
        const double a = 2.0 * std::numbers::pi * k / teeth;
        const double r = radius + (k % 2 ? tooth : -tooth);
        ring.emplace_back(QPointF{cx + r * std::cos(a), cy + r * std::sin(a)}, 0.0);
    }
    return ring;
}

// Копия coarseTolerance из pocketoffset.cpp: бенч обязан идти тем же
// расписанием допусков, что и настоящий карман.
double coarseTolerance(double stepOver, double toolDiameter) {
    const double tolerance = 0.25 * std::min(stepOver, toolDiameter - stepOver);
    return tolerance < exitWeldTolerance * 2.0 ? 0.0 : tolerance;
}

std::size_t verticesOf(const Polylines& contours) {
    std::size_t total{};
    for(const Polyline& contour: contours) total += contour.size();
    return total;
}

// Поле платы: рамка минус чередующиеся пады (кольца и квадраты) с редкими
// перемычками, чтобы после вычитания оставались и большие связные кляксы,
// и одиночные мелкие тела -- оба пути passParts (bigPool и parallelFor).
Polygons makeField(int padsPerSide) {
    const double pitch = 4.0;
    // Рамка с широкими полями вокруг сетки: узкие каналы между падами
    // заполняются за два-три витка, а внешняя площадь даёт длинный хвост
    // витков -- как флуд вокруг платы у настоящего кармана «снаружи».
    const double size = padsPerSide * pitch + 24.0;
    const Polyline frame = rectangle(-size / 2, -size / 2, size / 2, size / 2);

    Polylines copper;
    const double origin = -(padsPerSide - 1) * pitch / 2;
    for(int j = 0; j < padsPerSide; ++j)
        for(int i = 0; i < padsPerSide; ++i) {
            const double cx = origin + i * pitch, cy = origin + j * pitch;
            if((i + j) % 2)
                copper.push_back(toothedRing(cx, cy, 1.3, 0.12));
            else
                copper.push_back(rectangle(cx - 1.1, cy - 1.1, cx + 1.1, cy + 1.1));
            // Перемычка к соседу справа -- сшивает пады в кластеры.
            if(i + 1 < padsPerSide && j % 3 == 0)
                copper.push_back(rectangle(cx, cy - 0.25, cx + pitch, cy + 0.25));
        }

    return Polygons{Polylines{frame}} - Polygons{copper};
}

struct PassRow {
    double ms;
    std::size_t parts, loops, points;
};

// Один полный карман: движок создаётся заново, витки -- как в fieldLoops.
std::vector<PassRow> runPocket(const Polygons& field, double start, double step, double coarse) {
    InflatePasses passes;
    for(const Polygon& body: field.all()) passes.addField(body);

    std::vector<PassRow> rows;
    for(int i = 0;; ++i) {
        const auto t0 = std::chrono::steady_clock::now();
        const std::vector<Polygons> parts = passes.passParts(start + step * i, i ? coarse : 0.0);
        std::size_t loops{}, points{};
        for(const Polygons& part: parts) {
            const Polylines contours = part.contours();
            loops += contours.size();
            points += verticesOf(contours);
        }
        const std::chrono::duration<double, std::milli> spent = std::chrono::steady_clock::now() - t0;
        if(!loops) break;
        rows.push_back({spent.count(), parts.size(), loops, points});
    }
    return rows;
}

} // namespace

int main(int argc, char* argv[]) {
    const int padsPerSide = argc > 1 ? std::atoi(argv[1]) : 12;
    const int runs = std::max(argc > 2 ? std::atoi(argv[2]) : 5, 1);

    const double toolDiameter = 1.0;
    const double start = toolDiameter / 2;       // dOffset
    const double step = toolDiameter * 0.45;     // stepOver
    const double coarse = coarseTolerance(step, toolDiameter);

    const Polygons field = makeField(padsPerSide);
    std::printf("field: %zu bodies, pads %dx%d, tool %.2f, step %.3f, coarse %.3f\n",
        field.all().size(), padsPerSide, padsPerSide, toolDiameter, step, coarse);

    std::vector<double> totals;
    for(int r = 0; r < runs; ++r) {
        const std::vector<PassRow> rows = runPocket(field, start, step, coarse);
        double total{};
        std::size_t loops{}, points{};
        for(std::size_t i = 0; i < rows.size(); ++i) {
            total += rows[i].ms;
            loops += rows[i].loops;
            points += rows[i].points;
            if(!r)
                std::printf("  pass %2zu: %8.2f ms, %4zu parts, %5zu loops, %7zu pts\n",
                    i, rows[i].ms, rows[i].parts, rows[i].loops, rows[i].points);
        }
        totals.push_back(total);
        std::printf("run %d: %.1f ms, %zu passes, %zu loops, %zu pts\n",
            r, total, rows.size(), loops, points);
        std::printf("%s", phaseReport().c_str());
        std::fflush(stdout);
    }

    // Медиана по прогонам без прогревочного нулевого (если есть из чего).
    std::vector<double> timed(totals.begin() + (totals.size() > 2 ? 1 : 0), totals.end());
    std::ranges::sort(timed);
    std::printf("median: %.1f ms\n", timed[timed.size() / 2]);
    return 0;
}
