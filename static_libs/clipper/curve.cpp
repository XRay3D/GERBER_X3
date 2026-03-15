/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
// module;

#include "curve.h"
#include "gi_dbg.h"
#include <app.h>
#include <ranges>

Curve CircleCurve(double diametr, const Point& center) {
    if(qFuzzyIsNull(diametr)) return {};
    const double radius = diametr * 0.5;
    const int intSteps = App::settings().clpCircleSegments(radius);
    Curve polygon(intSteps);
    for(int i{}; auto&& pt: polygon) {
        pt = Point{
                 cos(i * 2 * pi / intSteps) * radius,
                 sin(i * 2 * pi / intSteps) * radius,
             }
            + center;
        ++i;
    };
    // r::for_each(polygon, std::bind(&SetZ, _1, center));
    return polygon;
}

Curve RectangleCurve(double width, double height, const Point& center) {
    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Curve polygon{
        {-halfWidth + center.x, +halfHeight + center.y},
        {-halfWidth + center.x, -halfHeight + center.y},
        {+halfWidth + center.x, -halfHeight + center.y},
        {+halfWidth + center.x, +halfHeight + center.y},
        // {-halfWidth + center.x, +halfHeight + center.y},
    };
    r::for_each(polygon, &SetZs);
    // if(Area(polygon) < 0.0) ReverseCurve(polygon);
    return polygon;
}

void RotateCurve(Curve& polygon, double angle, const Point& center) {
    const bool fl = Area(polygon) < 0;
    for(Point& pt: polygon) {
        const double dAangle = qDegreesToRadians(angle - Angle(center, pt));
        const double length = distTo(center, pt);
        pt = Point{cos(dAangle) * length, sin(dAangle) * length};
        pt.x += center.x;
        pt.y += center.y;
    }
    if(fl != (Area(polygon) < 0))
        ReverseCurve(polygon);
}

Curve& TranslateCurve(Curve& curve, const Point& pos) {
    if(pos.x || pos.y)
        for(auto& pt: curve) {
            pt.x += pos.x;
            pt.y += pos.y;
            SetZf(pt, GetZ(pt) + pos);
        }
    return curve;
}

Curves& TranslateCurves(Curves& curves, const Point& pos) {
    r::for_each(curves, std::bind(&TranslateCurve, _1, pos));
    return curves;
}

//  Вычисление площади.
constexpr auto Area(std::span<const QPointF> points) -> double {
    if(points.front() == points.back()) points = points.subspan(1);

    if(points.size() < 3) return std::nan("");

    double sum{};

    // Перебираем пары
    for(auto&& [it, next]: points | v::pairwise)
        sum += it.x() * next.y() - next.x() * it.y();

    // последний и первый
    sum += points.back().x() * points.front().y()
        - points.front().x() * points.back().y();

    return sum / 2;
}

/* ----------  Вспомогательные функции  ---------- */
constexpr double norm(const QPointF& p) noexcept { return std::hypot(p.x(), p.y()); }
constexpr double dot(const QPointF& a, const QPointF& b) noexcept { return QPointF::dotProduct(a, b); }

/* ----------  Главная функция ---------- */
/// \brief Находит пересечение двух окружностей.
/// \param center1   центр 1‑й окружности
/// \param onCircle1 точка на 1‑й окружности (для вычисления r1)
/// \param center2   центр 2‑й окружности
/// \param onCircle2 точка на 2‑й окружности (для вычисления r2)
/// \return Optional<vector<Point>>
///   - empty()       – отсутствует решение (совпадают окружности)
///   - empty vector  – пересечений нет
///   - vector с 1 или 2 точками – найденные пересечения
std::vector<QPointF> circleIntersection(
    const QPointF& center1, const QPointF& onCircle1,
    const QPointF& center2, const QPointF& onCircle2) {
    constexpr double EPS = 1e-10;

    // радиусы
    const double r1 = norm(onCircle1 - center1);
    const double r2 = norm(onCircle2 - center2);

    const QPointF dVec = center2 - center1; // вектор C1->C2
    const double d = norm(dVec);            // расстояние между центрами

    // особые случаи
    if(d <= EPS && std::abs(r1 - r2) <= EPS) { // совпадают
        return {};
    }
    if(d >= r1 + r2 - EPS || d <= std::abs(r1 - r2) + EPS) {
        if(std::abs(d - (r1 + r2)) <= EPS || std::abs(d - std::abs(r1 - r2)) <= EPS) {
            // касание – одна точка
            // пересчитаем a, h (h будет ~0)
            if(d <= EPS) // точно совпали, но это уже обработано выше
                return {center1};
        } else {
            // пересечений нет
            return {};
        }
    }

    // теперь считаем a, h
    const double a = (r1 * r1 - r2 * r2 + d * d) / (2.0 * d);
    const double h2 = r1 * r1 - a * a; // может быть слегка отрицательным из-за округления
    const double h = h2 <= EPS ? 0.0 : std::sqrt(h2);

    // точка между центрами
    const QPointF p0 = center1 + dVec * (a / d);

    // отступы перпендикулярно к прямой C1C2
    const double rx = -(dVec.y()) * (h / d);
    const double ry = dVec.x() * (h / d);

    std::vector<QPointF> intersections;
    // одна точка (касание)
    if(std::abs(h) <= EPS) {
        intersections.emplace_back(p0.x() + rx, p0.y() + ry);
    } else {
        intersections.emplace_back(p0.x() + rx, p0.y() + ry);
        intersections.emplace_back(p0.x() - rx, p0.y() - ry);
    }
    return intersections;
}

