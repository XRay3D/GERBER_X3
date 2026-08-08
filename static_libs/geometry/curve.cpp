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
// module;

#include "circle.h"
#include "geo/polygon.h"
#include "gi_dbg.h"
#include "span.h"
#include <QMatrix2x2>
#include <QVector2D>
#include <algorithm>
#include <app.h>
#include <ranges>

// https://spencermortensen.com/articles/bezier-circle/

using namespace geo;
namespace c = geo ::circle;

void drawCross90(const QPointF& cl, const QColor& color = Qt::red, double l = 0.2) { // draw cross of center 90
    QPainterPath pp;
    pp.moveTo(cl + QPointF{+l, 0.});
    pp.lineTo(cl + QPointF{-l, 0.});
    pp.moveTo(cl + QPointF{0., -l});
    pp.lineTo(cl + QPointF{0., +l});
    pp.addEllipse(cl, l / 50, l / 50);
    // Gi::Debug(pp, color)->arrows = {};
}

void drawCross45(const QPointF& cl, const QColor& color = Qt::red, double l = 0.2) { // draw cross of center 90
    QPainterPath pp;
    pp.moveTo(cl + QPointF{+l, +l});
    pp.lineTo(cl + QPointF{-l, -l});
    pp.moveTo(cl + QPointF{+l, -l});
    pp.lineTo(cl + QPointF{-l, +l});
    pp.addEllipse(cl, l / 50, l / 50);
    // Gi::Debug(pp, color)->arrows = {};
}

void drawCross(const QPointF& cl, QAnyStringView str, const QColor& color = Qt::red, double l = 0.2) { // draw cross of center 90
    QPainterPath ppText;
    ppText.addText({4, -4}, {}, str.toString());
    QPainterPath pp;
    pp.moveTo(cl + QPointF{+l, 0.});
    pp.lineTo(cl + QPointF{-l, 0.});
    pp.moveTo(cl + QPointF{0., -l});
    pp.lineTo(cl + QPointF{0., +l});
    pp.addEllipse(cl, l / 50, l / 50);
    pp.addPath(QTransform::fromScale(+0.005, -0.005).map(ppText).translated(cl));
    // Gi::Debug(pp, color)->arrows = {};
}

constexpr auto DrawCross90 = std::bind(drawCross90, _1, Qt::red, 0.2);
constexpr auto DrawCross45 = std::bind(drawCross45, _1, Qt::red, 0.2);

//------------------------------------------------------------------------------
// если у сегмента есть точный индекс дуги - берём центр из реестра,
// иначе (новые точки пересечения без индекса) восстанавливаем усреднением
static QPointF reconstructCenter(Segment seg) {
    if(const int32_t idx = GetCNextIndex(seg.front())) return CenterAt(idx);
    return ~r::fold_left(v::transform(seg, GetC), Point64{}, std::plus<Point64>{}) / seg.size();
}

// максимально допустимый угол между соседними точками ОДНОЙ дуги (общий центр),
// при котором это ещё шаг линейной аппроксимации дуги, а не отрезок, перекрывший
// кусок дуги, вырезанный клиппером (в этом случае у центров - совпадение по
// индексу, но геометрически это уже хорда/линия, а не продолжение дуги)
static bool isArcStep(const QPointF& c, const QPointF& p1, const QPointF& p2) {
    constexpr double tolerance = 2.0; // запас на дрожание координат при клиппинге
    const double radius = (Length(c, p1) + Length(c, p2)) * 0.5;
    if(radius < 1e-9) return true;
    const double maxAngle = (2.0 * pi / App::settings().clpCircleSegments(radius)) * tolerance;
    double a1 = std::atan2(p1.y() - c.y(), p1.x() - c.x());
    double a2 = std::atan2(p2.y() - c.y(), p2.x() - c.x());
    double da = std::abs(a2 - a1);
    if(da > pi) da = 2.0 * pi - da;
    return da <= maxAngle;
}

// Линейная аппроксимация дугового сегмента: точки ПОСЛЕ начальной и по
// конечную включительно. Индекс центра в реестре один на всю дугу -- по нему
// обратный перевод (toCurve) и узнаёт, что цепочка точек была одной дугой,
// а не ломаной.
static Path64 arcToPath(const QPointF& to, const geo::Arc& arc, int32_t centerIdx) {
    constexpr double accuracy = 0.01;

    if(arc.radius <= accuracy) return {~to};

    const double dphi = 2.0 * acos((arc.radius - accuracy) / arc.radius);
    const int segments = std::max(1, int(ceil(std::abs(arc.theta) / dphi) * 2));

    Path64 path;
    path.reserve(segments);
    for(int i{1}; i <= segments; ++i)
        SetCIndices(path.emplace_back(~arc.pointAt(double(i) / segments)), centerIdx, centerIdx);

    // конечную точку не пересчитываем, а берём как есть -- она общая со
    // следующим сегментом; индексы стыка выставляет вызывающий toPath(),
    // т.к. он знает индекс дуги СЛЕДУЮЩЕГО сегмента
    path.back() = ~to;
    return path;
}

//------------------------------------------------------------------------------

Curve CircleCurve(double diametr, const QPointF& center) {
    // Прогибом полный оборот не выразить (tan(2*pi/4) -- бесконечность),
    // поэтому окружность -- две полуокружности: прогиб 1 у каждой.
    Curve curve{
        Vertex{{center.x() - diametr / 2, center.y()}, 1.0},
        Vertex{{center.x() + diametr / 2, center.y()}, 1.0},
    };
    curve.closed = true;
    return curve;
}

Curve ArcCurve(const QPointF& center, double radius, double startAngle, double sweep) {
    if(std::abs(sweep) >= 2.0 * pi - 1e-12) {
        Curve curve = CircleCurve(radius * 2.0, center);
        if(sweep < 0.0) curve.reverse();
        return curve;
    }

    auto at = [&](double angle) {
        return center + QPointF{radius * cos(angle), radius * sin(angle)};
    };

    const int segments = std::max(1, int(ceil(std::abs(sweep) / pi)));
    const double step = sweep / segments;
    const double bulge = geo::bulgeOf(step);

    Curve curve;
    curve.reserve(segments + 1);
    for(int i{}; i < segments; ++i)
        curve.emplace_back(at(startAngle + step * i), bulge);
    curve.emplace_back(at(startAngle + sweep));
    return curve;
}

