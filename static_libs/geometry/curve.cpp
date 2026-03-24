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
#include "span.h"
#include <QMatrix2x2>
#include <QVector2D>
#include <app.h>
#include <ranges>

Curve CircleCurve(double diametr, const PointF& center) {
    Curve curve;
    PointF pt{center};
    pt.rx() -= diametr / 2;
    curve.emplace_back(pt, center, Vertex::Ccw);
    pt.rx() += diametr;
    curve.emplace_back(pt, center, Vertex::Ccw);
    pt.rx() -= diametr;
    curve.emplace_back(pt, center, Vertex::Ccw);
    return curve;
}

Curve RectangleCurve(double width, double height, const PointF& center) {
    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Curve curve{
        Vertex{{-halfWidth + center.x(), +halfHeight + center.y()}},
        Vertex{{-halfWidth + center.x(), -halfHeight + center.y()}},
        Vertex{{+halfWidth + center.x(), -halfHeight + center.y()}},
        Vertex{{+halfWidth + center.x(), +halfHeight + center.y()}},
        Vertex{{-halfWidth + center.x(), +halfHeight + center.y()}},
    };
    return curve;
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

// Curves& ReverseCurves(Curves& curves) {
//     r::for_each(curves.centerurves, &Curve::Reverse);
//     return curves;
// }

Curve& TranslateCurve(Curve& curve, const PointF& pos) {
    if(!qFuzzyIsNull(pos.length()))
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
        if(pt.type) rot(pt.center);
        rot(pt.pt);
    }

    // if(fl != (curve.GetArea() < 0))
    //     curve.Reverse();
}

QDataStream& operator<<(QDataStream& stream, const Vertex& vert) {
    return stream << vert.type
                  << vert.pt
                  << vert.center;
}

QDataStream& operator>>(QDataStream& stream, Vertex& vert) {
    return stream >> vert.type
        >> vert.pt
        >> vert.center;
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
/*
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
    r::for_each(curves, &Curve::reverse);
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
*/
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
    QPainterPath pPath;
    for(auto&& curve: curves) pPath.addPath(toPPath(curve));
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

Curve toCurve(std::span<Point> path, bool open) {
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

    // if(path.front() == path.back()) path = path.subspan(1);

    if(auto c = fitCircle(~path); c && path.size() > 4) { // Circle
        // pp.addEllipse(c->center, c->radius, c->radius);
        // Gi::Debug(pp, Qt::yellow);
        auto dir = DIR(~path[0], ~path[1], c->center);
        curve = CircleCurve(c->radius * 2, c->center);
        if(dir == Vertex::Cw) curve.reverse();
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

    if(chunks.front().size() == path.size()) { // Circle
        qWarning() << "circle" << path.size();
        QPointF center = ~GetC(path.front());
        auto dir = DIR(~path[0], ~path[1], center);
        double radius = length(center, ~path[0]);
        curve = CircleCurve(radius * 2, center);
        if(dir == Vertex::Cw) curve.reverse();
    } else {
        constexpr double epsilon = 0.01; // mm
        Vertex::Type type{};
        for(auto&& path: chunks) {
            QPointF pt{~path.front()};
            if(path.size() > 1) {
                // arc
                // if(Perimeter(path, true) < 0.1) {
                //     curve.emplace_back(pt);
                //     continue;
                // }
                QPointF center = ~GetC(path.front());
                type = DIR(pt, ~path[2], center);

                if(curve.size()) {
                    auto& v = curve.back();
                    if(v.type) {
                        if(TEST(v.radius(), v.center, pt, epsilon)) {
                            QLineF radLine{v.center, pt};
                            radLine.setLength(v.radius());
                            v.pt = radLine.p2(); // update prev arc
                        } else if(TEST(center, v.pt, center, pt, epsilon)) {
                            // QLineF radLine{center, v.pt};
                            // radLine.setLength(v.radius());
                            pt = v.pt; // update prev arc
                        }
                    }
                    if(!TEST(center, v.pt, center, pt, epsilon)) {
                        curve.emplace_back(pt);
                    }
                } else curve.emplace_back(pt);
                curve.emplace_back(~path.back(), center, type);
            } else {
                // line
                if(curve.size()) {
                    auto& v = curve.back();
                    if(v.type && TEST(v.radius(), v.center, pt, epsilon)) {
                        QLineF radLine{v.center, pt};
                        radLine.setLength(v.radius());
                        v.pt = radLine.p2(); // update prev arc
                        continue;
                    }
                }
                curve.emplace_back(pt);
            }
        }

        if(!open && curve.size() > 2 /*&& curve.back().pt != curve.front().pt*/) { // close curve
            auto &vb = curve.back(), &vf = curve.front(), &vc = curve[1];
            double rb = vb.radius(), rf = vc.radius();
            if(curve.back().type && vc.type) {
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
                    curve.emplace_back(vf.pt);
                }
            } else if(vb.type && !vc.type) {
                // qCritical("CL");
                if(TEST(rb, vb.center, vf.pt, 0.005)) {
                    QLineF radLine{vb.center, vf.pt};
                    radLine.setLength(rb);
                    vb.pt = radLine.p2();
                } else curve.emplace_back(vf.pt);
            } else if(!vb.type && vc.type) {
                // qCritical("LC");
                if(TEST(rf, vc.center, vb.pt, 0.005)) {
                    QLineF radLine{vc.center, vb.pt};
                    radLine.setLength(rf);
                    vf.pt = radLine.p2();
                } else curve.emplace_back(vf.pt);
            } else if(!vb.type && !vc.type) {
                // qCritical("LL");
                curve.emplace_back(vf.pt);
            }
        }
    }

    // Gi::Debug(toPPath(curve, {}, true), Qt::green);
    // Gi::Debug(pp, Qt::cyan);
    curve.area();

    return curve;
}

Curves toCurves(std::span<Path> paths, bool open) {
    return {std::from_range, paths | v::transform(std::bind(toCurve, _1, open))};
}

//------------------------------------------------------------------------------

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
    double area{};
    for(auto&& [fr, to]: *this | v::pairwise)
        area += Span{fr.pt, to}.GetArea();
    return area;
}

