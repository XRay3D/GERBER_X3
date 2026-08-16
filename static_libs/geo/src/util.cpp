#include "geo/util.h"

#include "geo/polygon.h"

#include <QPainterPath>
#include <QTransform>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Geo {

namespace {

constexpr double eps = 1e-12;

double dot(QPointF a, QPointF b) { return a.x() * b.x() + a.y() * b.y(); }
double cross(QPointF a, QPointF b) { return a.x() * b.y() - a.y() * b.x(); }

} // namespace

double Arc::length() const noexcept { return radius * std::abs(theta); }

QPointF Arc::pointAt(double t) const noexcept {
    const double a = startAngle + theta * t;
    return center + QPointF{radius * std::cos(a), radius * std::sin(a)};
}

double Arc::segmentArea() const noexcept {
    return 0.5 * radius * radius * (theta - std::sin(theta));
}

std::optional<Arc> arcOf(QPointF p1, QPointF p2, double bulge) {
    if(std::abs(bulge) < eps) return {};

    const double dx = p2.x() - p1.x(), dy = p2.y() - p1.y();
    const double chord = std::hypot(dx, dy);
    if(chord < eps) return {};

    // Знаковая стрела прогиба -- точное тождество sagitta = (хорда/2) * bulge,
    // из него же знаковый радиус по прямоугольному треугольнику
    // (хорда/2, sagitta, radius - sagitta).
    const double halfChord = chord / 2.0;
    const double sagitta = bulge * halfChord;
    const double radiusSigned = (halfChord * halfChord + sagitta * sagitta) / (2.0 * sagitta);

    // Левая нормаль к хорде: центр лежит на ней, на знаковом расстоянии
    // (radius - sagitta) от середины -- знак сам разворачивает центр на нужную
    // сторону хорды, отдельной ветки для отрицательного прогиба не нужно.
    const QPointF n{-dy / chord, dx / chord};
    const QPointF mid{(p1.x() + p2.x()) / 2.0, (p1.y() + p2.y()) / 2.0};

    Arc arc;
    arc.center = mid + n * (radiusSigned - sagitta);
    arc.radius = std::abs(radiusSigned);
    arc.startAngle = std::atan2(p1.y() - arc.center.y(), p1.x() - arc.center.x());
    arc.theta = 4.0 * std::atan(bulge);
    return arc;
}

double arcSweep(QPointF p1, QPointF p2, QPointF center, Vertex::Dir dir) {
    if(dir == Vertex::Line) return 0.0;
    const double a1 = std::atan2(p1.y() - center.y(), p1.x() - center.x());
    const double a2 = std::atan2(p2.y() - center.y(), p2.x() - center.x());
    double theta = a2 - a1;
    if(dir == Vertex::Ccw)
        while(theta <= 0.0) theta += 2.0 * pi;
    else
        while(theta >= 0.0) theta -= 2.0 * pi;
    return theta;
}

double bulgeOf(double theta) { return std::tan(theta / 4.0); }

double bulgeOf(QPointF p1, QPointF p2, QPointF center, Vertex::Dir dir) {
    return bulgeOf(arcSweep(p1, p2, center, dir));
}

double distance(QPointF a, QPointF b) { return std::hypot(b.x() - a.x(), b.y() - a.y()); }

double segmentLength(const Vertex& from, const Vertex& to) {
    if(const auto a = arcOf(from, to, from.bulge))
        return a->length();
    return distance(from, to);
}

