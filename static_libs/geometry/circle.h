/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "geometry.h"

// #include <curve.h>

namespace geo::circle {

using SpanPF = std::span<const QPointF>;

struct Circle {
    QPointF center;
    double radius;
    constexpr Circle(const QPointF& center, double radius)
        : center{center}, radius{radius} { }
    constexpr Circle(const QPointF& center, const QPointF& ptOn)
        : center{center}, radius{Length(center, ptOn)} { }
    constexpr Circle(const Vertex& vertex)
        : Circle{vertex.center, vertex.pt} { }
};

struct Arc {
    QPointF center;
    double radius;
    double startAngle; // радианы
    double endAngle;   // радианы
    bool isFullCircle;
    QVector<int> pointIndices; // индексы точек в исходном полигоне
};

// Находит пересечение двух окружностей.
struct {
    // - empty()       – отсутствует решение (совпадают окружности)
    // - empty vector  – пересечений нет
    // - vector с 1 или 2 точками – найденные пересеченияf
    constexpr QPolygonF operator()(
        const Circle& c1,
        const Circle& c2) const {
        constexpr double EPS = 1e-10;

        const QPointF dVec = c2.center - c1.center; // вектор C1->C2
        const double d = norm(dVec);                // расстояние между центрами

        // особые случаи
        if(d <= EPS && std::abs(c1.radius - c2.radius) <= EPS) return {}; // совпадают

        if(d >= c1.radius + c2.radius - EPS || d <= std::abs(c1.radius - c2.radius) + EPS) {
            if(std::abs(d - (c1.radius + c2.radius)) <= EPS || std::abs(d - std::abs(c1.radius - c2.radius)) <= EPS) {
                // касание – одна точка
                // пересчитаем a, h (h будет ~0)
                if(d <= EPS) // точно совпали, но это уже обработано выше
                    return {c1.center};
            } else {
                return {}; // пересечений нет
            }
        }

        // теперь считаем a, h
        const double a = (c1.radius * c1.radius - c2.radius * c2.radius + d * d) / (2.0 * d);
        const double h2 = c1.radius * c1.radius - a * a; // может быть слегка отрицательным из-за округления
        const double h = h2 <= EPS ? 0.0 : std::sqrt(h2);

        // точка между центрами
        const QPointF p0 = c1.center + dVec * (a / d);

        // отступы перпендикулярно к прямой C1C2
        const double rx = -(dVec.y()) * (h / d);
        const double ry = dVec.x() * (h / d);

        QPolygonF intersections;
        // одна точка (касание)
        if(std::abs(h) <= EPS) {
            intersections.emplace_back(p0.x() + rx, p0.y() + ry);
        } else {
            intersections.emplace_back(p0.x() + rx, p0.y() + ry);
            intersections.emplace_back(p0.x() - rx, p0.y() - ry);
        }
        return intersections;
    }

    constexpr QPolygonF operator()(
        const QPointF& center1, double r1,
        const QPointF& center2, double r2) const {
        return (*this)({center1, r1}, {center2, r2});
    }

    constexpr QPolygonF operator()(
        const QPointF& center1, const QPointF& onCircle1,
        const QPointF& center2, const QPointF& onCircle2) const {
        return (*this)({center1, onCircle1}, {center2, onCircle2});
    }

} intersection;

struct {
    // Возвращает вектор пар (точка_касания_на_первой_окружности, точка_касания_на_второй_окружности)
    // для всех внутренних (перекрёстных) общих касательных двух окружностей.
    // Если касательных нет, возвращает пустой вектор.
    constexpr std::vector<std::array<QPointF, 2>> operator()(const Circle& c1, const Circle& c2) const {
        const double eps = 1e-12;

        const QPointF& O1 = c1.center;
        const QPointF& O2 = c2.center;
        const double r1 = c1.radius;
        const double r2 = c2.radius;

        const double dx = O2.x() - O1.x();
        const double dy = O2.y() - O1.y();
        const double d = std::hypot(dx, dy);

        // Концентрические окружности – общих внутренних касательных нет
        if(d < eps) return {};

        const double sumR = r1 + r2;
        // Внутренние касательные существуют только при d >= r1 + r2
        if(d < sumR - eps) return {};

        // Единичный вектор от O1 к O2
        const QPointF u = {dx / d, dy / d};
        // Перпендикуляр к u (поворот на 90°)
        const QPointF u_perp = {-u.y(), u.x()};

        // cos(θ) = (r1 + r2) / d, где θ – угол между u и направлением на точку касания
        double cosTheta = sumR / d;
        if(cosTheta > 1.0) cosTheta = 1.0; // защита от погрешности
        if(cosTheta < -1.0) cosTheta = -1.0;
        const double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

        std::vector<std::array<QPointF, 2>> result;

        // Две касательные соответствуют знакам ± sinθ
        for(int sign: {+1, -1}) {
            const double s = sign * sinTheta;
            // Направление от O1 к точке касания на первой окружности (единичный вектор)
            const QPointF n = u * cosTheta + u_perp * s;
            // Точки касания: для внутренней касательной радиусы направлены противоположно
            const QPointF T1 = O1 + n * r1;
            const QPointF T2 = O2 - n * r2;
            // При d == sumR обе итерации дают одинаковую пару – добавляем только одну
            if(sinTheta < eps && sign == -1) continue;
            result.emplace_back(std::array{T1, T2});
        }
        return result;
    }
} cross_tangent_points;

struct {
    enum Type : int {
        Out,
        On,
        In = -1
    };