bool Curve::isClosed() const {
    if(empty()) return false;
    return front().pt == back().pt;
}

double Curve::perimetr() const {
    double perimetr{};
    for(auto&& [fr, to]: *this | v::pairwise)
        perimetr += Span{fr.pt, to}.Length();
    return perimetr;
}

static Path AddVertex(const Vertex& prev_vertex, const Vertex& vertex) {
    constexpr double accuracy = 0.01;

    if(!vertex.type) {
        return {~vertex.pt};
    } else {
        Path path;
        if(vertex.pt != prev_vertex.pt) {
            double phi, dphi;

            int Segments;
            int i;
            double ang1, ang2, phit;

            PointF d = (prev_vertex.pt - vertex.center);
            ang1 = atan2(d.y(), d.x());
            if(ang1 < 0) ang1 += 2.0 * pi;

            d = (vertex.pt - vertex.center);
            ang2 = atan2(d.y(), d.x());
            if(ang2 < 0) ang2 += 2.0 * pi;

            if(vertex.type == -1) { // clockwise
                if(ang2 > ang1) {
                    phit = 2.0 * pi - ang2 + ang1;
                } else {
                    phit = ang1 - ang2;
                }
            } else { // counter_clockwise
                if(ang1 > ang2) {
                    phit = -(2.0 * pi - ang1 + ang2);
                } else {
                    phit = -(ang2 - ang1);
                }
            }

            // what is the delta phi to get an accuracy of aber
            double radius = d.length();
            dphi = 2 * acos((radius - accuracy) / radius);

            // set the number of segments
            // if(phit > 0) {
            //     Segments = (int)ceil(phit / dphi);
            // } else {
            //     Segments = (int)ceil(-phit / dphi);
            // }

            // if(Segments < CArea::m_min_arc_points) {
            //     Segments = CArea::m_min_arc_points;
            // }
            // // if (Segments > CArea::m_max_arc_points)
            // //     Segments=CArea::m_max_arc_points;

            Segments = App::settings().clpCircleSegments(radius); // FIXME count by angle

            // qCritical() << "Segments" << Segments;

            dphi = phit / Segments;

            PointF p = prev_vertex.pt;

            for(i = 1; i <= Segments; i++) {
                d = p - vertex.center;
                phi = atan2(d.y(), d.x());
                double nx = vertex.center.x() + radius * cos(phi - dphi);
                double ny = vertex.center.y() + radius * sin(phi - dphi);
                p = PointF{nx, ny};
                SetCForce(path.emplace_back(~p), ~vertex.center);
            }
            return path;
        }
    }
    return {};
}

Path toPath(const Curve& curve) {
    Path path{~curve.front().pt};
    if(curve.empty()) return {};
    for(auto&& [fr, to]: curve | v::pairwise) {
        Path arc = AddVertex(fr, to);
        if(arc.size())
            if(Point pt = GetC(arc.front()); pt != Point{})
                SetCForce(path.back(), GetC(arc.front()));
        path.append_range(std::move(arc));
    }
    r::for_each(path, &SetCSelf);
    // Gi::Debug({path});
    return path;
}

