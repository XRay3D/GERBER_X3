// Клиппинг путей регионом (Geo::clipOpen) -- примитив AddOpenSubject из
// Clipper2, пересобранный в bulge-домене.
//
// Точной геометрии здесь нет НАМЕРЕННО. Результат -- траектория: он идёт на
// экран и в G-код и в булевы цепочки не возвращается, поэтому точки разреза
// достаточно считать в double (погрешность ~1e-12 при сварочном допуске
// выхода 1e-4). Точной остаётся КЛАССИФИКАЦИЯ -- по какую сторону границы
// лежит кусок, решает Polygons::contains в домене CGAL, а не подсчёт
// пересечений в double. Ошибиться в точке разреза можно на нанометр,
// ошибиться стороной -- нельзя.
//
// Отсюда же и терпимость к допускам: ЛИШНЯЯ точка разреза безвредна (оба
// куска классифицируются одинаково и сшиваются обратно), опасен только
// пропуск. Поэтому все пороги ниже щедрые -- в сомнении режем.

#include "geo/boolean.h"
#include "geo/cancel.h"
#include "geo/util.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace Geo {

namespace {

// Сегмент, разобранный один раз: геометрия, длина и габарит для префильтра.
// Габарит раздут на сварочный допуск ещё и потому, что у горизонтального
// отрезка он нулевой высоты, а пустые QRectF не пересекаются ни с чем.
struct Seg {
    QPointF a, b;
    std::optional<Arc> arc; // nullopt -- прямой
    double length{};
    QRectF box;