Curve RectangleCurve(double width, double height, const QPointF& center) {
    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Curve curve{
        Vertex{{-halfWidth + center.x(), +halfHeight + center.y()}},
        Vertex{{-halfWidth + center.x(), -halfHeight + center.y()}},
        Vertex{{+halfWidth + center.x(), -halfHeight + center.y()}},
        Vertex{{+halfWidth + center.x(), +halfHeight + center.y()}},
    };
    curve.closed = true;
    return curve;
}

Curve& TransformCurve(Curve& curve, const QTransform& tr) {
    // qInfo() << tr;

    if(tr.isIdentity()) return curve;

    // Прогиб -- величина безразмерная (отношение стрелы к полухорде), так что
    // перенос, поворот и равномерный масштаб его не трогают вовсе. Зеркальное
    // же отражение разворачивает обход дуги -- знак прогиба меняется.
    for(auto& v: curve) v.pt = tr.map(v.pt);

    if((tr.m11() < .0) ^ (tr.m22() < .0))
        for(auto&& v: curve) v.bulge = -v.bulge;

    return curve;
}

// Curves& ReverseCurves(Curves& curves) {
//     r::for_each(curves.centerurves, &Curve::Reverse);
//     return curves;
// }

Curve& TranslateCurve(Curve& curve, const QPointF& pos) {
    if(!pos.isNull())
        for(auto& v: curve) v.pt += pos;
    return curve;
}

void RotateCurve(Curve& curve, double angle, const QPointF& center) {
    for(auto&& v: curve) {
        v.pt.Rotate(qDegreesToRadians(angle));
        v.pt += center;
    }
}

double Angle(double a, double b, double c) {
    // double c = Length(~*it, ~side);
    // double aa = acos((a * a + c * c - b * b) / (2 * a * c)); // Для угла α: cos(α)
    // double ab = acos((a * a + b * b - c * c) / (2 * a * b)); // Для угла β: cos(β)
    // double ac = acos((b * b + c * c - a * a) / (2 * b * c)); // Для угла γ: cos(γ)
    // После нахождения косинусов углов можно получить сами углы
    // путём нахождения арккосинусов соответствующих значений:
    // α = arccos(cos(α)), β = arccos(cos(β)), γ = arccos(cos(γ)).
    // Результаты арккосинусов будут выражены в радианах,
    // их можно перевести в градусы, умножив на (180/pi). 3
    return acos((b * b + c * c - a * a) / (2 * b * c));
}

//------------------------------------------------------------------------------

QRectF Curve::boundingRect() const {
    if(empty()) return {};

    double xMin = front().x(), xMax = xMin;
    double yMin = front().y(), yMax = yMin;
    auto include = [&](QPointF p) {
        xMin = std::min(xMin, p.x()), xMax = std::max(xMax, p.x());
        yMin = std::min(yMin, p.y()), yMax = std::max(yMax, p.y());
    };

    for(auto&& [fr, to]: segments()) {
        include(to.pt);
        auto arc = geo::arcOf(fr.pt, to.pt, fr.bulge);
        if(!arc) continue;
        // Дуга выпирает за хорду, так что одних концов мало: крайние по осям
        // точки окружности (0, 90, 180, 270 градусов) попадают в габарит,
        // только когда лежат внутри размаха самой дуги.
        for(int quadrant{}; quadrant < 4; ++quadrant) {
            const double angle = quadrant * pi / 2.0;
            // смещение от начала дуги в сторону обхода: внутри размаха,
            // если не превосходит |theta|
            double delta = std::remainder(angle - arc->startAngle, 2.0 * pi);
            if(arc->theta >= 0.0) {
                if(delta < 0.0) delta += 2.0 * pi;
                if(delta > arc->theta) continue;
            } else {
                if(delta > 0.0) delta -= 2.0 * pi;
                if(delta < arc->theta) continue;
            }
            include(arc->center + QPointF{arc->radius * cos(angle), arc->radius * sin(angle)});
        }
    }

    return QRectF{
        QPointF{xMin, yMin},
        QPointF{xMax, yMax}
    };
}

Curve& Curve::reverse() {
    // Прогиб живёт на НАЧАЛЬНОЙ вершине сегмента, поэтому при развороте он не
    // просто едет вместе с точкой: сегмент i -> i+1 становится сегментом
    // (n-1-i) -> (n-i), а обход дуги разворачивается -- отсюда и смена знака.
    if(size() < 2) return *this;

    std::vector<double> bulge{std::from_range, *this | v::transform(&Vertex::bulge)};

    if(!closed) bulge.pop_back(); // последний сегмента за собой не имеет
    r::reverse(bulge);
    // У замкнутой кривой прогиб замыкающего сегмента после разворота
    // оказывается на новой ПЕРВОЙ вершине, а не последней -- отсюда сдвиг.
    if(closed) r::rotate(bulge, bulge.begin() + 1);

    r::reverse(*this);

    for(auto&& [vertex, b]: v::zip(*this, bulge)) vertex.bulge = -b;
    if(!closed) back().bulge = {};

    return *this;
}

Curve Curve::reversed() const {
    Curve curve{*this};
    return curve.reverse(), curve;
}

double Curve::area() const {
    // Знак -- как у прежней (обратной формуле шнурков) реализации через Span:
    // у обхода против часовой стрелки в математических координатах площадь
    // ОТРИЦАТЕЛЬНА. От этого зависит isPositive() и все его читатели.
    double area{};
    for(auto&& [fr, to]: segments()) {
        area += fr.x() * to.y() - to.x() * fr.y(); // шнурки (делим на 2 в конце)
        if(auto arc = geo::arcOf(fr.pt, to.pt, fr.bulge))
            area += 2.0 * arc->segmentArea(); // «довесок» дуги над хордой
    }
    return -area / 2.0;
}

bool Curve::isClosed() const {
    return closed;
}

bool Curve::isPositive() const {
    return area() > 0.0;
}