Paths toPaths(const Curves& curves) {
    return {std::from_range, curves | v::transform(toPath)};
}

// Вычисление центра окружности по трём точкам
bool circleCenter(
    const QPointF& A, const QPointF& B, const QPointF& C,
    QPointF& center, double& radius) {

    // Середины отрезков
    QPointF M_ab = (A + B) / 2.0;
    QPointF M_bc = (B + C) / 2.0;

    // Векторы хорд
    QPointF v_ab = B - A;
    QPointF v_bc = C - B;

    // Нормали (перпендикуляры)
    QPointF n_ab(-v_ab.y(), v_ab.x());
    QPointF n_bc(-v_bc.y(), v_bc.x());

    // Решение системы M_ab + t * n_ab = M_bc + s * n_bc
    // Выразим t из уравнения для x и y
    double det = n_ab.x() * n_bc.y() - n_ab.y() * n_bc.x();
    if(std::abs(det) < 1e-12) return false; // Точки коллинеарны

    QPointF delta = M_bc - M_ab;
    double t = (delta.x() * n_bc.y() - delta.y() * n_bc.x()) / det;
    center = M_ab + t * n_ab;
    radius = QPointF::dotProduct(center - A, center - A);
    radius = std::sqrt(radius);
    return true;
}

// Вычисление точки на кривой Безье по параметру t
QPointF bezierPoint(
    const QPointF& P0, const QPointF& P1,
    const QPointF& P2, const QPointF& P3,
    double t) {

    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;

    QPointF point
        = uuu * P0
        + 3.0 * uu * t * P1
        + 3.0 * u * tt * P2
        + ttt * P3;
    return point;
}

// Проверка, является ли кривая Безье дугой окружности
bool isArcOfCircle(
    const QPointF& P0, const QPointF& P1,
    const QPointF& P2, const QPointF& P3,
    QPointF& center, double& radius, double eps = 1e-5) {

    std::vector path{
        std::from_range,
        std::array{0.0, 0.25, 0.5, 0.75, 1.0}
            | v::transform(std::bind(bezierPoint, P0, P1, P2, P3, _1))
    };

    if(auto c = fitCircle(path, eps); c) { // Circle
        center = c->center, radius = c->radius;
        return true;
    }

    // Берём три точки: начало, середина, конец
    QPointF mid = bezierPoint(P0, P1, P2, P3, 0.5);

    if(!circleCenter(P0, mid, P3, center, radius))
        return false;

    // Проверяем несколько точек на кривой
    for(double t: {0.0, 0.25, 0.5, 0.75, 1.0}) {
        QPointF pt = bezierPoint(P0, P1, P2, P3, t) - center;
        double dist = QPointF::dotProduct(pt, pt);
        dist = std::sqrt(dist);
        if(!TEST(dist, radius, eps)) return false;
    }
    return true;
}

/**
 * \brief Проверка, что данный кубический Bézier задаёт круговую дугу, и поиск её центра.
 *
 * @param start    начальная точка
 * @param ctrl1    первый контрольный пункт
 * @param ctrl2    второй контрольный пункт
 * @param end      конечная точка
 * @param center   (out) центр найденного круга
 * @param radius   (out) радиус круга
 * @param tolerance допускается ошибка при проверке того, что кривая принадлежит кругу.
 * @return true, если кривая действительно представляет круговую дугу
 */