    // Концы отдаются ИСХОДНЫМИ точками, а не тригонометрией дуги: так куски
    // стыкуются друг с другом и с нетронутыми вершинами бит-в-бит.
    QPointF pointAt(double t) const {
        if(t <= 0.0) return a;
        if(t >= 1.0) return b;
        return arc ? arc->pointAt(t) : a + (b - a) * t;
    }
    // Прогиб куска [t0, t1]: у дуги угол сокращается в (t1 - t0) раз.
    double bulgeAt(double t0, double t1) const {
        return arc ? bulgeOf(arc->theta * (t1 - t0)) : 0.0;
    }
};

Seg makeSeg(const Vertex& from, const Vertex& to) {
    Seg s{from, to, arcOf(from, to, from.bulge)};
    s.length = segmentLength(from, to);
    if(s.arc) {
        // Дугу грубим до габарита её окружности: префильтру хватает.
        const double r = s.arc->radius;
        s.box = {s.arc->center - QPointF{r, r}, s.arc->center + QPointF{r, r}};
    } else {
        s.box = QRectF{s.a, s.b}.normalized();
    }
    s.box.adjust(-exitWeldTolerance, -exitWeldTolerance, exitWeldTolerance, exitWeldTolerance);
    return s;
}

double cross(QPointF a, QPointF b) { return a.x() * b.y() - a.y() * b.x(); }

// Доля пройденного угла дуги до точки p, лежащей на её окружности: 0 --
// начало, 1 -- конец, вне [0, 1] -- точка на окружности, но мимо дуги.
double arcParam(const Arc& arc, QPointF p) {
    const double angle = std::atan2(p.y() - arc.center.y(), p.x() - arc.center.x());
    double sweep = std::fmod(angle - arc.startAngle, 2.0 * pi);
    if(arc.theta > 0.0 && sweep < 0.0) sweep += 2.0 * pi;
    if(arc.theta < 0.0 && sweep > 0.0) sweep -= 2.0 * pi;
    return sweep / arc.theta;
}

// Лежит ли параметр на сегменте. Допуск -- сварочный, пересчитанный в долю
// длины: у короткого сегмента он щедрее, и это правильно -- см. шапку.
bool onSeg(const Seg& seg, double t) {
    const double slack = exitWeldTolerance / std::max(seg.length, exitWeldTolerance);
    return t > -slack && t < 1.0 + slack;
}

// Корни |p + t*d - c|^2 = r^2 по t; касание даёт один корень.
int lineCircleRoots(QPointF p, QPointF d, QPointF c, double r, double roots[2]) {
    const QPointF f = p - c;
    const double a = QPointF::dotProduct(d, d);
    if(a <= 0.0) return 0;
    const double b = 2.0 * QPointF::dotProduct(f, d);
    const double q = QPointF::dotProduct(f, f) - r * r;
    const double disc = b * b - 4.0 * a * q;
    if(disc < 0.0) return 0;
    const double sq = std::sqrt(disc);
    roots[0] = (-b - sq) / (2.0 * a);
    roots[1] = (-b + sq) / (2.0 * a);
    return sq == 0.0 ? 1 : 2;
}

// Параметры разреза субъекта `s` граничным сегментом `bs` -- в `ts`.
void cutSegSeg(std::vector<double>& ts, const Seg& s, const Seg& bs) {
    auto addCut = [&ts](double t) {
        if(t > 0.0 && t < 1.0) ts.push_back(t);
    };

    if(!s.arc && !bs.arc) { // отрезок x отрезок
        const QPointF d1 = s.b - s.a, d2 = bs.b - bs.a;
        const double denom = cross(d1, d2);
        if(std::abs(denom) <= 1e-12 * s.length * bs.length) {
            // Параллельные. Наложение вдоль одной прямой режем по КОНЦАМ
            // граничного сегмента: настоящих пересечений нет, а куски
            // «вдоль границы» отойдут под политику contains.
            if(s.length <= 0.0) return;
            if(std::abs(cross(d1, bs.a - s.a)) / s.length > exitWeldTolerance) return;
            if(std::abs(cross(d1, bs.b - s.a)) / s.length > exitWeldTolerance) return;
            const double dd = QPointF::dotProduct(d1, d1);
            addCut(QPointF::dotProduct(bs.a - s.a, d1) / dd);
            addCut(QPointF::dotProduct(bs.b - s.a, d1) / dd);
            return;
        }
        const double t = cross(bs.a - s.a, d2) / denom;
        const double u = cross(bs.a - s.a, d1) / denom;
        if(onSeg(bs, u)) addCut(t);
        return;
    }

    if(!s.arc) { // отрезок x дуга
        double roots[2];
        const QPointF d1 = s.b - s.a;
        const int n = lineCircleRoots(s.a, d1, bs.arc->center, bs.arc->radius, roots);
        for(int i{}; i < n; ++i)
            if(onSeg(bs, arcParam(*bs.arc, s.a + d1 * roots[i]))) addCut(roots[i]);
        return;
    }

    if(!bs.arc) { // дуга x отрезок: те же корни, но параметром на границе
        double roots[2];
        const QPointF d2 = bs.b - bs.a;
        const int n = lineCircleRoots(bs.a, d2, s.arc->center, s.arc->radius, roots);
        for(int i{}; i < n; ++i)
            if(onSeg(bs, roots[i])) addCut(arcParam(*s.arc, bs.a + d2 * roots[i]));
        return;
    }

    // Дуга x дуга.
    const Arc& sa = *s.arc;
    const Arc& ba = *bs.arc;
    const QPointF cc = ba.center - sa.center;
    const double d = std::hypot(cc.x(), cc.y());
    if(d < exitWeldTolerance && std::abs(sa.radius - ba.radius) < exitWeldTolerance) {
        // Одна окружность: наложение режем по концам граничной дуги.
        addCut(arcParam(sa, bs.a));
        addCut(arcParam(sa, bs.b));
        return;
    }
    if(d <= 0.0) return; // концентрические не пересекаются
    const double along = (sa.radius * sa.radius - ba.radius * ba.radius + d * d) / (2.0 * d);
    const double h2 = sa.radius * sa.radius - along * along;
    if(h2 < 0.0) return;
    const double h = std::sqrt(h2);
    const QPointF u = cc / d;
    const QPointF mid = sa.center + u * along;
    const QPointF perp{-u.y() * h, u.x() * h};
    for(const QPointF p: {mid + perp, mid - perp}) {
        if(onSeg(bs, arcParam(ba, p))) addCut(arcParam(sa, p));
        if(h == 0.0) break; // касание -- корень один
    }
}

// Граница региона, разобранная в сегменты один раз на весь вызов.
struct BoundaryContour {
    QRectF box;
    std::vector<Seg> segs;
};

std::vector<double> cutsFor(const Seg& s, const std::vector<BoundaryContour>& boundary) {
    std::vector<double> ts;
    for(const BoundaryContour& contour: boundary) {
        if(!s.box.intersects(contour.box)) continue;
        for(const Seg& bs: contour.segs)
            if(s.box.intersects(bs.box)) cutSegSeg(ts, s, bs);
    }
    if(ts.empty()) return ts;

    // Прополка: параметры у концов сегмента и дубли сливаются -- кусок
    // тоньше сварочного допуска не геометрия, а шум разреза.
    std::ranges::sort(ts);
    const double tol = exitWeldTolerance / std::max(s.length, exitWeldTolerance);
    std::vector<double> out;
    for(const double t: ts) {
        if(t < tol || t > 1.0 - tol) continue;
        if(!out.empty() && t - out.back() < tol) continue;
        out.push_back(t);
    }
    return out;
}

// Сторона точки относительно региона.
//
// Точный Polygons::contains -- обход ВСЕЙ точной арранжементы региона
// рациональной арифметикой на каждый вызов; на плотной меди (десятки тысяч
// дуг границы) классификация каждого куска им превращала резку в зависание.
// Поэтому чёт/нечет пересечений луча (направо из точки) считается в double
// по тем же сегментам границы, что и разрезы, а точный оракул остаётся
// ОТКАТОМ для сомнительных лучей: прошедших ближе допуска к вершине,
// касанию или к самой границе. Сомнение всегда уходит в точный домен --
// ошибиться СТОРОНОЙ по-прежнему нельзя, см. шапку файла.
class Side {
public:
    Side(const Polygons& region, const std::vector<BoundaryContour>& boundary)
        : region_{region}
        , boundary_{boundary} { }