struct {
    constexpr bool operator()(double l1, double l2, double epsilon = 0.001) const {
        return std::abs(l1 - l2) <= epsilon;
    }
    constexpr bool operator()(const QPointF& p1, const QPointF& p2,
        double l2, double epsilon = 0.001) const {
        return std::abs(length(p1, p2) - l2) <= epsilon;
    }
    constexpr bool operator()(double l1,
        const QPointF& p1, const QPointF& p2, double epsilon = 0.001) const {
        return std::abs(l1 - length(p1, p2)) <= epsilon;
    }
    constexpr bool operator()(const QPointF& p1, const QPointF& p2,
        const QPointF& p3, const QPointF& p4, double epsilon = 0.001) const {
        return std::abs(length(p1, p2) - length(p3, p4)) <= epsilon;
    }
} constexpr TEST;

struct {
    constexpr auto operator()(std::convertible_to<QPointF> auto&&... points) const
        requires(sizeof...(points) > 2)
    {
        return Area({std::forward<decltype(points)>(points)...}) > .0 ? Vertex::Ccw : Vertex::Cw;
    }
} constexpr DIR;

#if 1
struct Epsilon {
    double epsilon{0.001}; // mm
    double val{};
    friend constexpr Epsilon operator*(double l, Epsilon r) { return r.val = l, r; }
    friend constexpr Epsilon operator*(Epsilon l, double r) { return l.val = r, l; }
    friend constexpr bool operator==(Epsilon l, double r) {
        return std::abs(l.val - r) <= l.epsilon;
    }
    friend constexpr bool operator==(double l, Epsilon r) {
        return std::abs(l - r.val) <= r.epsilon;
    }
};

constexpr Epsilon operator""_e(long double epsilon) { return {static_cast<double>(epsilon)}; }

static_assert(10. * 0.001_e == 10);
static_assert(10. * 0.001_e != 11);

static_assert(10. == 10 * 0.001_e);
static_assert(10. != 11 * 0.001_e);
#endif

bool isPointOnCircleDistance(
    QPointF pt, QPointF center,
    double radius, double epsilon = 1e-4) {

    pt -= center;
    double distance = pt.x() * pt.x() + pt.y() * pt.y();
    return TEST(distance, radius * radius, epsilon);
}

bool isPointOnCircle(
    QPointF pt,
    QPointF center, QPointF cPt,
    double epsilon = 1e-4) {

    double radius = length(center, cPt);
    pt -= center;
    double distance = pt.x() * pt.x() + pt.y() * pt.y();
    return TEST(distance, radius * radius, epsilon);
}

bool isPointOnCircleInt(int x, int y, int centerX, int centerY, int diameter) {
    int radius = diameter / 2;
    int dx = x - centerX;
    int dy = y - centerY;
    int radiusSquared = radius * radius;
    // Проверяем точное равенство для целых чисел
    return (dx * dx + dy * dy) == radiusSquared;
}

//  Пересечение с прямой (бесконечной линией)
QPolygonF lineCircleIntersection(
    QPointF center, double radius,
    QPointF lineStart, QPointF lineEnd) {

    QPolygonF intersections;

    // Вектор направления линии
    double dx = lineEnd.x() - lineStart.x();
    double dy = lineEnd.y() - lineStart.y();

    // Переносим центр окружности в начало координат
    double fx = lineStart.x() - center.x();
    double fy = lineStart.y() - center.y();

    // Коэффициенты квадратного уравнения: a*t² + b*t + c = 0
    double a = dx * dx + dy * dy;
    double b = 2 * (fx * dx + fy * dy);
    double c = fx * fx + fy * fy - radius * radius;

    // Решаем квадратное уравнение
    double discriminant = b * b - 4 * a * c;

    if(discriminant < 0) {
        return intersections; // Нет пересечений
    }

    if(std::abs(discriminant) < 1e-10) {
        // Одно решение (касательная)
        double t = -b / (2 * a);
        intersections.push_back(QPointF(
            lineStart.x() + t * dx,
            lineStart.y() + t * dy));
    } else {
        // Два решения
        double sqrtDisc = std::sqrt(discriminant);
        double t1 = (-b + sqrtDisc) / (2 * a);
        double t2 = (-b - sqrtDisc) / (2 * a);

        intersections.push_back(QPointF(
            lineStart.x() + t1 * dx,
            lineStart.y() + t1 * dy));
        intersections.push_back(QPointF(
            lineStart.x() + t2 * dx,
            lineStart.y() + t2 * dy));
    }

    return intersections;
}
// Пересечение с отрезком (ограниченной линией)
QPolygonF segmentCircleIntersection(QPointF center, double radius, QPointF segStart, QPointF segEnd) {

    auto points = lineCircleIntersection(center, radius, segStart, segEnd);
    QPolygonF result;

    // Проверяем, попадают ли точки на отрезок
    for(const auto& p: points) {
        // Проверяем, что точка лежит между началом и концом отрезка
        double minX = std::min(segStart.x(), segEnd.x()) - 1e-10;
        double maxX = std::max(segStart.x(), segEnd.x()) + 1e-10;
        double minY = std::min(segStart.y(), segEnd.y()) - 1e-10;
        double maxY = std::max(segStart.y(), segEnd.y()) + 1e-10;

        if(p.x() >= minX && p.x() <= maxX && p.y() >= minY && p.y() <= maxY) {
            result.push_back(p);
        }
    }

    return result;
}

