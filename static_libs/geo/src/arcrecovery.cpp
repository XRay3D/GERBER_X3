#include "geo/arcrecovery.h"
#include "geo/cancel.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numbers>
#include <optional>

namespace Geo::ArcRecovery {

namespace {

constexpr double pi = std::numbers::pi;

constexpr double toRad(double degrees) { return degrees * pi / 180.0; }

// Разность направлений, приведённая к (-pi, pi].
double normalized(double angle) { return std::remainder(angle, 2.0 * pi); }

// Прогиб считается нулевым (сегмент -- отрезок) с тем же допуском, с каким
// вся остальная геометрия считает точки совпавшими.
constexpr double straightBulge = 1e-12;

struct Circle {
    QPointF center;
    double radius;
};

// Полилиния, разобранная на сегменты: у замкнутой последний ведёт из
// последней вершины в первую.
struct Segment {
    QPointF from, to;
    double length;
    double direction;
    bool straight;
};

std::vector<Segment> segmentsOf(const Polyline& poly) {
    std::vector<Segment> out;
    const std::size_t n = poly.size();
    const std::size_t count = poly.closed ? n : n - 1;
    out.reserve(count);
    for(std::size_t i = 0; i < count; ++i) {
        const QPointF from = poly[i], to = poly[(i + 1) % n];
        const double dx = to.x() - from.x(), dy = to.y() - from.y();
        out.push_back({from, to, std::hypot(dx, dy), std::atan2(dy, dx),
            std::abs(poly[i].bulge) < straightBulge});
    }
    return out;
}

// Поворот на стыке двух сегментов: насколько второй отклоняется от первого.
// Угол МЕЖДУ отрезками (тот, что мерят на чертеже) -- это 180 минус он.
double turnBetween(const Segment& before, const Segment& after) {
    return normalized(after.direction - before.direction);
}

bool sameLength(const Segment& a, const Segment& b, const Settings& settings) {
    const double longer = std::max(a.length, b.length);
    return longer > 0.0 && std::abs(a.length - b.length) <= settings.lengthTolerance * longer;
}

// Годится ли такой поворот в дискретизацию: не шум округления, не слишком
// крупный шаг и не угол ходовой фигуры.
bool turnUsable(double turn, const Settings& settings) {
    const double degrees = std::abs(turn) * 180.0 / pi;
    if(degrees < settings.minTurnDegrees || degrees > settings.maxTurnDegrees) return false;
    const double between = 180.0 - degrees;
    for(const double keep: settings.keepAngles)
        if(std::abs(between - keep) <= settings.keepAngleTolerance) return false;
    return true;
}

// Продолжает ли сегмент предыдущий как звено одной и той же дискретизации.
bool continues(const Segment& before, const Segment& after, const Settings& settings) {
    return before.straight && after.straight && sameLength(before, after, settings)
        && turnUsable(turnBetween(before, after), settings);
}

// Опорная окружность цепочки -- методом наименьших квадратов (алгебраичес-
// кая подгонка Касы: минимизируется невязка x^2 + y^2 = a x + b y + c,
// линейная по a, b, c). Регулярность саму окружность уже задаёт -- R = L /
// (2 sin(поворот / 2)) -- но центр, собранный по одним лишь хордам, слегка
// смещён округлением координат, а разница идёт прямо в допуск, по которому
// потом судят, лежат ли вершины на окружности.
//
// Координаты сдвинуты к центру тяжести: тогда суммы первых степеней равны
// нулю и система сворачивается с 3x3 до 2x2, заодно не теряя точности на
// чертеже, отодвинутом далеко от начала координат.
std::optional<Circle> circleOf(const std::vector<Segment>& segments, std::size_t first, std::size_t last) {
    std::vector<QPointF> points;
    points.reserve(last - first + 2);
    for(std::size_t i = first; i <= last; ++i) points.push_back(segments[i].from);
    points.push_back(segments[last].to);
    if(points.size() < 3) return std::nullopt;

    QPointF mean;
    for(const QPointF& point: points) mean += point;
    mean /= static_cast<double>(points.size());

    double suu = 0.0, svv = 0.0, suv = 0.0, su3 = 0.0, sv3 = 0.0, sq = 0.0;
    for(const QPointF& point: points) {
        const double u = point.x() - mean.x(), v = point.y() - mean.y();
        const double square = u * u + v * v;
        suu += u * u;
        svv += v * v;
        suv += u * v;
        su3 += u * square;
        sv3 += v * square;
        sq += square;
    }

    const double det = suu * svv - suv * suv;
    if(std::abs(det) <= 0.0) return std::nullopt; // точки на одной прямой

    const double a = (su3 * svv - sv3 * suv) / det;
    const double b = (sv3 * suu - su3 * suv) / det;
    const double c = sq / static_cast<double>(points.size());

    const double squared = c + (a * a + b * b) / 4.0;
    if(squared <= 0.0) return std::nullopt;
    return Circle{mean + QPointF(a, b) / 2.0, std::sqrt(squared)};
}

bool onCircle(const Circle& circle, QPointF point, const Settings& settings) {
    const double distance = std::hypot(point.x() - circle.center.x(), point.y() - circle.center.y());
    return std::abs(distance - circle.radius) <= settings.radialTolerance;
}

// Лежат ли на окружности все вершины цепочки -- ГЛАВНАЯ проверка. Равные
// длины и равные углы окружность лишь задают; то, что фигура ей и следует,
// известно только отсюда.
bool fitsCircle(const Circle& circle, const std::vector<Segment>& segments, std::size_t first,
    std::size_t last, const Settings& settings) {
    if(circle.radius <= 0.0) return false;
    for(std::size_t i = first; i <= last; ++i)
        if(!onCircle(circle, segments[i].from, settings)) return false;
    return onCircle(circle, segments[last].to, settings);
}

double angleAt(const Circle& circle, QPointF point) {
    return std::atan2(point.y() - circle.center.y(), point.x() - circle.center.x());
}

// Полный размах цепочки вокруг центра: шаги складываются со знаком обхода,
// поэтому размах свободно переваливает и за 180 градусов.
double sweepOf(const Circle& circle, const std::vector<Segment>& segments, std::size_t first,
    std::size_t last, double side) {
    double total = 0.0;
    double previous = angleAt(circle, segments[first].from);
    for(std::size_t i = first; i <= last; ++i) {
        const double current = angleAt(circle, segments[i].to);
        double step = normalized(current - previous);
        if(side > 0.0 && step < 0.0) step += 2.0 * pi;
        if(side < 0.0 && step > 0.0) step -= 2.0 * pi;
        total += step;
        previous = current;
    }
    return total;
}

// Окружность в каноничном для проекта виде: две вершины с прогибом +-1 --
// ровно так её отдаёт парсер для CIRCLE и так её узнаёт Offset::fullCircleOf.
Polyline asCircle(const Circle& circle, double side, double width) {
    Polyline out;
    out.closed = true;
    out.width = width;
    out.emplace_back(QPointF(circle.center.x() + circle.radius, circle.center.y()), side);
    out.emplace_back(QPointF(circle.center.x() - circle.radius, circle.center.y()), side);
    return out;
}

// Замена: вершины [first .. last] уходят, вместо них встают эти. Вершина
// last + 1 -- конец дуги -- остаётся на своём месте нетронутой.
struct Replacement {
    std::size_t first, last;
    std::vector<Vertex> vertices;
};

// Дуга размахом почти в полный круг непредставима одной вершиной: прогиб
// tan(размах / 4) уходит в бесконечность. Такую делим пополам.
constexpr double maxSingleBulgeSweep = toRad(300.0);

std::vector<Vertex> arcVertices(const Circle& circle, QPointF start, double sweep) {
    if(std::abs(sweep) <= maxSingleBulgeSweep) return {Vertex(start, std::tan(sweep / 4.0))};

    const double half = sweep / 2.0;
    const double middle = angleAt(circle, start) + half;
    const QPointF split(circle.center.x() + circle.radius * std::cos(middle),
        circle.center.y() + circle.radius * std::sin(middle));
    return {
        Vertex(start, std::tan(half / 4.0)),
        Vertex(split, std::tan(half / 4.0)),
    };
}

} // namespace

Polyline recover(const Polyline& poly, const Settings& settings) {
    if(!settings.enabled) return poly;

    const std::size_t n = poly.size();
    const std::size_t count = poly.closed ? n : (n ? n - 1 : 0);
    if(count < std::max<std::size_t>(settings.minSegments, 2)) return poly;

    Polyline work = poly;
    std::vector<Segment> segments = segmentsOf(work);

    if(work.closed) {
        // Стык замкнутой полилинии может прийтись на середину дуги -- тогда
        // линейный проход увидел бы два огрызка вместо одной цепочки. К
        // нумерации замкнутый контур безразличен, так что проще прокрутить
        // его до ближайшего разрыва регулярности.
        std::size_t start = count;
        for(std::size_t i = 0; i < count; ++i)
            if(!continues(segments[(i + count - 1) % count], segments[i], settings)) {
                start = i;
                break;
            }

        if(start == count) {
            // Разрывов нет вовсе: контур регулярен целиком. Это окружность,
            // если он ещё и обошёл полный круг.
            const double turn = turnBetween(segments[count - 1], segments[0]);
            const std::optional<Circle> circle = circleOf(segments, 0, count - 1);
            if(!circle || !fitsCircle(*circle, segments, 0, count - 1, settings)) return poly;
            const double side = turn > 0.0 ? 1.0 : -1.0;
            const double sweep = sweepOf(*circle, segments, 0, count - 1, side);
            if(std::abs(std::abs(sweep) - 2.0 * pi) > toRad(settings.fullCircleTolerance)) return poly;
            return asCircle(*circle, side, work.width);
        }

        if(start != 0) {
            std::rotate(work.begin(), work.begin() + start, work.end());
            segments = segmentsOf(work);
        }
    }

    std::vector<Replacement> replacements;
    std::size_t barrier = 0; // докуда уже разобрано: левее расширяться нельзя

    for(std::size_t i = 0; i + settings.minSegments <= count;) {
        if(!segments[i].straight) {
            ++i;
            continue;
        }

        // Регулярное ядро цепочки: длины сверяются с первым отрезком, а не
        // с соседним, иначе допуск накапливался бы вдоль всей цепочки.
        std::size_t last = i;
        double turn = 0.0;
        while(last + 1 < count && segments[last + 1].straight
            && sameLength(segments[i], segments[last + 1], settings)) {
            const double next = turnBetween(segments[last], segments[last + 1]);
            if(!turnUsable(next, settings)) break;
            if(last == i)
                turn = next;
            else if(std::abs(normalized(next - turn)) > toRad(settings.angleTolerance))
                break;
            ++last;
        }
        if(last - i + 1 < settings.minSegments) {
            ++i;
            continue;
        }

        const std::optional<Circle> circle = circleOf(segments, i, last);
        if(!circle || !fitsCircle(*circle, segments, i, last, settings)) {
            ++i;
            continue;
        }

        // Концы дуги экспортёр режет не по шагу: у крайних отрезков и длина
        // другая, и угол при них свой -- регулярности там нет и быть не
        // может. Зато у них можно спросить напрямую, лежат ли их точки на
        // уже найденной окружности, и не длиннее ли они шага (длинная хорда
        // с концами на окружности -- это уже не дискретизация, а срез).
        const double side = turn > 0.0 ? 1.0 : -1.0;
        auto reaches = [&](const Segment& outer, double turnAtJoint, QPointF tip) {
            return outer.straight
                && outer.length <= segments[i].length * (1.0 + settings.lengthTolerance)
                && turnAtJoint * side > 0.0 && onCircle(*circle, tip, settings);
        };

        std::size_t first = i;
        while(first > barrier
            && reaches(segments[first - 1], turnBetween(segments[first - 1], segments[first]),
                segments[first - 1].from))
            --first;
        while(last + 1 < count
            && reaches(segments[last + 1], turnBetween(segments[last], segments[last + 1]),
                segments[last + 1].to))
            ++last;

        const double sweep = sweepOf(*circle, segments, first, last, side);
        if(std::abs(sweep) > 2.0 * pi + toRad(settings.fullCircleTolerance)) {
            ++i;
            continue;
        }

        // Расширение концов могло замкнуть круг целиком -- тогда это всё та
        // же окружность, просто с одним нерегулярным отрезком.
        if(work.closed && first == 0 && last + 1 == count
            && std::abs(std::abs(sweep) - 2.0 * pi) <= toRad(settings.fullCircleTolerance))
            return asCircle(*circle, side, work.width);

        replacements.push_back({first, last, arcVertices(*circle, segments[first].from, sweep)});
        barrier = last + 1;
        i = last + 1;
    }

    if(replacements.empty()) return poly;

    Polyline out;
    out.closed = work.closed;
    out.width = work.width;
    out.reserve(work.size());
    auto replacement = replacements.begin();
    for(std::size_t v = 0; v < n;) {
        if(replacement != replacements.end() && replacement->first == v) {
            for(const Vertex& vertex: replacement->vertices) out.push_back(vertex);
            v = replacement->last + 1; // конец дуги -- уже обычная вершина
            ++replacement;
            continue;
        }
        out.push_back(work[v]);
        ++v;
    }
    return out;
}

void stitch(Polylines& polylines, const Settings& settings) {
    const double tolerance = settings.stitchTolerance;

    // Концы всех открытых контуров: чётный номер -- начало контура,
    // нечётный -- его конец.
    const auto owner = [](std::size_t end) { return end / 2; };
    const auto isTail = [](std::size_t end) { return end % 2 == 1; };
    const auto joinable = [](const Polyline& poly) { return !poly.closed && poly.size() >= 2; };

    auto pointOf = [&](std::size_t end) -> QPointF {
        const Polyline& poly = polylines[owner(end)];
        return isTail(end) ? poly.back() : poly.front();
    };

    // Раскладка концов по ячейкам сетки с шагом в допуск: партнёр точки --
    // всегда в её собственной ячейке или в одной из восьми соседних.
    using Cell = std::pair<long long, long long>;
    const auto cellOf = [&](QPointF point) {
        return Cell{static_cast<long long>(std::floor(point.x() / tolerance)),
            static_cast<long long>(std::floor(point.y() / tolerance))};
    };
    std::map<Cell, std::vector<std::size_t>> grid;
    for(std::size_t i = 0; i < polylines.size(); ++i) {
        if(!joinable(polylines[i])) continue;
        grid[cellOf(polylines[i].front())].push_back(2 * i);
        grid[cellOf(polylines[i].back())].push_back(2 * i + 1);
    }

    // Сшивается только однозначное: ровно один партнёр с обеих сторон и
    // одинаковая ширина линии. Где сходятся три конца -- развилка, и
    // выбирать за пользователя, куда пойти, здесь нечем.
    constexpr std::size_t none = static_cast<std::size_t>(-1);
    std::vector<std::size_t> partner(2 * polylines.size(), none);
    for(std::size_t i = 0; i < polylines.size(); ++i) {
        if(!joinable(polylines[i])) continue;
        for(std::size_t end: {2 * i, 2 * i + 1}) {
            const QPointF point = pointOf(end);
            const Cell cell = cellOf(point);
            std::size_t found = none;
            std::size_t matches = 0;
            for(long long dx = -1; dx <= 1; ++dx)
                for(long long dy = -1; dy <= 1; ++dy) {
                    const auto neighbours = grid.find(Cell{cell.first + dx, cell.second + dy});
                    if(neighbours == grid.end()) continue;
                    for(const std::size_t other: neighbours->second) {
                        if(other == end) continue;
                        const QPointF at = pointOf(other);
                        if(std::hypot(at.x() - point.x(), at.y() - point.y()) > tolerance) continue;
                        ++matches;
                        found = other;
                    }
                }
            if(matches == 1 && polylines[owner(found)].width == polylines[i].width) partner[end] = found;
        }
    }
    for(std::size_t end = 0; end < partner.size(); ++end) // взаимность
        if(partner[end] != none && partner[partner[end]] != end) partner[end] = none;

    // Цепочка ведётся от свободного конца; куски, у которых свободных
    // концов нет вовсе, разбираются вторым проходом -- это кольца.
    //
    // Кольцо НЕ помечается замкнутым, хотя и вернулось в свою начальную
    // точку. Замкнутость здесь -- не про геометрию, а про смысл: замкнутый
    // контур в этом проекте задаёт область, причём ориентация решает, тело
    // это или пустота в чьём-то теле (см. Geo::Polygons). У россыпи же
    // отдельных отрезков никакого смысла на этот счёт нет, а порядок, в
    // котором экспортёр их записал, задал бы обход как попало -- половина
    // колец шелкографии тогда вырезала бы медь вместо того, чтобы просто
    // быть нарисованной.
    std::vector<char> used(polylines.size(), 0);
    Polylines out;
    auto follow = [&](std::size_t start) {
        Polyline chain;
        chain.width = polylines[owner(start)].width;
        for(std::size_t end = start; end != none;) {
            Polyline piece = polylines[owner(end)];
            if(isTail(end)) piece.reverse(); // входим с хвоста -- разворачиваем
            used[owner(end)] = 1;

            if(chain.empty())
                chain = piece;
            else {
                // Стыковая вершина уже в цепочке -- у неё меняется только
                // прогиб: он описывает СЛЕДУЮЩИЙ сегмент, а тот пришёл с
                // новым куском.
                chain.back().bulge = piece.front().bulge;
                chain.insert(chain.end(), piece.begin() + 1, piece.end());
            }

            const std::size_t exit = isTail(end) ? 2 * owner(end) : 2 * owner(end) + 1;
            const std::size_t next = partner[exit];
            if(next == none || used[owner(next)]) break;
            end = next;
        }
        return chain;
    };

    for(std::size_t i = 0; i < polylines.size(); ++i) {
        checkCancelled();
        if(used[i] || !joinable(polylines[i])) continue;
        if(partner[2 * i] == none)
            out.push_back(follow(2 * i));
        else if(partner[2 * i + 1] == none)
            out.push_back(follow(2 * i + 1));
    }
    for(std::size_t i = 0; i < polylines.size(); ++i) { // кольца без свободных концов
        if(used[i] || !joinable(polylines[i])) continue;
        out.push_back(follow(2 * i));
    }
    for(std::size_t i = 0; i < polylines.size(); ++i) // всё, что сшивать было нечем
        if(!used[i] && !joinable(polylines[i])) out.push_back(std::move(polylines[i]));

    polylines = std::move(out);
}

void recover(Polylines& polylines, const Settings& settings) {
    if(!settings.enabled) return;
    if(settings.stitch) stitch(polylines, settings);
    for(Polyline& poly: polylines) {
        // Полилиния -- естественная единица работы: разбор одной от другой
        // не зависит, так что прерваться между ними безопасно. Отмена
        // выбрасывает наружу недоделанный список, но им никто и не
        // воспользуется -- Cancelled летит до самого вызывающего.
        checkCancelled();
        Polyline arced = recover(poly, settings);
        // Откат замены, которую вызывающий принять не может (см. accept в
        // Settings) -- но только если исходный контур он принимал: чинить
        // то, что было сломано и до нас, здесь не нанимались.
        if(settings.accept && poly.closed && !settings.accept(arced) && settings.accept(poly)) continue;
        poly = std::move(arced);
    }
}

} // namespace Geo::ArcRecovery