// Второй заход -- уже по ГОТОВОЙ полилинии и с допуском сварки.
//
// Первый (sameSupport) сшивает то, что и в точном домене одно: куски одной
// кривой. Сюда доходит другое -- дуги из РАЗНЫХ построений, совпавшие лишь
// численно: две половины одной исходной окружности дают две капсулы, у каждой
// свой центр, и разойтись они успевают на пару последних цифр. Для вывода это
// одна дуга, и мерить её надо тем же допуском, каким меряется всё на выходе.
//
// Замыкающая пара (последняя вершина -- первая) намеренно не трогается: сшивка
// там означала бы сдвиг начала контура, а начало у него задаёт врезку.
void stitchArcs(Polyline& poly, double tolerance) {
    if(poly.size() < 3) return;

    Polyline out;
    out.reserve(poly.size());
    double keptSweep{}; // размах дуги, накопленный последней вершиной out

    for(std::size_t i{}; i < poly.size(); ++i) {
        const Vertex& vertex = poly[i];
        const Vertex& next = poly[(i + 1) % poly.size()];

        if(!out.empty() && out.back().isArc() && vertex.isArc()) {
            const auto prev = arcOf(out.back(), vertex, out.back().bulge);
            const auto curr = arcOf(vertex, next, vertex.bulge);
            const double sweep = keptSweep + (curr ? curr->theta : 0.0);
            if(prev && curr
                && (prev->theta > 0.0) == (curr->theta > 0.0)
                && std::abs(sweep) < maxBulgeSweep
                && distance(prev->center, curr->center) <= tolerance
                && std::abs(prev->radius - curr->radius) <= tolerance) {
                // Дуга, кончающаяся в собственном начале, -- окружность: её
                // прогибом не выразить, вершин у неё должно остаться две, и
                // держится она ДВУМЯ ПОЛОВИНАМИ -- такой её пишет одной командой
                // вывод УП. Иначе куски одной окружности из разных построений
                // (свип режет по вертикальным касательным, капсулы -- по своим
                // швам) сшивались бы в четверть и три четверти. Размах больше
                // полуоборота отсекает микродугу, у которой конец тоже у самого
                // начала. См. Cgal::toPolyline о том же.
                if(std::abs(sweep) > pi && distance(next, out.back()) <= tolerance) {
                    // Недобор до оборота -- шум, половины ровно по pi:
                    // прогиб +-1, как у Geo::circle.
                    const double half = std::copysign(pi, sweep);
                    const QPointF start = out.back();
                    out.back().bulge = std::copysign(1.0, half);
                    out.emplace_back(prev->center * 2.0 - start, std::copysign(1.0, half));
                    keptSweep = half;
                    continue;
                }
                out.back().bulge = bulgeOf(sweep);
                keptSweep = sweep;
                continue; // вершина между двумя кусками одной дуги не нужна
            }
        }

        out.push_back(vertex);
        const auto arc = vertex.isArc() ? arcOf(vertex, next, vertex.bulge) : std::nullopt;
        keptSweep = arc ? arc->theta : 0.0;
    }

    out.closed = poly.closed;
    out.width = poly.width;
    poly = std::move(out);
}

//------------------------------------------------------------------------------

Polyline circle(double diameter, QPointF center) {
    const double radius = diameter / 2.0;
    Polyline polyline{
        Vertex{{center.x() - radius, center.y()}, 1.0},
        Vertex{{center.x() + radius, center.y()}, 1.0},
    };
    polyline.closed = true;
    return polyline;
}

Polyline rectangle(double width, double height, QPointF center) {
    const double w = width / 2.0, h = height / 2.0;
    // Против часовой стрелки: канон внешней границы.
    Polyline polyline{
        Vertex{{center.x() - w, center.y() - h}},
        Vertex{{center.x() + w, center.y() - h}},
        Vertex{{center.x() + w, center.y() + h}},
        Vertex{{center.x() - w, center.y() + h}},
    };
    polyline.closed = true;
    return polyline;
}

Polyline obround(double width, double height, QPointF center) {
    if(std::abs(width - height) < eps)
        return circle(width, center);

    Polyline polyline;
    if(width > height) { // скругления слева и справа
        const double r = height / 2.0, straight = (width - height) / 2.0;
        polyline.assign({
            Vertex{{center.x() - straight, center.y() - r}},
            Vertex{{center.x() + straight, center.y() - r}, 1.0},
            Vertex{{center.x() + straight, center.y() + r}},
            Vertex{{center.x() - straight, center.y() + r}, 1.0},
        });
    } else { // скругления снизу и сверху
        const double r = width / 2.0, straight = (height - width) / 2.0;
        polyline.assign({
            Vertex{{center.x() + r, center.y() - straight}},
            Vertex{{center.x() + r, center.y() + straight}, 1.0},
            Vertex{{center.x() - r, center.y() + straight}},
            Vertex{{center.x() - r, center.y() - straight}, 1.0},
        });
    }
    polyline.closed = true;
    return polyline;
}