Curve& TransformCurve(Curve& curve, const QTransform& tr) {
    qInfo() << tr;

    if(tr.isIdentity()) return curve;

    for(auto& v: curve) {
        if(v.type) v.center = tr.map(v.center);
        v.pt = tr.map(v.pt);
    }

    if((tr.m11() < .0) ^ (tr.m22() < .0))
        for(auto&& v: curve) v.type = Vertex::Type{-v.type};

    return curve;
}

Curves& ReverseCurves(Curves& curves) {
    // r::for_each(curves, &Curve::Reverse);
    return curves;
}

Curve& TranslateCurve(Curve& curve, const QPointF& pos) {
    if(!pos.isNull())
        for(auto& pt: curve) {
            pt.center += pos;
            pt.pt += pos;
        }
    return curve;
}

void RotateCurve(Curve& curve, double angle, const PointF& center) {
    // const bool fl = curve.GetArea() < 0;

    auto rot = [center, angle](PointF& pt) {
        pt.Rotate(qDegreesToRadians(angle));
        pt += center;
    };

    for(auto&& pt: curve) {
        if(pt.type) (rot(pt.center));
        rot(pt.pt);
    }

    // if(fl != (curve.GetArea() < 0))
    //     curve.Reverse();
}

QPainterPath toPPath(Curve curve, std::optional<QTransform> tr, bool arcOnly) {
    if(curve.size() < 2) return {};

    if(tr) TransformCurve(curve, *tr);

    QPointF source = curve.front().pt;

    QPainterPath pPath;
    pPath.moveTo(source.x(), source.y());

    for(auto&& v: curve | v::drop(1)) {
        if(!v.type) {
            arcOnly ? pPath.moveTo(v.pt.x(), v.pt.y())
                    : pPath.lineTo(v.pt.x(), v.pt.y());
            source = v.pt;
            continue;
        }

        const QPointF target = v.pt;

        QLineF ls{v.center, source};
        const double r = ls.length();
        const double asource = ls.angle();
        const double atarget = ls.angleTo(QLineF{v.center, target});

        double span = atarget;
        // if(curve.size() == 3)
        //     qCritical() << asource
        //                 << atarget
        //                 << span;

        QRectF rect{
            v.center.x() - r,
            v.center.y() - r,
            r * 2,
            r * 2,
        };

        if(v.type == Vertex::Ccw) span = span - 360.;

        pPath.arcTo(rect, asource, span);

        source = v.pt;
    }
    return pPath;
}

QPainterPath toPPath(const Curves& curves) {
    if(curves.empty()) return {};
    QPainterPath pPath(toPPath(curves.front()));
    for(const auto& curve: curves | v::drop(1))
        pPath.addPath(toPPath(curve));
    return pPath;
}

struct Arc {
    QPointF center;
    double radius;
    double startAngle; // радианы
    double endAngle;   // радианы
    bool isFullCircle;
    QVector<int> pointIndices; // индексы точек в исходном полигоне
};

struct Circle {
    QPointF center;
    double radius{}, maxError{};
};
using SpanPF = std::span<const QPointF>;
// Аппроксимация окружности по набору точек (возвращает true, если успешно)
bool fitCircle(SpanPF points, QPointF& center, double& radius, double& maxError) {
    int n = points.size();
    if(n < 3) return false;

    // Составляем систему линейных уравнений A * [A, B, C]^T = b
    // (используем обозначения A,B,C для коэффициентов, чтобы не путать с центром)
    double sumX{}, sumY{}, sumX2{}, sumY2{}, sumXY{},
        sumX3{}, sumY3{}, sumX2Y{}, sumXY2{};

    for(auto [x, y]: points) {
        double x2 = x * x;
        double y2 = y * y;
        sumX += x;
        sumY += y;
        sumX2 += x2;
        sumY2 += y2;
        sumXY += x * y;
        sumX3 += x2 * x;
        sumY3 += y2 * y;
        sumX2Y += x2 * y;
        sumXY2 += x * y2;
    }

    // Матрица нормальных уравнений для метода наименьших квадратов
    // (здесь используется алгебраический метод, минимизирующий сумму квадратов невязок)
    double B11 = sumX2;
    double B12 = sumXY;
    double B13 = sumX;
    double B21 = sumXY;
    double B22 = sumY2;
    double B23 = sumY;
    double B31 = sumX;
    double B32 = sumY;
    double B33 = n;

    double C1 = -(sumX3 + sumXY2);
    double C2 = -(sumX2Y + sumY3);
    double C3 = -(sumX2 + sumY2);

    // Решаем систему 3x3 методом Крамера (или любым другим)
    double det
        = B11 * (B22 * B33 - B23 * B32)
        - B12 * (B21 * B33 - B23 * B31)
        + B13 * (B21 * B32 - B22 * B31);

    if(std::abs(det) < 1e-12) return false;

    double detA
        = C1 * (B22 * B33 - B23 * B32)
        - B12 * (C2 * B33 - B23 * C3)
        + B13 * (C2 * B32 - B22 * C3);
    double detB
        = B11 * (C2 * B33 - B23 * C3)
        - C1 * (B21 * B33 - B23 * B31)
        + B13 * (B21 * C3 - C2 * B31);
    double detC
        = B11 * (B22 * C3 - C2 * B32)
        - B12 * (B21 * C3 - C2 * B31)
        + C1 * (B21 * B32 - B22 * B31);

    double Acoef = detA / det;
    double Bcoef = detB / det;
    double Ccoef = detC / det;

    // Вычисляем параметры окружности
    center.setX(-Acoef / 2.0);
    center.setY(-Bcoef / 2.0);
    radius = sqrt(QPointF::dotProduct(center, center) - Ccoef);

    // Оценка максимального отклонения
    maxError = 0;
    for(QPointF p: points) {
        p -= center;
        double d = hypot(p.x(), p.y());
        maxError = std::max(maxError, std::abs(d - radius));
    }
    return true;
}