    bool inside(QPointF p) const {
        int crossings{};
        for(const BoundaryContour& contour: boundary_) {
            if(p.y() < contour.box.top() || p.y() > contour.box.bottom()
                || p.x() > contour.box.right()) continue;
            for(const Seg& seg: contour.segs) {
                if(p.y() < seg.box.top() || p.y() > seg.box.bottom()
                    || p.x() > seg.box.right()) continue;
                const int n = seg.arc ? crossArc(*seg.arc, seg, p) : crossLine(seg, p);
                if(n < 0) return region_.contains(p); // сомнение -- к точному
                crossings += n;
            }
        }
        return crossings & 1;
    }

private:
    // Вершина ближе этого к лучу -- не доказательство: у почти
    // горизонтального ребра деление ay / (ay - by) раздувает погрешность.
    static constexpr double vertexEps = 1e-6;
    // Пересечение ближе этого к самой точке -- «граница под точкой»: та же
    // величина, что и сварочный допуск, туда же попадают куски ВДОЛЬ границы.
    static constexpr double xEps = exitWeldTolerance;

    // Пересечения ребра с лучом: 0/1/2 или -1 -- «луч не доказывает».
    static int crossLine(const Seg& s, QPointF p) {
        const double ay = s.a.y() - p.y(), by = s.b.y() - p.y();
        if(std::abs(ay) < vertexEps || std::abs(by) < vertexEps) return -1; // вершина на луче
        if((ay > 0.0) == (by > 0.0)) return 0;
        const double x = s.a.x() + (s.b.x() - s.a.x()) * ay / (ay - by);
        if(x > p.x() + xEps) return 1;
        if(x < p.x() - xEps) return 0;
        return -1;
    }

    static int crossArc(const Arc& arc, const Seg& s, QPointF p) {
        const double dy = p.y() - arc.center.y();
        const double disc = arc.radius * arc.radius - dy * dy;
        const double tangentBand = vertexEps * std::max(arc.radius, 1.0);
        if(disc <= tangentBand) {
            if(disc < -tangentBand) return 0; // мимо окружности
            // Горизонтальная касательная на уровне луча.
            return arc.center.x() > p.x() - xEps ? -1 : 0;
        }
        const double sq = std::sqrt(disc);
        // Близость параметра к концам дуги -- та же «вершина на луче».
        const double slack = exitWeldTolerance / std::max(s.length, exitWeldTolerance);
        int n{};
        for(const double x: {arc.center.x() - sq, arc.center.x() + sq}) {
            if(x < p.x() - xEps) continue;
            if(x < p.x() + xEps) return -1;
            const double t = arcParam(arc, {x, p.y()});
            if(t < -slack || t > 1.0 + slack) continue; // на окружности, но мимо дуги
            if(t < slack || t > 1.0 - slack) return -1; // конец дуги на луче
            ++n;
        }
        return n;
    }

