// Змейка (Geo::zigzagFill) -- растровое заполнение региона, собранное из
// clipOpen: строки -- открытая змейка ∩ регион, куски границы между
// строками -- контуры региона, разрезанные ГРЕБЁНКОЙ (замкнутая змейка как
// площадь), сшивка -- жадная, направленная, по совпадению концов в пределах
// сварочного допуска.

#include "geo/fill.h"

#include "geo/boolean.h"
#include "geo/cancel.h"
#include "geo/util.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <optional>

namespace Geo {

namespace {

// Змейка по габариту: строки через step, соединители за левым/правым краем
// (запас 1 мм), чтобы в регион попадали только горизонтальные куски.
std::optional<Polyline> calcZigzag(QRectF rect, double step) {
    const double o = 1.0 - std::fmod(rect.height(), step) * 0.5; // центровка строк
    rect.adjust(-1.0, -o, +1.0, +o);
    Polyline zigzag;
    bool fl{};
    for(double y = rect.top(); y <= rect.bottom() || fl; fl = !fl, y += step) {
        if(!fl) {
            zigzag.emplace_back(rect.left(), y);
            zigzag.emplace_back(rect.right(), y);
        } else {
            zigzag.emplace_back(rect.right(), y);
            zigzag.emplace_back(rect.left(), y);
        }
    }
    if(zigzag.empty()) return std::nullopt;
    // Замыкающее ребро гребёнки должно пройти ещё левее соединителей.
    zigzag.front().rx() -= step;
    zigzag.back().rx() -= step;
    return zigzag;
}

// Строки: змейка ∩ регион. Куски одной строки наследуют y своего сегмента
// змейки ТОЧНО (интерполяция горизонтали не трогает y), поэтому группировка
// строк по равенству y законна и в double. Направление строк чередуется --
// боустрофедон.
Polylines calcScanLines(const Polygons& region, const Polyline& zigzag) {
    Polylines sl = clipOpen(BooleanOp_::Intersection, {zigzag}, region);
    if(sl.empty()) return sl;

    std::ranges::sort(sl, {}, [](const Polyline& p) { return p.front().y(); }); // vertical sort

    double start = sl.front().front().y();
    bool fl = {};
    for(size_t i{}, last{}; i < sl.size(); ++i) {
        if(auto y = sl[i].front().y(); y != start || i - 1 == sl.size()) {

            fl ? std::ranges::sort(sl.begin() + last, sl.begin() + i, {}, [](const Polyline& p) { return p.front().x(); }) :           // horizontal sort
                std::ranges::sort(sl.begin() + last, sl.begin() + i, std::greater(), [](const Polyline& p) { return p.front().x(); }); // horizontal sort

            for(size_t k = last; k < i; ++k) // fix direction
                if(fl ^ (sl[k].front().x() < sl[k].back().x()))
                    sl[k].reverse();

            start = y;
            fl = !fl;
            last = i;
        }
    }
    return sl;
}

// Куски границы региона между соседними строками: контуры, разрезанные
// гребёнкой с ОБЕИХ сторон (внутри неё и вне) -- то есть на каждом
// пересечении со строкой. Направление -- снизу вверх.
Polylines calcFrames(const Polylines& contours, const Polygons& comb) {
    Polylines frames = clipOpen(BooleanOp_::Intersection, contours, comb);
    frames.append_range(clipOpen(BooleanOp_::Difference, contours, comb));

    std::ranges::sort(frames, {}, [](const Polyline& p) { return p.front().y(); }); // vertical sort
    for(auto& path: frames)
        if(path.front().y() > path.back().y())
            path.reverse(); // fix vertical direction
    return frames;
}

// Точки совпадают в пределах сварочного допуска: точки разреза считаются в
// double, и куски строки и границы сходятся не бит-в-бит, как это было на
// целочисленной сетке клиппера, а с погрешностью ~1e-12.
bool joined(const QPointF& a, const QPointF& b) {
    return distance(a, b) <= exitWeldTolerance;
}

// Жадная НАПРАВЛЕННАЯ сшивка (в отличие от Geo::stitch куски не
// переворачиваются -- направление строк уже выставлено): к хвосту пути
// клеится следующая строка, между ними -- восходящий кусок границы; к началу
// пути -- нисходящий. Всё, что не склеилось, остаётся отдельным проходом.
Polylines merge(Polylines&& scanLines, Polylines&& frames) {
    Polylines merged;
    merged.reserve(scanLines.size() / 10);
    std::list<Polyline> bList;
    for(auto&& path: scanLines)
        bList.emplace_back(std::move(path));

    std::list<Polyline> fList;
    for(auto&& path: frames)
        fList.emplace_back(std::move(path));

    while(bList.begin() != bList.end()) {
        merged.resize(merged.size() + 1);
        auto& path = merged.back();
        for(auto bit = bList.begin(); bit != bList.end(); ++bit) {
            checkCancelled();
            if(path.empty() || joined(path.back(), bit->front())) {
                path.empty() ? path.append_range(*bit)
                             : path.append_range(*bit | std::views::drop(1));
                bList.erase(bit);
                for(auto fit = fList.begin(); fit != fList.end(); ++fit) {
                    if(joined(path.back(), fit->front()) && fit->front().y() < fit->at(1).y()) {
                        path.append_range(*fit | std::views::drop(1));
                        fList.erase(fit);
                        bit = bList.begin();
                        break;
                    }
                }
                bit = bList.begin();
            }
            if(bList.begin() == bList.end())
                break;
        }
        for(auto fit = fList.begin(); fit != fList.end(); ++fit) {
            if(joined(path.front(), fit->back()) && fit->front().y() > fit->at(1).y()) {
                fit->append_range(path | std::views::drop(1));
                std::swap(*fit, path);
                fList.erase(fit);
                break;
            }
        }
    }
    merged.shrink_to_fit();
    return merged;
}

} // namespace

Polylines zigzagFill(const Polygons& region, double step, double angle) {
    if(region.empty() || step <= 0.0) return {};

    // Регион подставляется ПОД горизонтальную змейку: поворачиваются его
    // контуры (bulge-вид поворот переживает точно по форме), а регион для
    // резки пересобирается из них.
    Polylines contours = region.contours();
    if(!qFuzzyIsNull(angle)) rotate(contours, angle);
    const Polygons rotated{contours};

    const auto zigzag = calcZigzag(rotated.boundingRect(), step);
    if(!zigzag) return {};

    Polyline combContour{*zigzag};
    combContour.closed = true;
    if(combContour.signedArea() < 0) combContour.reverse();
    const Polygons comb{Polylines{std::move(combContour)}};

    Polylines scanLines = calcScanLines(rotated, *zigzag);
    Polylines frames = calcFrames(contours, comb);
    if(scanLines.empty() || frames.empty()) return {};

    Polylines merged = merge(std::move(scanLines), std::move(frames));
    if(!qFuzzyIsNull(angle)) rotate(merged, -angle);
    return merged;
}

} // namespace Geo