std::optional<Circle> fitCircle(SpanPF points, double maxError = 1e-5) {
    Circle c;
    if(fitCircle(points, c.center, c.radius, c.maxError) && c.maxError < maxError) {
        qWarning() << c.maxError;
        return c;
    } else if(!c.center.isNull())
        qDebug() << c.maxError;
    return {};
}

// Сегментация полигона на дуги
QVector<Arc> extractArcs(const QPolygonF& polygon, double tolerance = 2.0) {
    QVector<Arc> arcs;
    int n = polygon.size();
    if(n < 3) return arcs;

    // Копируем точки для удобства
    QPolygonF points{polygon};

    // Если полигон не замкнут, добавим первую точку в конец для обработки замыкания
    bool closed = polygon.isClosed();
    if(!closed)
        points.append(points.front());
    closed = true;
    auto end = points.end() - 2;
    auto startIdx = points.begin();
    while(startIdx < end) {
        SpanPF candidate; //{startIdx, startIdx + 1};
        auto endIdx = startIdx + 3;
        QPointF center;
        double radius, error;

        // Пытаемся расширять участок, пока качество аппроксимации удовлетворительно
        while(endIdx < points.end()) {
            candidate = SpanPF{startIdx, endIdx};
            qWarning() << points.size() << candidate.size();
            if(!fitCircle(candidate, center, radius, error) || error > tolerance) {
                // Не подходит – убираем последнюю точку и завершаем дугу
                candidate = candidate.subspan(0, candidate.size() - 1);
                break;
            }
            ++endIdx;
        }
        qWarning() << points.size() << candidate.size();

        // Если набрано минимум 3 точки – фиксируем дугу
        if(candidate.size() >= 4) {
            Arc arc;
            arc.center = center;
            arc.radius = radius;
            arc.isFullCircle = false;
            // Определим углы
            QVector<double> angles;
            for(QPointF p: candidate) {
                p -= center;
                double angle = atan2(p.y(), p.x());
                angles.append(angle);
            }
            // Нормализуем углы и найдём размах
            r::sort(angles);
            double minAng = angles.front();
            double maxAng = angles.back();
            // Проверка на охват почти 360°
            if(closed && (maxAng - minAng > two_pi)) { // 2π ≈ 6.283
                arc.isFullCircle = true;
                arc.startAngle = 0;
                arc.endAngle = two_pi;
            } else {
                arc.startAngle = minAng;
                arc.endAngle = maxAng;
            }
            // Сохраняем индексы точек (оригинальные, без учёта возможного дублирования)
            // for(int i = 0; i < candidate.size(); ++i) {
            //     // Ищем индекс в исходном полигоне
            //     int idx = polygon.indexOf(candidate[i]);
            //     if(idx >= 0) arc.pointIndices.append(idx);
            // }
            arcs.append(arc);
            // Новый старт – с последней точки текущей дуги
            startIdx = endIdx - 1;
        } else {
            // Не удалось построить дугу – сдвигаем старт
            ++startIdx;
        }
    }
    return arcs;
}