Polyline arc(QPointF center, double radius, double startAngle, double sweep) {
    auto at = [&](double angle) {
        return center + QPointF{radius * std::cos(angle), radius * std::sin(angle)};
    };

    if(std::abs(sweep) >= 2.0 * pi - eps) {
        // Полный оборот -- те же две полуокружности, что и у circle(), но
        // обойдённые ОТ ЗАДАННОГО УГЛА: вызывающий стыкует с началом обхода
        // остальной контур (в Gerber дуга приходит серединой пути), и
        // подменять его точкой на оси незачем.
        const double half = sweep < 0.0 ? -pi : pi;
        Polyline polyline{
            Vertex{at(startAngle),        bulgeOf(half)},
            Vertex{at(startAngle + half), bulgeOf(half)},
        };
        polyline.closed = true;
        return polyline;
    }

    const int segments = std::max(1, static_cast<int>(std::ceil(std::abs(sweep) / pi)));
    const double step = sweep / segments;
    const double b = bulgeOf(step);

    Polyline polyline;
    polyline.reserve(segments + 1);
    for(int i{}; i < segments; ++i)
        polyline.emplace_back(at(startAngle + step * i), b);
    polyline.emplace_back(at(startAngle + sweep));
    return polyline;
}

//------------------------------------------------------------------------------

bool isSimilarity(const QTransform& tr) {
    // Столбцы линейной части должны быть ортогональны и равны по длине -- это
    // и есть поворот с равномерным масштабом, возможно с зеркалом. Проекции
    // (m13/m23) дугу тоже не сохраняют.
    if(!tr.isAffine()) return false;
    const QPointF col0{tr.m11(), tr.m12()};
    const QPointF col1{tr.m21(), tr.m22()};
    const double len0 = dot(col0, col0), len1 = dot(col1, col1);
    const double scale = std::max({len0, len1, 1.0});
    return std::abs(len0 - len1) <= eps * scale && std::abs(dot(col0, col1)) <= eps * scale;
}

bool hasArcs(const Polyline& polyline) {
    return std::ranges::any_of(polyline, &Vertex::isArc);
}

Polyline& transform(Polyline& polyline, const QTransform& tr) {
    if(tr.isIdentity()) return polyline;

    for(auto& vertex: polyline)
        static_cast<QPointF&>(vertex) = tr.map(static_cast<const QPointF&>(vertex));

    // Зеркало разворачивает обход дуги -- определитель отрицателен ровно тогда,
    // когда ориентация плоскости сменилась (поворот сам по себе её сохраняет).
    if(tr.determinant() < 0.0)
        for(auto& vertex: polyline) vertex.bulge = -vertex.bulge;

    return polyline;
}

Polylines& transform(Polylines& polylines, const QTransform& tr) {
    for(auto& polyline: polylines) transform(polyline, tr);
    return polylines;
}

Polyline& translate(Polyline& polyline, QPointF offset) {
    if(offset.isNull()) return polyline;
    for(auto& vertex: polyline) static_cast<QPointF&>(vertex) += offset;
    return polyline;
}

Polylines& translate(Polylines& polylines, QPointF offset) {
    for(auto& polyline: polylines) translate(polyline, offset);
    return polylines;
}

Polyline& rotate(Polyline& polyline, double angle, QPointF center) {
    QTransform tr;
    tr.translate(center.x(), center.y());
    tr.rotate(angle);
    tr.translate(-center.x(), -center.y());
    return transform(polyline, tr);
}

Polylines& rotate(Polylines& polylines, double angle, QPointF center) {
    for(auto& polyline: polylines) rotate(polyline, angle, center);
    return polylines;
}