    constexpr Type operator()(QPointF pt, const Circle& c, double epsilon = 1e-4) const {
        pt -= c.center;
        double distance = pt.x() * pt.x() + pt.y() * pt.y();
        double sr = c.radius * c.radius;
        if(TEST(distance, sr, epsilon)) return On;
        return distance > sr ? Out : In;
    }

    constexpr Type operator()(QPointF pt,
        QPointF center, double radius, double epsilon = 1e-4) const {
        return (*this)(pt, {center, radius}, epsilon);
    }

    constexpr Type operator()(QPointF pt,
        QPointF center, QPointF cPt, double epsilon = 1e-4) const {
        return (*this)(pt, {center, cPt}, epsilon);
    }

} is_point_oncircle;

// bool isPointOnCircleInt(int x, int y, int centerX, int centerY, int diameter) {
//     int radius = diameter / 2;
//     int dx = x - centerX;
//     int dy = y - centerY;
//     int radiusSquared = radius * radius;
//     // Проверяем точное равенство для целых чисел
//     return (dx * dx + dy * dy) == radiusSquared;
// }

//  Пересечение с прямой (бесконечной линией)
struct {
    constexpr QPolygonF operator()(Circle cir,
        QPointF lineStart, QPointF lineEnd) const {

        QPolygonF intersections;

        // Вектор направления линии
        double dx = lineEnd.x() - lineStart.x();
        double dy = lineEnd.y() - lineStart.y();

        // Переносим центр окружности в начало координат
        double fx = lineStart.x() - cir.center.x();
        double fy = lineStart.y() - cir.center.y();

        // Коэффициенты квадратного уравнения: a*t² + b*t + c = 0
        double a = dx * dx + dy * dy;
        double b = 2 * (fx * dx + fy * dy);
        double c = fx * fx + fy * fy - cir.radius * cir.radius;

        // Решаем квадратное уравнение
        double discriminant = b * b - 4 * a * c;

        if(discriminant < 0) return {}; // Нет пересечений

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

    constexpr QPolygonF operator()(QPointF center, double radius,
        QPointF lineStart, QPointF lineEnd) const {
        return (*this)({center, radius}, lineStart, lineEnd);
    }
} line_intersection;

// Пересечение с отрезком (ограниченной линией)
struct name {
    constexpr QPolygonF operator()(QPointF center, double radius, QPointF segStart, QPointF segEnd) const {
        const QPolygonF points = line_intersection(center, radius, segStart, segEnd);
        constexpr double k = 1e-6; // 1e-10;
        const double minX = std::min(segStart.x(), segEnd.x()) - k;
        const double maxX = std::max(segStart.x(), segEnd.x()) + k;
        const double minY = std::min(segStart.y(), segEnd.y()) - k;
        const double maxY = std::max(segStart.y(), segEnd.y()) + k;

        QPolygonF result;
        // Проверяем, попадают ли точки на отрезок
        for(const auto& p: points)
            // Проверяем, что точка лежит между началом и концом отрезка
            if(minX <= p.x() && p.x() <= maxX && minY <= p.y() && p.y() <= maxY)
                result.push_back(p);

        return result;
    }
    constexpr QPolygonF operator()(const Vertex& v, QPointF segStart, QPointF segEnd) {
        return (*this)(v.center, v.radius(), segStart, segEnd);
    }
} segment_intersection;

// Аппроксимация окружности по набору точек (возвращает true, если успешно)
struct {
    constexpr bool operator()(SpanPF points, QPointF& center, double& radius, double& maxError) const {
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

    constexpr auto operator()(SpanPF points, double maxError = 1e-5) {
        struct Circle {
            QPointF center;
            double radius;
            double maxError;
        } c;
        if((*this)(points, c.center, c.radius, c.maxError) && c.maxError < maxError) {
            // qWarning() << c.maxError;
            return std::optional<Circle>{c};
        } else if(!c.center.isNull()) {
            // qCritical() << c.maxError;
        }
        return std::optional<Circle>{};
    }
} fit;

// Сегментация полигона на дуги
constexpr QVector<Arc> extractArcs(const QPolygonF& polygon, double tolerance = 2.0) {
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
            if(!fit(candidate, center, radius, error) || error > tolerance) {
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

} // namespace geo::circle