double Curve::perimetr() const {
    double perimetr{};
    for(auto&& [fr, to]: segments()) {
        auto arc = geo::arcOf(fr.pt, to.pt, fr.bulge);
        perimetr += arc ? arc->length() : Length(fr.pt, to.pt);
    }
    return perimetr;
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
    PointF pt = center - A;
    radius = pt.length();
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
#if 1
    std::vector path{
        std::from_range,
        std::array{0.0, /*0.25,*/ 0.5, /*0.75,*/ 1.0}
            | v::transform(std::bind(bezierPoint, P0, P1, P2, P3, _1))
    };

    if(auto c = c::fit(path, eps); c) { // Circle
        center = c->center, radius = c->radius;
        return true;
    }

    // Берём три точки: начало, середина, конец
    QPointF mid = bezierPoint(P0, P1, P2, P3, 0.5);

    if(!circleCenter(P0, mid, P3, center, radius))
        return false;

    // Проверяем несколько точек на кривой
    for(double t: {0.0, /* 0.25,*/ 0.5, /*0.75,*/ 1.0}) {
        QPointF pt = bezierPoint(P0, P1, P2, P3, t) - center;
        double dist = QPointF::dotProduct(pt, pt);
        dist = std::sqrt(dist);
        if(!TEST(dist, radius, eps)) return false;
    }
#elif 0 // точки на окружности
    const qreal epsilon = 1e-10;

    // Используем все 4 точки для нахождения центра методом наименьших квадратов
    qreal sum_x = P0.x() + P1.x() + P2.x() + P3.x();
    qreal sum_y = P0.y() + P1.y() + P2.y() + P3.y();
    qreal sum_x2 = P0.x() * P0.x() + P1.x() * P1.x() + P2.x() * P2.x() + P3.x() * P3.x();
    qreal sum_y2 = P0.y() * P0.y() + P1.y() * P1.y() + P2.y() * P2.y() + P3.y() * P3.y();
    qreal sum_xy = P0.x() * P0.y() + P1.x() * P1.y() + P2.x() * P2.y() + P3.x() * P3.y();
    qreal sum_x3 = P0.x() * P0.x() * P0.x() + P1.x() * P1.x() * P1.x() + P2.x() * P2.x() * P2.x() + P3.x() * P3.x() * P3.x();
    qreal sum_y3 = P0.y() * P0.y() * P0.y() + P1.y() * P1.y() * P1.y() + P2.y() * P2.y() * P2.y() + P3.y() * P3.y() * P3.y();
    qreal sum_x2y = P0.x() * P0.x() * P0.y() + P1.x() * P1.x() * P1.y() + P2.x() * P2.x() * P2.y() + P3.x() * P3.x() * P3.y();
    qreal sum_xy2 = P0.x() * P0.y() * P0.y() + P1.x() * P1.y() * P1.y() + P2.x() * P2.y() * P2.y() + P3.x() * P3.y() * P3.y();

    qreal n = 4.0;
    qreal A = n * sum_x2 - sum_x * sum_x;
    qreal B = n * sum_xy - sum_x * sum_y;
    qreal C = n * sum_y2 - sum_y * sum_y;
    qreal D = 0.5 * (n * sum_x3 + n * sum_xy2 - sum_x * sum_x2 - sum_x * sum_y2);
    qreal E = 0.5 * (n * sum_x2y + n * sum_y3 - sum_y * sum_x2 - sum_y * sum_y2);

    qreal denom = A * C - B * B;

    if(std::abs(denom) < epsilon) return false; // Сингулярная матрица

    qreal cx = (D * C - B * E) / denom;
    qreal cy = (A * E - B * D) / denom;

    center.setX(cx);
    center.setY(cy);

    // Вычисляем радиус как среднее расстояние от центра до всех точек
    qreal r0 = std::hypot(P0.x() - cx, P0.y() - cy);
    qreal r1 = std::hypot(P1.x() - cx, P1.y() - cy);
    qreal r2 = std::hypot(P2.x() - cx, P2.y() - cy);
    qreal r3 = std::hypot(P3.x() - cx, P3.y() - cy);

    radius = (r0 + r1 + r2 + r3) / 4.0;

    if(!std::isfinite(radius) || radius <= 0) return false;
#else
    using qreal = long double;

    const qreal epsilon = 1e-10;

    // ===== МЕТОД 1: Использование касательных в конечных точках =====

    // Касательный вектор в начале (направление от P0 к P1)
    qreal t0x = P1.x() - P0.x();
    qreal t0y = P1.y() - P0.y();
    qreal t0_len = std::hypot(t0x, t0y);

    // Касательный вектор в конце (направление от P2 к P3)
    qreal t3x = P3.x() - P2.x();
    qreal t3y = P3.y() - P2.y();
    qreal t3_len = std::hypot(t3x, t3y);

    // Проверка на вырожденные случаи
    if(t0_len < epsilon) {
        qCritical() << "P0 и P1 совпадают (касательная в P0 вырождена)";
        return false;
    }
    if(t3_len < epsilon) {
        qCritical() << "P2 и P3 совпадают (касательная в P3 вырождена)";
        return false;
    }

    // Нормализуем касательные векторы
    t0x /= t0_len;
    t0y /= t0_len;
    t3x /= t3_len;
    t3y /= t3_len;

    // Радиус-векторы перпендикулярны касательным
    // Если касательная (tx, ty), то перпендикуляр (-ty, tx)
    qreal r0x = -t0y;
    qreal r0y = t0x;

    qreal r3x = -t3y;
    qreal r3y = t3x;

    // Центр окружности находится на пересечении двух прямых:
    // L1: P0 + lambda0 * (r0x, r0y)  [перпендикуляр в P0]
    // L2: P3 + lambda3 * (r3x, r3y)  [перпендикуляр в P3]
    //
    // P0 + lambda0 * (r0x, r0y) = P3 + lambda3 * (r3x, r3y)
    //
    // lambda0 * r0x - lambda3 * r3x = P3.x - P0.x
    // lambda0 * r0y - lambda3 * r3y = P3.y - P0.y
    //
    // В матричной форме:
    // | r0x  -r3x | | lambda0 |   | P3.x - P0.x |
    // | r0y  -r3y | | lambda3 | = | P3.y - P0.y |

    qreal dx = P3.x() - P0.x();
    qreal dy = P3.y() - P0.y();

    // Детерминант матрицы коэффициентов
    qreal det = r0x * (-r3y) - r0y * (-r3x);
    det = r0x * r3y - r0y * r3x;

    if(std::abs(det) < epsilon) {
        qCritical() << "Перпендикулярные биссектрисы параллельны (точки коллинеарны или дуга вырождена)";
        return false;
    }

    // Решаем систему линейных уравнений (правило Крамера)
    qreal lambda0 = (dx * r3y - dy * r3x) / det;

    // Вычисляем центр окружности
    center.setX(P0.x() + lambda0 * r0x);
    center.setY(P0.y() + lambda0 * r0y);

    // Вычисляем радиус
    radius = std::hypot(center.x() - P0.x(),
        center.y() - P0.y());

    // Проверки корректности
    if(!std::isfinite(center.x()) || !std::isfinite(center.y())) {
        qCritical() << "Центр содержит NaN или Inf";
        return false;
    }

    if(!std::isfinite(radius) || radius <= epsilon) {
        qCritical() << "Радиус некорректен";
        return false;
    }

    // Проверяем, что обе конечные точки действительно лежат на окружности
    qreal r0_actual = std::hypot(P0.x() - center.x(), P0.y() - center.y());
    qreal r3_actual = std::hypot(P3.x() - center.x(), P3.y() - center.y());

    qreal tolerance = 0.01 * radius; // 1% допуска

    if(std::abs(r0_actual - radius) > tolerance) {
        qCritical() << u"P0 не лежит на окружности (отклонение: %1)"_s.arg(std::abs(r0_actual - radius));
        return false;
    }

    if(std::abs(r3_actual - radius) > tolerance) {
        qCritical() << u"P3 не лежит на окружности (отклонение: %1)"_s.arg(std::abs(r3_actual - radius));
        return false;
    }

#endif
    return true;
}