namespace {

// Зеркало меняет знак обхода каждого контура, и внешняя граница начинает
// выглядеть дыркой. Разворот возвращает канон, не трогая формы.
void restoreOrientation(Polylines& contours, const QTransform& tr) {
    if(tr.determinant() >= 0.0) return;
    for(auto& contour: contours) contour.reverse();
}

} // namespace

Polygon transformed(const Polygon& polygon, const QTransform& tr) {
    // Чистый перенос -- САМЫЙ частый случай (вспышка апертуры в точку на
    // плате) -- считается точно, без выхода в double и обратно: см.
    // Geo::translated. Остаток (поворот, масштаб, зеркало) через double
    // пройти вынужден -- у CGAL-домена этой библиотеки нет представления
    // для иррационального cos/sin произвольного угла.
    if(tr.type() <= QTransform::TxTranslate)
        return translated(polygon, QPointF{tr.dx(), tr.dy()});

    Polylines outer{polygon.outer()};
    Polylines holes{polygon.holes()};
    transform(outer, tr);
    transform(holes, tr);
    restoreOrientation(outer, tr);
    restoreOrientation(holes, tr);
    // Конструктор приводит обход к канону сам, так что развёрнутые контуры
    // помеченного пустотой дадут то же самое тело; пометку переносим отдельно.
    Polygon result{outer.front(), holes};
    if(polygon.isInverted()) result.invert();
    return result;
}

Polygons transformed(const Polygons& polygons, const QTransform& tr) {
    if(tr.type() <= QTransform::TxTranslate)
        return translated(polygons, QPointF{tr.dx(), tr.dy()});

    // Поштучно, а не общим списком контуров: у каждого тела свои дырки, и в
    // плоском списке вложенность выражена одной ориентацией -- дырка одного
    // тела вычла бы чужое тело, лежащее внутри неё, и оно бы пропало.
    std::vector<Polygon> parts;
    parts.reserve(polygons.size());
    for(const Polygon& polygon: polygons) parts.push_back(transformed(polygon, tr));
    return Polygons{std::span<const Polygon>{parts}};
}

//------------------------------------------------------------------------------

Polyline operator*(Polyline polyline, const QTransform& tr) { return transform(polyline, tr), polyline; }
Polyline& operator*=(Polyline& polyline, const QTransform& tr) { return transform(polyline, tr); }
Polylines operator*(Polylines polylines, const QTransform& tr) { return transform(polylines, tr), polylines; }
Polylines& operator*=(Polylines& polylines, const QTransform& tr) { return transform(polylines, tr); }

Polygon operator*(const Polygon& polygon, const QTransform& tr) { return transformed(polygon, tr); }
Polygon& operator*=(Polygon& polygon, const QTransform& tr) { return polygon = transformed(polygon, tr); }
Polygons operator*(const Polygons& polygons, const QTransform& tr) { return transformed(polygons, tr); }
Polygons& operator*=(Polygons& polygons, const QTransform& tr) { return polygons = transformed(polygons, tr); }

//------------------------------------------------------------------------------