Curve toCurve(Path& path, bool open) {
    Curve curve;
    QPainterPath pp;
#if 0
    { // find arcs
        static auto eqCenter = [line = QLineF{}, a = 0.](Point& l, Point& r) mutable {
            constexpr double epsilon = 0.2; // mm
            if(line.isNull()) {
                line = {~l, ~r};
                return true;
            }
            QLineF newl{~l, ~r};
            double angle = line.angleTo(newl);

            double k = line.length() / newl.length();
            qCritical().noquote() << std::format("{:+8.3f} {:+8.3f}", std::abs(a - angle), k);

            bool fl = std::clamp(k, /*1 - epsilon*/ 0.1, 1. /*1 + epsilon*/) == k
                && ((angle < 90 || angle > 270) && TEST(a,angle,10));
            line = newl;
            a = angle;
            return fl;
        };

        static auto notEqCenter = [](Point& l, Point& r) { return !eqCenter(l, r); };

        if(auto it = r::adjacent_find(path, notEqCenter); it != path.end())
            r::rotate(path, r::adjacent_find(path, notEqCenter));

        auto chunks = v::chunk_by(path, eqCenter);

        for(auto&& path: chunks) {
            qDebug() << path.size();
            if(auto c = fitCircle(path | v::transform(toQPointF) | r::to<QList>(), 1e-3)) {
                QPainterPath pp;
                pp.addEllipse(c->center, c->radius, c->radius);
                Gi::Debug(pp, Qt::magenta);
                // curve.emplace_back(~path.front());
                // curve.emplace_back(~path.front(), c->center, Vertex::Cw);
                // return curve;
            }
        }
    }
#endif

    if(!open && path.front() != path.back()) path.emplace_back(path.front());

    if(auto c = fitCircle(~path); c && path.size() > 4) {
        pp.addEllipse(c->center, c->radius, c->radius);
        Gi::Debug(pp, Qt::yellow);
        curve.emplace_back(~path.front());
        auto dir = DIR(~path[0], ~path[1], c->center);
        curve.emplace_back(~path[path.size() / 2], c->center, dir);
        curve.emplace_back(~path.front(), c->center, dir);
        return curve;
    }

#if 0
    {
        std::set<QPointF> skip;
        for(auto&& var: ~path | v::slide(10)) {
            if(auto c = fitCircle(var); c && skip.emplace(c->center).second) {
                pp.addEllipse(c->center, c->radius, c->radius);
                new Gi::Debug{pp, Qt::yellow};
            }
        }
        return {};

        if(auto arcs = extractArcs(~path, 1e-6); arcs.size()) {
            for(auto&& a: arcs) {
                pp.moveTo(QLineF::fromPolar(a.radius, qRadiansToDegrees(a.startAngle)).p2() + a.center);
                pp.lineTo(a.center);
                pp.lineTo(QLineF::fromPolar(a.radius, qRadiansToDegrees(a.endAngle)).p2() + a.center);
                // pp.addEllipse(a.center, a.radius, a.radius);
                qInfo() << a.radius;
            }
            new Gi::Debug{pp, Qt::green};
            return {};
            pp.clear();
            for(auto&& a: arcs) {
                pp.moveTo(~path[a.pointIndices.front()]);
                for(auto&& i: a.pointIndices) {
                    pp.lineTo(~path[i]);
                }
            }
            new Gi::Debug{pp};
        }

        return {};

        static auto eqCenter = [&path](Point& l, Point& r) {
            if(&l == &path.front()) return true;
            static double ap, ac;
            QLineF l1{~(&l)[-1], ~l};
            QLineF l2{~l, ~r};

            ac = l1.angleTo(l2);
            bool fl = TEST(l1.length(), l2.length(), std::min(l1.length(), l2.length()) / 2)
                // && TEST(ap,ac,0.1))
                ;
            ap = ac;

            return fl;
        };

        auto chunks = v::chunk_by(path, eqCenter);
        for(auto&& path: chunks) {
            pp.moveTo(~path.front());
            if(path.size() > 2) {
                for(Point& pt: path | skipFront)
                    pp.lineTo(~pt);
            }
        }

        new Gi::Debug{pp, Qt::cyan};

        return {};
    }
#endif

    if(1) { // Исправление центров объединённых линий.
        static auto fixJoinedCenters = +[](Point& p1, Point& p2, Point& p3) {
            constexpr double epsilon = 0.001; // mm
            const QPointF c1{~GetC(p1)}, c2{~GetC(p2)}, c3{~GetC(p3)};
            if(c2.isNull() && c1 == c3 && TEST(c1, ~p1, c1, ~p2, epsilon))
                SetCForce(p2, ~c1);
        };
        for(auto&& [p1, p2, p3]: path | v::adjacent<3>)
            fixJoinedCenters(p1, p2, p3);
        // fixJoinedCenters(path[1], path.back(), path.front());
        // fixJoinedCenters(path[1], path.front(), path.back());
    }

    static auto eqCenter = +[](Point& l, Point& r) {
        constexpr double epsilon = 0.001; // mm
        auto cl{~GetC(l)}, cr{~GetC(r)};

        if(cl.isNull() && cr.isNull()) return false;
#if 0
        if(cl.isNull() && TEST(cr, ~l, cr, ~r, epsilon))
            return SetCForce(l, ~cr), true;
        if(cr.isNull() && TEST(cl, ~l, cl, ~r, epsilon))
            return SetCForce(r, ~cl), true;
        else
#endif
        return cl == cr && TEST(cl, ~l, cr, ~r, epsilon);
    };

    static auto notEqCenter = +[](Point& l, Point& r) { return !eqCenter(l, r); };

    if(auto it = r::adjacent_find(path, notEqCenter); it != path.end())
        r::rotate(path, it + 1);

    auto chunks = v::chunk_by(path, eqCenter);

    qInfo() << "toCurve" << path.size();

#if 0
    for(auto&& [p1, p2, p3]: chunks | v::adjacent<3>) { // NOTE fix centers
        QPointF p{~p2.front()};
        double epsilon = 0.001; // mm
        if(p2.size() == 1) {
            QPointF c1{~GetC(p1.back())}, c3{~GetC(p3.front())};
            if(c1 == c3)
                SetCForce(p2.front(), GetC(p1.back()));
            else {

                // if(p1.size() > 2
                //     //&& TEST(~p1[0], ~p1[1],~p1.front(), p,epsilon))
                //     && TEST(c1, ~p1.front()),c1, p),epsilon))
                //     SetCForce(p2.front(), GetC(p1.front()));
                // else if(p3.size() > 2
                //     //&& TEST(~p3[0], ~p3[1],~p3.front(), p,epsilon))
                //     && TEST(c3, ~p3.front(),c3, p,epsilon))
                //     SetCForce(p2.front(), GetC(p3.front()));

                // if(auto c1 = fitCircle(p1
                //        | v::transform(toQPointF)
                //        | r::to<QList>()),
                //     c2 = fitCircle(std::array{p1, p2}
                //         | v::join
                //         | v::transform(toQPointF)
                //         | r::to<QList>());
                //     c1 && c2 && TEST(c1->maxError,c2->maxError,1e-4)) {
                //     qInfo() << c1->maxError << c2->maxError;
                //     SetCForce(p2.front(), GetC(p3.front()));
                // } else if(auto c1 = fitCircle(p3
                //               | v::transform(toQPointF) | r::to<QList>()),
                //     c2 = fitCircle(std::array{p3, p2}
                //         | v::join
                //         | v::transform(toQPointF)
                //         | r::to<QList>());
                //     c1 && c2 && TEST(c1->maxError,c2->maxError,1e-4)) {
                //     qInfo() << c1->maxError << c2->maxError;
                //     SetCForce(p2.front(), GetC(p3.front()));
                // }

                // else if(p3.size() > 2
                //     //&& TEST(~p3[0], ~p3[1],~p3.front(), p,epsilon))
                //     && TEST(c3, ~p3.front(),c3, p,epsilon))
                //     SetCForce(p2.front(), GetC(p3.front()));
            }
        }
    }
#endif

    chunks = v::chunk_by(path, eqCenter);

    if(chunks.front().size() == path.size()) {
        qWarning() << "circle" << path.size();
        QPointF center = ~GetC(path.front());
        auto dir = DIR(~path[0], ~path[1], center);
        double radius = length(center, ~path[0]);
        curve.emplace_back(center + PointF{0., +radius});
        curve.emplace_back(center + QPointF{0., -radius}, center, dir);
        curve.emplace_back(center + QPointF{0., +radius}, center, dir);
    } else {
        constexpr double epsilon = 0.01; // mm

        Vertex::Type type{};
        for(auto&& path: chunks) {
            QPointF pt{~path.front()};
            if(path.size() > 1) {
                if(Perimeter(path, true) < 0.1) {
                    curve.emplace_back(pt);
                    continue;
                }
                QPointF center = ~GetC(path.front());
                type = DIR(pt, ~path[2], center);

                if(curve.size()) {
                    auto& v = curve.back();
                    if(v.type) {
                        if(TEST(v.radius(), v.center, pt, epsilon)) {
                            v.pt = pt; // NOTE update prev arc
                        } else if(TEST(center, v.pt, center, pt, epsilon)) {
                            pt = v.pt; // NOTE update prev arc
                        }
                    }
                    if(!TEST(center, v.pt, center, pt, epsilon)) {
                        curve.emplace_back(pt);
                    }
                } else curve.emplace_back(pt);
                curve.emplace_back(~path.back(), center, type);
            } else {
                if(curve.size()) {
                    auto& v = curve.back();
                    // if(v.type && TEST(v.radius(), v.center, pt, epsilon)) {
                    //     v.pt = pt; // NOTE update prev arc
                    //     continue;
                    // }
                    if(v.type) {
                        if(TEST(v.radius(), v.center, pt, epsilon)) {
                            v.pt = pt; // NOTE update prev arc
                            continue;
                        }
                        // else if(TEST(center, v.pt, center, pt, epsilon)) {
                        //     pt = v.pt; // NOTE update prev arc
                        //     continue;
                        // }
                    }
                }
                curve.emplace_back(pt);
            }
        }
        if(curve.size() > 2) {
            auto &vb = curve.back(), &vf = curve.front();
            double rb = vb.radius(), rf = curve[1].radius();
            if(curve.back().type && curve[1].type) {
                if(TEST(rb, vb.center, vf.pt)) {
                    // qCritical("CC1");
                    vb.pt = vf.pt;
                } else if(TEST(rf, vf.center, vb.pt)) {
                    qCritical("CC2 NO test cases");
                    // pp.addEllipse(QRectF{-5e-1, -5e-1, 1e-0, 1e-0}.translated(vb.pt));
                    // pp.addEllipse(QRectF{-5e-1, -5e-1, 1e-0, 1e-0}.translated(vf.pt));
                    vf.pt = vb.pt;
                } else {
                    // qCritical("CC3");
                    curve.emplace_back(curve.front());
                }
            } else if(vb.type && !curve[1].type) {
                qCritical("CL");
                if(TEST(rb, vb.center, vf.pt, 0.005)) vb.pt = vf.pt;
                else curve.emplace_back(curve.front());
            } else if(!vb.type && curve[1].type) {
                // qCritical("LC");
                if(TEST(rf, curve[1].center, vb.pt, 0.005)) vf.pt = vb.pt;
                else curve.emplace_back(curve.front());
            } else if(!vb.type && !curve[1].type) {
                // qCritical("LL");
                curve.emplace_back(curve.front());
            }

            // && 0
            // && curve.back().type
            // // && isPointOnCircle(curve.front().pt, curve.back().center, curve.back().pt, 0.01)
            // && !curve[0].type
            // && !vf.type) {

            // double radius = length(curve.back().center, curve.back().pt);
            // auto pts = lineCircleIntersection(curve.back().center, radius, curve[0].pt, curve[1].pt);
            // qCritical() << "Fix end" << pts;

            // if(!pts.empty()) {
            //     if(length(pts.front(), curve.front().pt) < length(pts.back(), curve.front().pt)) { // TODO fix backward.
            //         pp.addEllipse(QRectF{-5e-2, -5e-2, 1e-1, 1e-1}.translated(pts.front()));
            //         curve.front().pt = curve.back().pt = pts.front();
            //     } else
            //         curve.front().pt = curve.back().pt = pts.back();
            // }
        }
    }

    // new Gi::Debug{toPPath(curve)};
    Gi::Debug(toPPath(curve, {}, true), Qt::green);
    // new Gi::Debug{toPPath(curve.reverse()), Qt::magenta};

    // for(auto&& v: curve)
    // pp.addEllipse(QRectF{-5e-2, -5e-2, 1e-1, 1e-1}.translated(v.pt));
    Gi::Debug(pp, Qt::cyan);
    return curve;
}
//------------------------------------------------------------------------------
static int GetQuadrant(const PointF& v) {
    // 0 = [+,+], 1 = [-,+], 2 = [-,-], 3 = [+,-]
    if(v.x() > 0) {
        if(v.y() > 0) {
            return 0;
        }
        return 3;
    }
    if(v.y() > 0) {
        return 1;
    }
    return 2;
}