    const Polygons& region_;
    const std::vector<BoundaryContour>& boundary_;
};

// Кусок сегмента между соседними параметрами разреза.
struct Atom {
    Vertex start; // с прогибом куска
    QPointF end;
    QPointF mid;
    bool keep{};
};

Polylines clipPath(const Polyline& path, const Side& side,
    const std::vector<BoundaryContour>& boundary, bool keepInside) {
    auto keeps = [&side, keepInside](QPointF p) { return side.inside(p) == keepInside; };

    if(path.size() < 2) // одинокой вершине резать нечего: судьба решается ей самой
        return path.empty() || !keeps(path.front()) ? Polylines{} : Polylines{path};

    std::vector<Atom> atoms;
    for(const auto& [from, to]: segments(path)) {
        checkCancelled();
        const Seg s = makeSeg(from, to);
        auto ts = cutsFor(s, boundary);
        ts.push_back(1.0);
        double prev{};
        for(const double t: ts) {
            Atom& a = atoms.emplace_back(
                Vertex{s.pointAt(prev), s.bulgeAt(prev, t)},
                s.pointAt(t), s.pointAt((prev + t) * 0.5));
            a.keep = keeps(a.mid);
            prev = t;
        }
    }

    // Целиком по одну сторону -- вход возвращается КАК ЕСТЬ, с исходными
    // вершинами и замкнутостью: пересборка из атомов только насажала бы
    // лишних точек в местах касаний.
    if(std::ranges::none_of(atoms, &Atom::keep)) return {};
    if(std::ranges::all_of(atoms, &Atom::keep)) return {path};

    Polylines out;
    Polyline cur;
    auto flush = [&out, &cur] {
        if(cur.size() > 1) out.push_back(std::move(cur));
        cur.clear();
    };
    for(const Atom& a: atoms) {
        if(!a.keep) {
            flush();
            continue;
        }
        // Конец предыдущего атома и есть начало этого -- точке хватает
        // прогиба, дублировать её не надо.
        if(cur.empty()) cur.push_back(a.start);
        else cur.back().bulge = a.start.bulge;
        cur.emplace_back(a.end);
    }
    flush();

    // У замкнутого входа шов лежит в стартовой вершине: если уцелели оба
    // прилегающих к ней куска, это ОДИН кусок -- сшиваем через шов.
    if(path.closed && out.size() > 1 && atoms.front().keep && atoms.back().keep) {
        Polyline& tail = out.back();
        Polyline& head = out.front();
        tail.back().bulge = head.front().bulge;
        tail.insert(tail.end(), head.begin() + 1, head.end());
        head = std::move(tail);
        out.pop_back();
    }

    std::erase_if(out, [](const Polyline& p) { return p.perimeter() < exitWeldTolerance; });
    return out;
}

} // namespace

Polylines clipOpen(ClipType_ clipType, const Polylines& paths, const Polygons& region)
    pre(clipType == BooleanOp_::Intersection || clipType == BooleanOp_::Difference) {
    const bool keepInside = clipType == BooleanOp_::Intersection;

    // Пустой регион: снаружи него -- всё, внутри -- ничего.
    if(region.empty()) return keepInside ? Polylines{} : paths;

    const Polylines contours = region.contours();
    std::vector<BoundaryContour> boundary;
    boundary.reserve(contours.size());
    for(const Polyline& contour: contours) {
        BoundaryContour& bc = boundary.emplace_back();
        bc.box = contour.boundingRect().adjusted(
            -exitWeldTolerance, -exitWeldTolerance, exitWeldTolerance, exitWeldTolerance);
        bc.segs.reserve(contour.size());
        for(const auto& [from, to]: segments(contour)) bc.segs.push_back(makeSeg(from, to));
    }

    const Side side{region, boundary};

    Polylines result;
    for(const Polyline& path: paths)
        for(Polyline& piece: clipPath(path, side, boundary, keepInside))
            result.push_back(std::move(piece));
    return result;
}

} // namespace Geo