namespace {

// Кубик Безье, оказавшийся дугой окружности: центр -- пересечение нормалей к
// касательным в концах, а совпадение проверяется по нескольким точкам самой
// кривой. nullopt -- не дуга (прямая, почти параллельные нормали, промах).
std::optional<Arc> arcOfCubic(QPointF start, QPointF ctrl1, QPointF ctrl2, QPointF end,
    double tolerance) {
    const QPointF t0 = ctrl1 - start; // касательная в начале
    const QPointF t1 = end - ctrl2;   // касательная в конце
    const double det = cross(t0, t1);
    if(std::abs(det) < eps) return {}; // прямая или касательные сонаправлены

    // (X - start)*t0 == 0 и (X - end)*t1 == 0 -- две прямые, на пересечении центр.
    const double c0 = dot(start, t0), c1 = dot(end, t1);
    const QPointF center{
        (c0 * t1.y() - c1 * t0.y()) / det,
        (c1 * t0.x() - c0 * t1.x()) / det};

    const double radius = std::hypot(start.x() - center.x(), start.y() - center.y());
    if(!std::isfinite(radius) || radius < eps) return {};

    auto at = [&](double t) {
        const double u = 1.0 - t;
        return u * u * u * start + 3.0 * u * u * t * ctrl1 + 3.0 * u * t * t * ctrl2 + t * t * t * end;
    };
    constexpr int samples = 5;
    for(int i = 1; i <= samples; ++i) {
        const QPointF pt = at(i / static_cast<double>(samples + 1));
        const double d = std::hypot(pt.x() - center.x(), pt.y() - center.y());
        if(std::abs(d - radius) > tolerance * radius) return {};
    }

    // Направление -- по знаку поворота от радиуса к касательной в начале.
    const auto dir = cross(start - center, t0) > 0.0 ? Vertex::Ccw : Vertex::Cw;
    Arc result;
    result.center = center;
    result.radius = radius;
    result.startAngle = std::atan2(start.y() - center.y(), start.x() - center.x());
    result.theta = arcSweep(start, end, center, dir);
    return result;
}

// Кубик, дугой не оказавшийся, дробится на отрезки. Число кусков -- по длине
// контрольной ломаной: она мажорирует длину самой кривой.
void flattenCubic(Polyline& polyline, QPointF start, QPointF ctrl1, QPointF ctrl2, QPointF end,
    double tolerance) {
    auto dist = [](QPointF a, QPointF b) { return std::hypot(b.x() - a.x(), b.y() - a.y()); };
    const double hull = dist(start, ctrl1) + dist(ctrl1, ctrl2) + dist(ctrl2, end);
    const int segments = std::clamp(static_cast<int>(std::ceil(hull / std::max(tolerance, eps))), 1, 64);
    for(int i = 1; i <= segments; ++i) {
        const double t = i / static_cast<double>(segments), u = 1.0 - t;
        polyline.emplace_back(u * u * u * start + 3.0 * u * u * t * ctrl1
            + 3.0 * u * t * t * ctrl2 + t * t * t * end);
    }
}

} // namespace

Polylines fromPath(const QPainterPath& path, double tolerance) {
    Polylines result;
    Polyline current;

    auto flush = [&] {
        if(current.size() < 2) return current.clear();
        // Путь Qt замыкается повтором первой точки -- ровно то, что убирает close().
        if(static_cast<const QPointF&>(current.front()) == static_cast<const QPointF&>(current.back()))
            current.close();
        result.emplace_back(std::move(current));
        current.clear();
    };

    for(int i{}; i < path.elementCount();) {
        const auto element = path.elementAt(i);
        switch(element.type) {
        case QPainterPath::MoveToElement:
            flush();
            current.emplace_back(QPointF{element});
            ++i;
            break;

        case QPainterPath::LineToElement:
            current.emplace_back(QPointF{element});
            ++i;
            break;

        case QPainterPath::CurveToElement: {
            // Кубик -- это CurveTo плюс два CurveToData; без них путь битый и
            // разбирать дальше нечего.
            if(current.empty() || i + 2 >= path.elementCount()) {
                flush();
                return result;
            }
            const QPointF start = current.back();
            const QPointF ctrl1{element};
            const QPointF ctrl2{path.elementAt(i + 1)};
            const QPointF end{path.elementAt(i + 2)};
            if(auto a = arcOfCubic(start, ctrl1, ctrl2, end, tolerance)) {
                // Прогиб пишется на НАЧАЛЬНОЙ вершине дуги -- она уже в контуре.
                current.back().bulge = bulgeOf(a->theta);
                current.emplace_back(end);
            } else
                flattenCubic(current, start, ctrl1, ctrl2, end, tolerance);
            i += 3;
        } break;

        case QPainterPath::CurveToDataElement: // разобран вместе со своим CurveTo
            ++i;
            break;
        }
    }
    flush();

    return result;
}

QPainterPath toPath(const Polylines& polylines) {
    QPainterPath path;
    for(const Polyline& polyline: polylines)
        path.addPath(polyline.toPath());
    return path;
}

} // namespace Geo