// Вспомогательная функция: найти центр окружности по трём точкам
static bool /*findCircleCenter*/ isArcOfCircle(
    const QPointF& P0, const QPointF& P1, const QPointF& P2,
    QPointF& center, qreal& radius) {
    const qreal epsilon = 1e-10;

    using qreal = long double;
    // Вычисляем векторы
    qreal ax = P1.x() - P0.x();
    qreal ay = P1.y() - P0.y();
    qreal bx = P2.x() - P0.x();
    qreal by = P2.y() - P0.y();

    // Вычисляем расстояния в квадрате
    qreal a_sq = ax * ax + ay * ay;
    qreal b_sq = bx * bx + by * by;

    // Проверяем вырожденные случаи
    if(a_sq < epsilon || b_sq < epsilon) return false; // Одна из точек совпадает с P0

    // Вычисляем скалярное произведение
    qreal dot = ax * bx + ay * by;

    // Вычисляем 2D кросс-произведение (компонента Z)
    qreal cross = ax * by - ay * bx;

    // Проверяем, коллинеарны ли точки
    if(std::abs(cross) < epsilon) return false; // Точки лежат на одной прямой

    // Используем формулу для центра окружности через два вектора
    // Центр находится на пересечении перпендикулярных биссектрис
    qreal d = 2.0 * cross * cross; // 2 * |cross|^2

    // Параметры для вычисления центра
    qreal ux = (b_sq * ax - dot * bx) / d;
    qreal uy = (b_sq * ay - dot * by) / d;

    // Центр окружности
    center.setX(P0.x() + ux);
    center.setY(P0.y() + uy);

    // Вычисляем радиус
    radius = std::hypot(ux, uy);

    // Проверяем, что радиус конечен и положителен
    if(!std::isfinite(radius) || radius <= 0) return false;

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
    radius = Length(center, start);

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
        double d = Length(pt, center);
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

QPointF polar(QPointF p, double angle, double distance) {
    // Returns the point at a specified `angle` and `distance` from point `p`.
    return p + QPointF{cos(angle) * distance, sin(angle) * distance};
    // return QLineF::fromPolar(distance, angle).p2() + p;
}

double angle(QPointF p1, QPointF p2) {
    // Returns angle a line defined by two endpoints and x-axis in radians.
    p2 -= p1;
    return atan2(p2.y(), p2.x());
    // return QLineF{p1, p2}.angle();
}

double signedBulgeRadius(QPointF start, QPointF end, double bulge) {
    return Length(start, end) * (1.0 + (bulge * bulge)) / 4.0 / bulge;
}

std::tuple<QPointF, double, double, double> bulgeToArc(QPointF start, QPointF end, double bulge) {
    /*
    Returns arc parameters from bulge parameters.
    Based on Bulge to Arc by `Lee Mac`_.
    Args:
        start: start vertex as :class:`Vec2` compatible object
        end: end vertex as :class:`Vec2` compatible object
        bulge: bulge value
    Returns:
        Tuple: (center, start_angle, end_angle, radius)
    */

    double r = signedBulgeRadius(start, end, bulge);
    double a = angle(start, end) + ((pi / 2.0 - atan(bulge) * 2.0));
    QPointF c = polar(start, a, r);

    double a1 = angle(c, end);
    double a2 = angle(c, start);

    if(bulge < 0.0)
        return {c, a1, a2, abs(r)};
    else
        return {c, a2, a1, abs(r)};
}
//------------------------------------------------------------------------------

QPainterPath toPPath(Curve curve, std::optional<QTransform> tr, int arcOnly) {
    if(curve.size() < 2) return {};

    if(tr) TransformCurve(curve, *tr);

    QPainterPath pPath;
    pPath.moveTo(curve.front().pt);

    for(auto&& [fr, to]: curve.segments()) {
        // слишком короткую дугу рисуем хордой: и незаметно, и устойчивее
        auto arc = Length(fr.pt, to.pt) < 0.1
            ? std::nullopt
            : geo::arcOf(fr.pt, to.pt, fr.bulge);

        switch(arcOnly) {
        case 1: // Arc
            if(!arc) {
                pPath.moveTo(to.pt);
                continue;
            }
            break;
        case 2: // Line
            arc ? pPath.moveTo(to.pt)
                : pPath.lineTo(to.pt);
            continue;
        default:
            if(!arc) {
                pPath.lineTo(to.pt);
                continue;
            }
            break;
        }

        drawCross45(arc->center);

        QRectF rect{
            arc->center.x() - arc->radius,
            arc->center.y() - arc->radius,
            arc->radius * 2,
            arc->radius * 2,
        };

        // NOTE Qt stupid angles: у Qt угол растёт в сторону, противоположную
        // математической (ось Y смотрит вниз), поэтому знак размаха обратный.
        pPath.arcTo(rect,
            QLineF{arc->center, fr.pt}.angle(),
            -qRadiansToDegrees(arc->theta));
    }

    if(curve.closed && !arcOnly) pPath.closeSubpath();

    return pPath;
}

QPainterPath toPPath(const Curves& curves) {
    if(curves.empty()) return {};
    QPainterPath pPath;
    for(auto&& curve: curves) pPath.addPath(toPPath(curve));
    return pPath;
}

Curve toCurve(std::span<const QPointF> path) {
    Curve curve{
        std::from_range,
        v ::transform(path, [](const QPointF& p) -> Vertex { return {p}; }),
    };
    // повтор первой точки в конце -- как контур замыкают снаружи; у Curve для
    // этого флаг
    if(curve.size() > 2 && curve.front().pt == curve.back().pt) curve.close();
    return curve;
}

namespace {

// Промежуточное представление ТОЛЬКО для восстановления дуг из Path64.
//
// Клиппер отдаёт дуги россыпью точек с общим центром, и весь разбор ниже
// (склейка кусков, подгонка стыков) устроен вокруг «центр + направление» --
// на прогибе он бы не сошёлся: прогиб про ПАРУ точек, а тут центр известен
// раньше, чем ясно, где у дуги концы. Поэтому центр живёт до самого конца
// разбора, а в прогиб всё переводится один раз, на выходе (toBulgeCurve).
struct AVertex {
    PointF pt{}, center{};
    Vertex::Dir dir{};

    double radius() const { return dir ? Length(center, pt) : std::nan(""); }
    AVertex& setRadius(double r) {
        if(dir) {
            QLineF l{center, pt};
            l.setLength(r);
            pt = l.p2();
        }
        return *this;
    }
    operator bool() const { return dir != Vertex::Line; }
    operator QPointF() const { return pt; }
};

using ACurve = std::vector<AVertex>;

// Дуга у AVertex записана на КОНЕЧНОЙ вершине сегмента, у Vertex прогиб -- на
// начальной; здесь этот сдвиг и происходит.
Curve toBulgeCurve(const ACurve& acurve) {
    Curve curve;
    curve.reserve(acurve.size());
    for(auto&& [fr, to]: acurve | v::pairwise)
        curve.emplace_back(fr.pt,
            to.dir ? geo::bulgeOf(fr.pt, to.pt, to.center, to.dir) : 0.0);
    if(acurve.size()) curve.emplace_back(acurve.back().pt);
    return curve;
}

} // namespace

Curve toCurve(std::span<const Point64> path_) {
    if(path_.size() == 1
        || (path_.size() == 2 && path_.front() == path_.back()))
        return {
            geo::Vertex{~path_.front()}};

    bool closed = path_.front() == path_.back();
    if(closed) path_ = path_.subspan(1); // FIXME

    Path64 path{std::from_range, path_};

    if(path.empty()) return {};

    static auto eqCenter = +[](Point64& l, Point64& r) {
        // точный путь: индекс дуги, начинающейся в l, должен совпасть с индексом
        // дуги, заканчивающейся в r - если индексы известны, эпсилон не нужен
        const int32_t ln = GetCNextIndex(l), rp = GetCPrevIndex(r);
        if(ln || rp) {
            if(!ln || ln != rp) return false;
            // центры совпадают, но если между точками слишком большой угол
            // (клиппер вырезал кусок дуги между ними) - это уже хорда, а не
            // шаг аппроксимации: считаем стык линией, а не продолжением дуги
            return isArcStep(CenterAt(ln), ~l, ~r);
        }

        constexpr double epsilon = 0.1; // mm
        static const Point64 null{};
        Point64 cl = !l, cr = !r;
        return cl == cr
            && cr != null
            && TEST(cl, l, cr, r, epsilon); // radiusz
    };

    static auto notEqCenter = +[](Point64& l, Point64& r) { return !eqCenter(l, r); };

    auto segments = v::chunk_by(path, eqCenter);

    if(closed)
        if(auto it = r::adjacent_find(path, notEqCenter); it != path.end())
            r::rotate(path, ++it);

    {                                     // Исправление центров
        constexpr double epsilon = 0.001; // mm, допуск совпадения центров дуг

        // Исправление пляшущих дуг после объединения открытых концов линий:
        // p2 - точка БЕЗ индекса (новая точка пересечения, добавленная клиппером),
        // зажатая между p1 и p3, у которых индекс НЕИЗВЕСТНОЙ ими самими, но ОБЩЕЙ
        // дуги уже точно известен - если p2 к тому же лежит на той же окружности,
        // она однозначно (без гадания по координатам) принадлежит этой дуге
        auto fixClippedArcs = [](Point64& p1, Point64& p2, Point64& p3) {
            if(GetCPrevIndex(p2) || GetCNextIndex(p2)) return; // у p2 уже есть точная информация

            const int32_t idx = GetCNextIndex(p1);
            if(!idx || idx != GetCPrevIndex(p3)) return; // p1/p3 - не одна и та же дуга

            const QPointF c = CenterAt(idx), p = ~p2;
            if(TEST(c, ~p1, c, p, epsilon) && TEST(c, ~p3, c, p, epsilon))
                SetCIndices(p2, idx, idx);
        };
        for(auto&& [p1, p2, p3]: v::adjacent<3>(path))
            fixClippedArcs(p1, p2, p3);
        if(closed && path.size() > 2) {
            fixClippedArcs(path.back(), path[0], path[1]);
            fixClippedArcs(path.end()[-2], path.back(), path[0]);
        }
        segments = v::chunk_by(path, eqCenter); // Update segments

        using PSpan = std::span<Point64>;

        // Исправление единичных точек без индекса рядом с дугой: если одиночная
        // точка (новое пересечение) вплотную примыкает к известной дуге и лежит
        // на той же окружности - присоединяем её к этой дуге по точному индексу
        auto fixCenterS2 = [](PSpan s1, PSpan s2) -> bool {
            if(s1.size() > 1 && s2.size() == 1
                && !GetCPrevIndex(s2.front()) && !GetCNextIndex(s2.front())) {
                if(const int32_t idx = GetCNextIndex(s1.back())) {
                    const QPointF c = CenterAt(idx);
                    if(TEST(c, ~s1.back(), c, ~s2.front(), epsilon)) {
                        SetCIndices(s2.front(), idx, idx);
                        return true;
                    }
                }
            }
            if(s1.size() == 1 && s2.size() > 1
                && !GetCPrevIndex(s1.front()) && !GetCNextIndex(s1.front())) {
                if(const int32_t idx = GetCPrevIndex(s2.front())) {
                    const QPointF c = CenterAt(idx);
                    if(TEST(c, ~s2.front(), c, ~s1.front(), epsilon)) {
                        SetCIndices(s1.front(), idx, idx);
                        return true;
                    }
                }
            }
            return false;
        };
        int ctr;
        do { // TODO переписать на однопроходный?
            ctr = 0;
            for(auto&& sTypl: v::pairwise(segments))
                ctr += std::apply(fixCenterS2, sTypl);
            ctr += fixCenterS2(segments.back(), segments.front());
            segments = v::chunk_by(path, eqCenter); // Update segments
            // qCritical() << "ctr" << ctr;
        } while(ctr && segments.front().size() != path.size());
    }

    if(closed && segments.front().size() == path.size()) { // Circle
        // qCritical() << "circle" << path.size();
        QPointF center = ~!path.front();
        auto dir = DIR(~path[0], center, ~path[1]);
        double radius = get_roundest(v::transform(segments.front(), Radius));
        Curve curve = CircleCurve(radius * 2, center);
        if(dir == Vertex::Cw) curve.reverse();
        assert(closed && curve.isClosed());
        return curve;
    }

    struct SegData : Segment {
        SegData(Segment seg)
            : Segment{seg}
            , center{reconstructCenter(seg)}
            , radius{get_roundest(v::transform(seg, Radius))} { assert(seg.size()); } // "denoised" radius
        QPointF center;
        double radius;
        auto& operator[](int i) const noexcept {
            return i < 0 ? rbegin()[-i]
                         : at(i);
        };
        bool isArc() const noexcept { return size() > 1; }
        operator bool() const noexcept { return isArc(); }
        AVertex vtx() const noexcept { return {~front()}; }
        AVertex vtx1() const noexcept {
            if(!isArc()) return {~front()};
            QLineF l{center, ~front()};
            l.setLength(radius);
            return {l.p2()};
        }
        AVertex vtx2() const noexcept {
            if(isArc()) return AVertex{~back(), center, DIR(*this)}.setRadius(radius);
            return {~front()};
        }
    };

    const std::vector sds{
        std::from_range,
        v::transform(segments, +[](Segment&& seg) -> SegData { return {seg}; }),
    };

    // fill curve
    ACurve curve;
    for(const SegData& sd: sds) {
        if(sd) { // arcTo
            curve.emplace_back(sd.vtx());
            curve.emplace_back(sd.vtx2());
        } else { // lineTo
            curve.emplace_back(sd.vtx());
        }
    }

    // fix arcs
    using VSpan = std::span<AVertex>;
    using SDSpan = std::span<const SegData>;
    // using SDSpan = decltype((tmp.cbegin()));
    // auto sd = tmp.begin();
    // for(VSpan vs: v::chunk_by(curve, [](AVertex&, AVertex& r) -> bool { return r; })) {
    //     // qWarning() << vs.size() << !!vs.front() << !!vs.back();
    // }

    enum Joins {
        L, // line
        C, // curve
        LLL = L,
        LLC,
        LCL,
        LCC,
        CLL,
        CLC,
        CCL,
        CCC,
    };

    // index into `fixers` below: v0/v1/v2 each contribute their L/C bit, matching the Joins enum
    constexpr static auto joinKind = +[](VSpan v0, VSpan v1, VSpan v2) -> Joins {
        return Joins(v0.back() * 4 + v1.back() * 2 + v2.back() * 1);
    };

    constexpr static auto fixCC = +[](VSpan v0, VSpan v1, SDSpan sd) {
        QPolygonF pts = c::intersection(v0[1], v1[1]);
        if(pts.size() == 1) { // circles touch/cross at a single point
            v0[1].pt = v1[0].pt = pts.front();
        } else if(pts.size() > 1 && TEST(pts.front(), pts.back(), 0.01)) { // two intersections collapse to ~one point
            v0[1].pt = r::min(pts, {}, std::bind(Length, _1, v0[1]));
            v1[0].pt = r::min(pts, {}, std::bind(Length, _1, v1[0]));
        } else if(pts.size() > 1 && (TEST(pts.front(), v0[1], 0.01) || TEST(v1[0], pts.back(), 0.01))) { // one root already matches an endpoint
            v1[0].pt = v0[1].pt = r::min(pts, {}, std::bind(Length, _1, v0[1]));
        } else if(TEST(v0[1].center, v1[1].center, sd[0].radius + sd[1].radius, 0.01)) { // circles externally tangent within tolerance, but not exactly touching
            auto tangents = c::cross_tangent_points(v0[1], v1[1]);
            if(tangents.size() == 2) {
                auto& best = Length(tangents[0][0], v0[1]) + Length(tangents[0][1], v1[0])
                        <= Length(tangents[1][0], v0[1]) + Length(tangents[1][1], v1[0])
                    ? tangents[0]
                    : tangents[1];
                v0[1].pt = best[0];
                v1[0].pt = best[1];
            }
        }
    };

    constexpr static auto fixLCL = +[](AVertex& Cir, AVertex& fixC,
                                        AVertex& L0, AVertex& fixL, double radius) {
        QPolygonF pts = c::line_intersection(Cir, L0, fixL);
        // for(QPointF pt: pts) drawCross(pt, "LCL", Qt::white);
        // Пересечений может не быть вовсе -- тогда подгонять стык не к чему.
        // (Раньше в этом случае в дело шла точка по умолчанию, то есть начало
        // координат, и стык рядом с ним «притягивался» к нулю.)
        if(pts.size()) {
            QPointF pt = r::min(pts, {}, std::bind(Length, _1, fixL));
            if(TEST(pt, fixL, 0.01)) { // FIXME
                fixL.pt = fixC.pt = pt;
                // drawCross(fixC, "LCL", Qt::yellow);
                return true;
            }
        }
        if(TEST(Cir.center, fixL, Cir.radius(), 0.01)) {
            // drawCross(fixC, "LCL", Qt::magenta);
            return true;
            fixC.pt = fixL;
            fixC.setRadius(radius);
            fixL.pt = fixC;
        }
        // drawCross(fixC, "LCL", Qt::red);
        return false;
    };

    constexpr static auto fixCLL = +[](VSpan v0, VSpan v1, VSpan v2, SDSpan sd) {
        fixLCL(v0[1], // Circle
            v0[1],    // fix Circle
            v2[0],    // L0
            v1[0],    // fixL
            sd[0].radius);
    };

    constexpr static auto fixLLC = +[](VSpan v0, VSpan v1, VSpan v2, SDSpan sd) {
        fixLCL(v2[1], // Circle
            v2[0],    // fix Circle
            v0[0],    // L0
            v1[0],    // fixL
            sd[2].radius);
    };

    constexpr std::array fixers{
        +[/*fixLLL*/](VSpan, VSpan, VSpan, SDSpan) {}, // do nothing

        fixLLC,
        +[/*fixLCL*/](VSpan v0, VSpan v1, VSpan v2, SDSpan sd) { // TODO???
            // qWarning() << "LCL" << v0.size() << v1.size() << v2.size() << sd.size();
            (void)(v0.size() + v1.size() + v2.size() + sd.size());
        },
        +[/*fixLCC*/](VSpan /*v0*/, VSpan v1, VSpan v2, SDSpan sd) {
            fixCC(v1, v2, sd.subspan(1));
        },
        fixCLL,
        +[/*fixCLC*/](VSpan v0, VSpan v1, VSpan v2, SDSpan sd) {
            fixCLL(v0, v1, v2.subspan(0, 1), sd);
            fixLLC(v0.subspan(1, 1), v1, v2, sd);
        },
        +[/*fixCCL*/](VSpan v0, VSpan v1, VSpan /*v2*/, SDSpan sd) {
            fixCC(v0, v1, sd.subspan(0, 2));
        },
        +[/*fixCCC*/](VSpan v0, VSpan v1, VSpan v2, SDSpan sd) {
            fixCC(v0, v1, sd);
            fixCC(v1, v2, sd.subspan(1));
        },
    };

    if(1) {
        bool fl{};
        auto adjacent = v::adjacent<3>(v::chunk_by(curve, [](AVertex&, AVertex& r) -> bool { return r; }));
        for(auto* sd = sds.data(); auto [v0, v1, v2]: adjacent)
            fl |= 1, fixers[joinKind(v0, v1, v2)](v0, v1, v2, {sd, sd++ + 3});

        if(fl && closed) { // Close
            auto [vf0, vf1, vf2] = adjacent.front();
            auto [vb0, vb1, vb2] = adjacent.back();
            fixers[joinKind(vb2, vf0, vf1)](vb2, vf0, vf1, std::array{sds.back(), sds.front(), sds[1]});
            fixers[joinKind(vb1, vb2, vf0)](vb1, vb2, vf0, std::array{sds.end()[-2], sds.back(), sds.front()});
        }

        size_t s;
        do { // clean duplicates after fixers
            s = curve.size();
            for(auto it = curve.begin(); it != curve.end();) {
                AVertex& v1 = *it++;
                if(it == curve.end()) break;
                AVertex& v2 = *it;
                if(v1.pt == v2.pt) it = curve.erase(v1 ? it : (--it));
            }
        } while(s != curve.size());
    }

    Curve result = toBulgeCurve(curve);
    result.closed = closed;
    return result;
}

Curves toCurves(std::span<const Path64> paths) {
    constexpr auto size = std::bind(&Path64::size, _1);
    return {
        std::from_range,
        v::transform(v::filter(paths, size), qOverload<std::span<const Point64>>(toCurve)),
    };
}

Curvess toCurvess(std::span<const Paths64> pathss) {
    constexpr auto size = std::bind(&Paths64::size, _1);
    return {
        std::from_range,
        v::transform(v::filter(pathss, size), qOverload<std::span<const Path64>>(toCurves)),
    };
}

Path64 toPath(const Curve& curve) {
    // qInfo() << "toPath";

    if(curve.empty()) return {};

    const bool closed = curve.closed;
    const size_t n = curve.size();
    const size_t segs = curve.segmentCount();

    // Геометрия и индекс центра каждого сегмента (nullopt/0, если сегмент --
    // отрезок). Центр регистрируется один раз на дугу и переиспользуется для
    // всех её точек в arcToPath: по нему обратный перевод и опознаёт дугу.
    std::vector<std::optional<geo::Arc>> segArc(segs);
    std::vector<int32_t> segCenter(segs, 0);
    for(size_t i{}; i < segs; ++i)
        if((segArc[i] = curve.arcAt(i)))
            segCenter[i] = RegisterCenter(segArc[i]->center);

    Path64 path{~curve.front().pt};
    if(segs) SetCIndices(path.back(), 0, segCenter[0]); // дуга, начинающаяся в первой точке

    for(size_t i{}; i < segs; ++i) {
        const QPointF& to = curve[(i + 1) % n].pt;
        if(segArc[i])
            path.append_range(arcToPath(to, *segArc[i], segCenter[i]));
        else
            path.emplace_back(~to);
        // точка стыка: индекс дуги, ЗАКОНЧИВШЕЙСЯ здесь, и индекс дуги, НАЧИНАЮЩЕЙСЯ здесь
        const int32_t nextIdx = (i + 1 < segs) ? segCenter[i + 1] : 0;
        SetCIndices(path.back(), segCenter[i], nextIdx);
    }

    // CL2::TrimCollinear(path, !closed);
    // CL2::StripDuplicates(path, closed);
    // CL2::StripNearEqual(path, uScale, closed);

    // Замкнутость выражена повтором первой точки: другого способа сообщить её
    // клипперу (и обратному переводу toCurve) в Path64 просто нет.
    if(closed && path.back() != path.front())
        path.emplace_back(path.front());

    // Gi::Debug({path}, {255, 0, 255, 128});

    return path;
}

Paths64 toPaths(const Curves& curves) {
    Paths64 paths{std::from_range, curves | v::transform(toPath)};
    return paths;
}

Curves toCurves(const QPainterPath& pPath) {
    using QPP = QPainterPath;
    using El = QPP::Element;
    Curves curves;
    for(auto&& elements: v::iota(0, pPath.elementCount())
            | v::transform(std::bind(&QPP::elementAt, pPath, _1))            // to Element
            | v::chunk_by(+[](const El&, const El& r) { return r.type; })) { // to Subpath Polygons

        // qInfo() << "elements" << elements.size();                         // count of elements

        constexpr auto splitPaths = +[](const El&, const El& r) { return r.type > QPP::CurveToElement; };
        auto subpaths = v::chunk_by(elements, splitPaths); // separate Curves

        Curve curve;
        for(auto&& [from, to]: v::pairwise(subpaths)) { // 'from' point to bezier Point64 in 'to' if
            if(curve.empty()) curve.emplace_back(static_cast<QPointF>(from.front()));
            if(to.front().type == QPP::CurveToElement) { // is arcTo
                // Проверка, является ли кривая Безье дугой окружности
                QPointF center;
                double radius;
                if(isArcOfCircle(from.back(), to[0], to[1], to[2], center, radius, 5e-3)) {
                    // qInfo() << "Arc 1" << radius << center;
                    // прогиб пишется на НАЧАЛЬНОЙ вершине дуги - она уже в кривой
                    curve.back().bulge = geo::bulgeOf(curve.back().pt, to.back(), center,
                        DIR(from.back(), center, to.back()));
                    curve.emplace_back(static_cast<QPointF>(to.back()));
                } else {                          // is lineTo
                    drawCross45(center, Qt::red); // debug
                    curve.emplace_back(static_cast<QPointF>(to.back()));
                }
            } else
                curve.emplace_back(static_cast<QPointF>(to.front()));
        }
        if(curve.size()) curves.emplace_back(std::move(curve));
    }
    return curves;
}

Curves& TransformCurves(Curves& curves, const QTransform& tr) {
    r::for_each(curves, std::bind(TransformCurve, _1, tr));
    return curves;
}

QIcon drawIcon(const Curves& curves, QColor color) {
    return drawIcon(toPPath(curves), color);
}

namespace cl = Clipper2Lib;

// Clipper отдаёт полигоны с неявным замыканием (последняя точка != первой).
// Замыкаем явно: иначе toCurve() сочтёт контур открытым и результат нельзя
// будет подать обратно ни в булеву операцию, ни в offset.
static Paths64& closePaths(Paths64& paths) {
    for(Path64& path: paths)
        if(path.size() > 2 && path.front() != path.back())
            path.emplace_back(path.front());
    return paths;
}

Curves BoolOp_::operator()(ClipType ct, FillRule fr,
    const Curves& subjects, const Curves& clips) {
    Paths64 result;
    cl::Clipper64 clipper;
    clipper.AddSubject({
        std::from_range,
        subjects | v::filter(&Curve::isClosed) | v::transform(toPath),
    });
    clipper.AddOpenSubject({
        std::from_range,
        subjects | v::filter(std::not_fn(&Curve::isClosed)) | v::transform(toPath),
    });
    clipper.AddClip(toPaths(clips));
    clipper.Execute(
        static_cast<cl::ClipType>(ct),
        static_cast<cl::FillRule>(fr), result);
    return toCurves(closePaths(result));
}

void BoolOp_::operator()(ClipType ct, FillRule fr,
    const Curves& subjects, const Curves& clips, CurveTree& solution) {
    Paths64 sol_open;
    cl::Clipper64 clipper;
    clipper.AddSubject({
        std::from_range,
        subjects | v::filter(&Curve::isClosed) | v::transform(toPath),
    });
    clipper.AddOpenSubject({
        std::from_range,
        subjects | v::filter(std::not_fn(&Curve::isClosed)) | v::transform(toPath),
    });
    clipper.AddClip(toPaths(clips));
    cl::PolyTree64 polytree; // solution
    clipper.Execute(
        static_cast<cl::ClipType>(ct),
        static_cast<cl::FillRule>(fr), polytree, sol_open);
}

Curves BoolOp_::Intersect(const Curves& subjects, const Curves& clips,
    FillRule fr) {
    return operator()(ClipType::Intersection, fr, subjects, clips);
}

Curves BoolOp_::Union(const Curves& subjects, const Curves& clips,
    FillRule fr) {
    return operator()(ClipType::Union, fr, subjects, clips);
}

Curves BoolOp_::Union(const Curves& subjects, FillRule fr) {
    Paths64 result;
    cl::Clipper64 clipper;
    clipper.AddSubject({
        std::from_range,
        subjects | v::filter(&Curve::isClosed) | v::transform(toPath),
    });
    clipper.Execute(cl::ClipType::Union, static_cast<cl::FillRule>(fr), result);
    return toCurves(closePaths(result));
}

Curves BoolOp_::Difference(const Curves& subjects, const Curves& clips,
    FillRule fr) {

    return operator()(ClipType::Difference, fr, subjects, clips);
}

Curves BoolOp_::Xor(const Curves& subjects, const Curves& clips, FillRule fr) {

    return operator()(ClipType::Xor, fr, subjects, clips);
}

Curves Inflate64::operator()(const Curves& paths, double delta,
    JoinType jt, EndType et,
    double miterLimit, double arcTolerance) {

    if(delta == 0.0) return paths;
    if(!arcTolerance) arcTolerance = delta * 1e-3;
    return PathsZ(paths, delta * 0.5, jt, et, miterLimit, arcTolerance);

    // cl::ClipperOffset clip_offset(miter_limit, arcTolerance);
    // clip_offset.AddPaths(toPaths(paths), static_cast<cl::JoinType>(jt), static_cast<cl::EndType>(et));
    // Paths64 solution;
    // clip_offset.Execute(delta * uScale, solution);
    // return toCurves(solution);
}

Curves Inflate64::PathsZ(const Curves& paths, double delta,
    JoinType jt, EndType et,
    double miterLimit, double arcTolerance) {
    // roundJoinCache.clear();
    // Paths64 input = paths;
    // PrepareCornersForOffset(input, jt);
    // cl::ClipperOffset offset{miterLimit, arcTolerance};
    // offset.SetZCallback(UpdateCenter);
    // offset.AddPaths(input, jt, et);
    // Paths64 solution;
    // offset.Execute(delta, solution);
    // return solution;

    cl::ClipperOffset clip_offset(miterLimit * uScale, arcTolerance * uScale);
    clip_offset.AddPaths(toPaths(paths), static_cast<cl::JoinType>(jt), static_cast<cl::EndType>(et));
    Paths64 solution;
    clip_offset.Execute(delta * uScale, solution);
    return toCurves(closePaths(solution));
}

Curves Inflate64::RoundPolygon(const Curves& paths,
    double delta, double miterLimit, double arcTolerance) {

    return PathsZ(paths, delta * 0.5,
        JoinType::Round, EndType::Polygon, miterLimit, arcTolerance);
}

Curves Inflate64::MiterPolygon(const Curves& paths,
    double delta, double miterLimit, double arcTolerance) {

    return PathsZ(paths, delta * 0.5,
        JoinType::Miter, EndType::Polygon, miterLimit, arcTolerance);
}