static PointF QuadrantEndPoint(int i) {
    if(i > 3) i -= 4;
    switch(i) {
    case 0 : return {0.0, +1.0};
    case 1 : return {-1.0, 0.0};
    case 2 : return {0.0, -1.0};
    default: return {+1.0, 0.0};
    }
}

double IncludedAngle(const PointF& v0, const PointF& v1, int dir) {
    // returns the absolute included angle between 2 vectors in the direction of dir ( 1=acw  -1=cw)
    double inc_ang = PointF::dotProduct(v0, v1);
    if(inc_ang > 1. - 1.0e-10) {
        return 0;
    }
    if(inc_ang < -1. + 1.0e-10) {
        inc_ang = pi;
    } else { // dot product,   v1 . v2  =  cos ang
        if(inc_ang > 1.0) {
            inc_ang = 1.0;
        }
        inc_ang = acos(inc_ang); // 0 to pi radians

        if(dir * PointF::crossProduct(v0, v1) < 0) {
            inc_ang = 2 * pi - inc_ang; // cp
        }
    }
    return dir * inc_ang;
}

struct Span {
    //     PointF NearestPointNotOnSpan(const PointF& p) const;
    //     double Parameter(const PointF& p) const;
    //     PointF NearestPointToSpan(const Span& p, double& d) const;

