#pragma once

// Разложение офсета в сумму Минковского с кругом.
//
// CGAL умеет точные булевы операции над дуговыми полигонами, но не умеет
// офсетить их напрямую. Зато офсет фигуры на d -- это в точности её сумма
// Минковского с кругом радиуса d, а такая сумма для полилинии собирается
// из элементарных кусков: тело каждого ребра (прямоугольник для прямого,
// кольцевой сектор для дугового) плюс диск вокруг каждой вершины. Их
// объединение и есть раздутая фигура -- уже точным движком.
//
// Куски намеренно строятся БЕЗ полуокружных колпачков на торцах рёбер:
// скругления дают отдельные ПОЛНЫЕ диски вокруг вершин, и тогда у общей
// вершины двух рёбер обе стороны получают буквально одну и ту же окружность
// (диск задан лишь центром и радиусом), а не две «почти одинаковые» дуги,
// построенные из разных троек точек, чей микрозигзаг потом ломает
// объединение.
//
// Все дуги результата лежат ТОЧНО на окружностях R0 +- d вокруг исходных
// центров дуг и на окружностях радиуса d вокруг исходных вершин.
//
// Здесь только Qt/Geo-типы -- никаких зависимостей от геометрических
// библиотек.

#include "geo/polyline.h"

#include <cmath>
#include <numbers>
#include <optional>
#include <utility>
#include <vector>

namespace Geo::Offset {

// Замкнутый контур против часовой стрелки (канон для тела).
inline Polyline ccw(Polyline poly) {
    poly.closed = true;
    if(poly.size() > 1 && poly.signedArea() < 0.0) poly.reverse();
    return poly;
}

// Полный круг радиуса d вокруг точки -- скругление вершины, а заодно и
// цельный офсет исходной окружности.
inline Polyline disc(QPointF center, double d) {
    Polyline out;
    out.closed = true;
    out.emplace_back(QPointF(center.x() - d, center.y()), 1.0);
    out.emplace_back(QPointF(center.x() + d, center.y()), 1.0);
    return out;
}

// Замкнутая полилиния из двух вершин с одинаковым прогибом +-1 -- полная
// окружность; возвращает её центр и радиус. Офсет такой полилинии -- сразу
// один диск радиуса R0 + d вместо пары полусекторов, чьи точно совпадающие
// радиальные торцы -- худший случай для любого булева движка.
inline std::optional<std::pair<QPointF, double>> fullCircleOf(const Polyline& poly) {
    if(!poly.closed || poly.size() != 2) return std::nullopt;
    const double b = poly[0].bulge;
    if(poly[1].bulge != b || std::abs(std::abs(b) - 1.0) > 1e-12) return std::nullopt;
    const QPointF center((poly[0].x() + poly[1].x()) / 2.0, (poly[0].y() + poly[1].y()) / 2.0);
    const double radius = std::hypot(poly[1].x() - poly[0].x(), poly[1].y() - poly[0].y()) / 2.0;
    if(radius <= 0.0) return std::nullopt;
    return std::pair{center, radius};
}

// Тело одного ребра (from -> to, прогиб у from), раздутое на d. Торцы --
// прямые; скругления концов дают диски вершин, см. capsulesFor(poly, d).
inline Polylines capsulesFor(const Vertex& from, QPointF to, double d) {
    Polylines out;
    const double dx    = to.x() - from.x();
    const double dy    = to.y() - from.y();
    const double chord = std::hypot(dx, dy);

    if(chord < 1e-12) return out; // вырожденное ребро: хватит дисков вершин

    if(from.bulge == 0.0) {
        // Прямое ребро -- прямоугольник (против часовой стрелки: первая
        // сторона справа по ходу).
        const QPointF nR(dy / chord * d, -dx / chord * d); // правая нормаль * d
        Polyline quad;
        quad.closed = true;
        quad.emplace_back(QPointF(from) + nR, 0.0);
        quad.emplace_back(to + nR, 0.0);
        quad.emplace_back(to - nR, 0.0);
        quad.emplace_back(QPointF(from) - nR, 0.0);
        out.push_back(std::move(quad));
        return out;
    }

    // Дуговое ребро -- кольцевой сектор между R0 - d и R0 + d.
    const double theta   = 4.0 * std::atan(from.bulge);
    const double sagitta = from.bulge * chord / 2.0;
    const double radius  = chord / (2.0 * std::abs(std::sin(theta / 2.0)));
    const QPointF n(dy / chord, -dx / chord);
    const QPointF mid((QPointF(from) + to) / 2.0);
    const QPointF center = mid + n * (sagitta - std::copysign(radius, from.bulge));

    if(radius - d < 1e-9) {
        // Внутренняя окружность вырождается: режем дугу на короткие хорды и
        // раздуваем каждую прямым ребром; диски промежуточных стыков --
        // явно, их больше некому добавить.
        const int pieces = std::max(2, static_cast<int>(std::ceil(std::abs(theta) / 0.3)));
        const double a0  = std::atan2(from.y() - center.y(), from.x() - center.x());
        QPointF prev{from};
        for(int i = 1; i <= pieces; ++i) {
            const double a  = a0 + theta * i / pieces;
            const QPointF p = i == pieces
                ? to
                : QPointF(center.x() + radius * std::cos(a), center.y() + radius * std::sin(a));
            for(auto& s: capsulesFor(Vertex(prev, 0.0), p, d)) out.push_back(std::move(s));
            if(i != pieces) out.push_back(disc(p, d));
            prev = p;
        }
        return out;
    }

    const double a0 = std::atan2(from.y() - center.y(), from.x() - center.x());
    const double a1 = a0 + theta;
    auto at         = [&](double a, double r) {
        return QPointF(center.x() + r * std::cos(a), center.y() + r * std::sin(a));
    };

    Polyline sector;
    sector.closed = true;
    sector.emplace_back(at(a0, radius + d), std::tan(theta / 4.0));  // внешняя дуга (тот же размах)
    sector.emplace_back(at(a1, radius + d), 0.0);                    // торец у to
    sector.emplace_back(at(a1, radius - d), std::tan(-theta / 4.0)); // внутренняя дуга (обратно)
    sector.emplace_back(at(a0, radius - d), 0.0);                    // торец у from
    out.push_back(ccw(std::move(sector)));
    return out;
}

// Все куски офсета одной полилинии: тела рёбер плюс диск вокруг КАЖДОЙ
// вершины. Сам полигон замкнутой полилинии вызывающий добавляет отдельно.
inline Polylines capsulesFor(const Polyline& poly, double d) {
    Polylines out;
    if(poly.empty()) return out;
    for(const Vertex& v: poly) out.push_back(disc(v, d));
    for(std::size_t i = 0; i + 1 < poly.size(); ++i)
        for(auto& c: capsulesFor(poly[i], poly[i + 1], d)) out.push_back(std::move(c));
    if(poly.closed && poly.size() > 1)
        for(auto& c: capsulesFor(poly.back(), poly.front(), d)) out.push_back(std::move(c));
    return out;
}

} // namespace Geo::Offset