bool circleArcFromCubic(const QPointF& start, const QPointF& ctrl1,
    const QPointF& ctrl2, const QPointF& end,
    QPointF& center, double& radius,
    double tolerance = 1e-5) {
    // 1. Никакие контрольные векторы не нулевые
    const QPointF tanStart = ctrl1 - start;
    const QPointF tanEnd = end - ctrl2;
    if((tanStart - QPointF(0, 0)).manhattanLength() < tolerance || (tanEnd - QPointF(0, 0)).manhattanLength() < tolerance)
        return false; // это линия, а не круг

    // 2. Пересечение двух прямых: (X - start) · tanStart = 0
    //                                      (X - end)   · tanEnd   = 0
    // Решаем 2x2 систему: A * (Cx, Cy)^T = b
    QMatrix2x2 A;
    A.setToIdentity();
    A(0, 0) = tanStart.x();
    A(0, 1) = tanStart.y();
    A(1, 0) = tanEnd.x();
    A(1, 1) = tanEnd.y();
    QVector2D b0 = -QVector2D(start.x() * tanStart.x() + start.y() * tanStart.y(), 0);
    QVector2D b1 = -QVector2D(end.x() * tanEnd.x() + end.y() * tanEnd.y(), 0);
    // Вектор b: ( -start·tanStart, -end·tanEnd )
    //   Мы не будем использовать QMatrix2x2, а решим напрямую
    double det = A(0, 0) * A(1, 1) - A(0, 1) * A(1, 0);
    if(qAbs(det) < tolerance) return false; // почти параллельные прямые

    double cx = (-(start.x() * tanStart.x() + start.y() * tanStart.y()) * A(1, 1)
                    - -(end.x() * tanEnd.x() + end.y() * tanEnd.y()) * A(0, 1))
        / det;
    double cy = (A(0, 0) * -(end.x() * tanEnd.x() + end.y() * tanEnd.y())
                    - A(1, 0) * -(start.x() * tanStart.x() + start.y() * tanStart.y()))
        / det;

    center = QPointF(cx, cy);

    // 3. Радиус: расстояние от центра до начала
    radius = QLineF(center, start).length();

    // Проверяем, что расстояние до конца и до точки t=0.5
    // находятся на той же окружности (с заданной точностью)
    auto evaluateCubic = [&](qreal t) {
        qreal u = 1 - t;
        qreal tt = t * t, uu = u * u;
        return u * uu * start + 3 * u * tt * ctrl1 + 3 * tt * u * ctrl2 + t * t * t * end;
    };
    const int samples = 5;
    for(int i = 1; i < samples; ++i) {
        qreal t = i / static_cast<qreal>(samples + 1);
        QPointF pt = evaluateCubic(t);
        double d = QLineF(pt, center).length();
        if(qAbs(d - radius) > tolerance * radius) // относительная ошибка
            return false;
    }
    return true;
}

bool isClockwise(const QPointF& center,
    const QPointF& start,
    const QPointF& ctrl1) {
    QVector2D R{start - center};
    QVector2D T{ctrl1 - start}; // касательная в начале
    double cross = R.x() * T.y() - R.y() * T.x();
    return cross < 0; // true → по часовой стрелке
}

Paths toPaths(const QPainterPath& pPath) {
    using QPP = QPainterPath;
    using El = QPP::Element;

    constexpr auto name = +[](const El& e) {
        switch(e.type) {
        case QPP::MoveToElement     : return "MoveTo"sv;
        case QPP::LineToElement     : return "LineTo"sv;
        case QPP::CurveToElement    : return "CurveTo"sv;
        case QPP::CurveToDataElement: return "CurveToData"sv;
        }
        return ""sv;
    };
    Paths paths;
    for(auto&& elements:
        v::iota(0, pPath.elementCount())
            | v::transform(std::bind(&QPP::elementAt, pPath, _1))           // to Element
            | v::chunk_by([](const El&, const El& r) { return r.type; })) { // to Subpath Polygons
        qInfo() << "elements" << elements.size();                           // count of elements

        auto segments = elements
            | v::chunk_by([](const El&, const El& r) { return r.type > 2; }); // separate Curves

        for(auto&& segment: segments)
            qDebug() << std::vector{std::from_range,
                segment | v::transform(name)};
        Path path;
        for(auto&& [from, to]:
            segments | v::pairwise) { // 'from' point to bezier Point in 'to' if
            if(path.empty()) path.emplace_back(~from.back());

            // 'to' is Curve
            if(to.front().type == QPP::CurveToElement) {
                // Проверка, является ли кривая Безье дугой окружности
                QPointF center;
                double radius;
                if(/*circleArcFromCubic*/ isArcOfCircle(from.back(), to[0], to[1], to[2], center, radius,
                    5e-3)) {
                    qWarning() << radius << center;
                    Vertex t{
                        QPointF{to.back()},
                        center,
                        // isClockwise(center, from.back(), to[0]) ? Vertex::Cw : Vertex::Ccw,
                        DIR(center, from.back(), to.back()),
                    };
                    path.append_range(AddVertex({{from.back()}}, t));
                } else {
                    path.emplace_back(~to.back());
                    // Gi::Debug({CircleCurve(0.5, {to[1]})});
                    // qCritical() << "WTF must be an arc!";
                    // qCritical() << (from | r::to<QPolygonF>()) << (to | r::to<QPolygonF>()) << (QPointF(to[0]) == to[1]) << (QPointF(to[1]) == to[2]);
                }
            } else
                path.emplace_back(~to.front());
        }
        r::for_each(path, &SetCSelf);
        if(path.size()) paths.emplace_back(std::move(path));
    }
    return paths;
}