    //     static const PointF null_point;
    //     static const Vertex null_vertex;

    // public:
    PointF pt;
    Vertex vx;
    bool m_start_span{};
    //     Span();
    //     Span(const PointF& p, const Vertex& v, bool start_span = false)
    //         : m_start_span(start_span)
    //         , pt(p)
    //         , vx(v)
    //     {}
    //     PointF NearestPoint(const PointF& p) const;
    //     PointF NearestPoint(const Span& p, double* d = NULL) const;
    //     void GetBox(CBox2D& box);
    //     double IncludedAngle() const;
    //     double GetArea() const;
    //     bool On(const PointF& p, double* t = NULL) const;
    //     PointF MidPerim(double d) const;
    //     PointF MidParam(double param) const;
    //     double Length() const;
    //     PointF GetVector(double fraction) const;
    //     void Intersect(
    //         const Span& s,
    //         std::list<PointF>& pts
    //         ) const;  // finds all the intersection points between two spans

    PointF NearestPointNotOnSpan(const PointF& p) const {
        if(!vx.type) {
            PointF Vs{vx.pt - pt};
            Vs.normalize();
            double dp = PointF::dotProduct((p - pt), Vs);
            return (Vs * dp) + pt;
        } else {
            double radius = pt.dist(vx.center);
            double r = p.dist(vx.center);
            if(r < PointF::tolerance) return pt;
            PointF vc{vx.center - p};
            return p + vc * ((r - radius) / r);
        }
    }

    PointF NearestPoint(const PointF& p) const {
        PointF np = NearestPointNotOnSpan(p);
        double t = Parameter(np);
        if(0.0 <= t && t <= 1.0) return np;
        double d1 = p.dist(pt);
        double d2 = p.dist(vx.pt);
        return (d1 < d2) ? pt : vx.pt;
    }

    PointF MidPerim(double d) const {
        /// returns a point which is 0-d along span
        PointF p;
        if(vx.type == 0) {
            PointF vs{vx.pt - pt};
            vs.normalize();
            p = vs * d + pt;
        } else {
            PointF v{pt - vx.center};
            double radius = v.length();
            v.Rotate(d * int(vx.type) / radius);
            p = v + vx.center;
        }
        return p;
    }
    PointF MidParam(double param) const {
        /// returns a point which is 0-1 along span
        if(qFuzzyIsNull(param)) return pt;

        if(qFuzzyIsNull(param - 1.0)) return vx.pt;

        PointF p;
        if(vx.type == 0) {
            PointF vs{vx.pt - pt};
            p = vs * param + pt;
        } else {
            PointF v{pt - vx.center};
            v.Rotate(param * IncludedAngle());
            p = v + vx.center;
        }
        return p;
    }

    PointF NearestPointToSpan(const Span& p, double& d) const {
        PointF midpoint = MidParam(0.5);
        PointF np = p.NearestPoint(pt);
        PointF best_point = pt;
        double dist = np.dist(pt);
        if(p.m_start_span) {
            dist -= (PointF::tolerance * 2); // give start of curve most priority
        }
        PointF npm = p.NearestPoint(midpoint);
        double dm = npm.dist(midpoint)
            - PointF::tolerance; // lie about midpoint distance to give midpoints priority
        if(dm < dist) {
            dist = dm;
            best_point = midpoint;
        }
        PointF np2 = p.NearestPoint(vx.pt);
        double dp2 = np2.dist(vx.pt);
        if(dp2 < dist) {
            dist = dp2;
            best_point = vx.pt;
        }
        d = dist;
        return best_point;
    }
    PointF NearestPoint(const Span& p, double* d) const {
        double best_dist;
        PointF best_point = this->NearestPointToSpan(p, best_dist);

        // try the other way round too
        double best_dist2;
        PointF best_point2 = p.NearestPointToSpan(*this, best_dist2);
        if(best_dist2 < best_dist) {
            best_point = NearestPoint(best_point2);
            best_dist = best_dist2;
        }

        if(d) {
            *d = best_dist;
        }
        return best_point;
    }
    QRectF boundingRect() {
        QPolygonF box{pt, vx.pt};

        if(this->vx.type) {
            // arc, add quadrant points
            PointF vs = pt - vx.center;
            PointF ve = vx.pt - vx.center;
            int qs = GetQuadrant(vs);
            int qe = GetQuadrant(ve);
            if(vx.type == -1) {
                // swap qs and qe
                int t = qs;
                qs = qe;
                qe = t;
            }

            if(qe < qs) qe = qe + 4;

            double rad = vx.pt.dist(vx.center);

            for(int i = qs; i < qe; i++)
                box.emplace_back(vx.center + QuadrantEndPoint(i) * rad);
        }
        return box.boundingRect();
    }

    double IncludedAngle() const {
        if(vx.type) {
            PointF vs{pt - vx.center};
            PointF ve{vx.pt - vx.center};
            vs.ry() *= -1.;
            ve.ry() *= -1.;

            if(vx.type == -1) {
                vs = -vs;
                ve = -ve;
            }
            vs.normalize();
            ve.normalize();

            return ::IncludedAngle(vs, ve, vx.type);
        }

        return 0.0;
    }
    double GetArea() const {
        if(vx.type) {
            double angle = IncludedAngle();
            double radius = pt.dist(vx.center);
            return (
                0.5
                * ((vx.center.x() - pt.x()) * (vx.center.y() + pt.y())
                    - (vx.center.x() - vx.pt.x()) * (vx.center.y() + vx.pt.y()) - angle * radius * radius));
        }

        return 0.5 * (vx.pt.x() - pt.x()) * (pt.y() + vx.pt.y());
    }
    double Parameter(const PointF& p) const {
        double t;
        if(vx.type == 0) {
            PointF v0{p - pt};
            PointF vs{vx.pt - pt};
            double length = vs.length();
            vs.normalize();
            t = PointF::dotProduct(vs, v0);
            t = t / length;
        } else {
            // true if p lies on arc span sp (p must be on circle of span)
            PointF vs{pt - vx.center};
            PointF v{p - vx.center};
            vs.ry() *= -1.;
            v.ry() *= -1.;
            vs.normalize();
            v.normalize();
            if(vx.type == -1) {
                vs = -vs;
                v = -v;
            }
            double ang = ::IncludedAngle(vs, v, vx.type);
            double angle = IncludedAngle();
            t = ang / angle;
        }
        return t;
    }
    bool On(const PointF& p, double* t) const {
        if(p != NearestPoint(p)) {
            return false;
        }
        if(t) {
            *t = Parameter(p);
        }
        return true;
    }
    double Length() const {
        if(vx.type) {
            double radius = pt ^ vx.center;
            return std::abs(IncludedAngle()) * radius;
        }

        return pt ^ vx.pt;
    }
    PointF GetVector(double fraction) const {
        /// returns the direction vector at point which is 0-1 along span
        if(vx.type == 0) {
            PointF v{vx.pt - pt};
            v.normalize();
            return v;
        }

        PointF p = MidParam(fraction);
        PointF v{p - vx.center};
        v.normalize();
        if(vx.type == 1) {
            return PointF(-v.y(), v.x());
        } else {
            return PointF(v.y(), -v.x());
        }
    }
    // void Intersect(const Span& s, std::list<PointF>& pts) const {
    //     // finds all the intersection points between two spans and puts them in the given list
    //     geoff_geometry::PointF pInt1, pInt2;
    //     double t[4];
    //     int num_int = MakeSpan(*this).Intof(MakeSpan(s), pInt1, pInt2, t);
    //     if(num_int > 0) {
    //         pts.emplace_back(pInt1.x(), pInt1.y());
    //     }
    //     if(num_int > 1) {
    //         pts.emplace_back(pInt2.x(), pInt2.y());
    //     }
    // }
};

//------------------------------------------------------------------------------

QRectF Curve::boundingRect() const {
    QRectF rect;
    // PointF prev_p{};
    // bool prev_p_valid = false;
    // for(auto It = begin(); It != end(); It++) {
    //     Vertex& vertex = *It;
    //     if(prev_p_valid) {
    //         rect = rect | Span{prev_p, vertex}.boundingRect();
    //     }
    //     prev_p = vertex.pt;
    //     prev_p_valid = true;
    // }

    for(auto&& [s, d]: *this | v::pairwise)
        rect = rect | Span{s.pt, d}.boundingRect();
    return rect;
}

Curve& Curve::reverse() {
    Curve new_vertices;
    Vertex* prev{};
    qWarning() << *this;

    for(Vertex& v: *this | v::reverse) {
        Vertex::Type type{};
        PointF cp{};
        if(prev) {
            type = Vertex::Type{-prev->type};
            cp = prev->center;
        }
        Vertex new_v(v.pt, cp, type);
        new_vertices.push_back(new_v);
        prev = &v;
    }
    swap(new_vertices);

    qWarning() << *this;

    // r::reverse(*this);

    // Vertex prev;
    // for(auto&& v: *this) {
    // }
    // Vertex prev;
    // for(auto&& [s, d]: *this | v::pairwise) {
    //     prev = s;
    //     // std::swap(s.center, d.center);
    //     // if(prev.type) {
    //     //     std::swap(s.type, d.type);
    //     //     d.type = Vertex::Type{-d.type};
    //     // }
    // }
    return *this;
}

double Curve::area() const {
    double area = 0.0;
    PointF prev_p = PointF(0, 0);
    bool prev_p_valid = false;
    for(auto It = begin(); It != end(); It++) {
        const Vertex& vertex = *It;
        if(prev_p_valid) {
            area += Span(prev_p, vertex).GetArea();
        }
        prev_p = vertex.pt;
        prev_p_valid = true;
    }
    return area;
}

bool Curve::isClosed() const {
    if(size() == 0) {
        return false;
    }
    return front().pt == back().pt;
}
