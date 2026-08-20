// Вспомогательная обвязка: примитивы, математика дуг, трансформации, разбор
// пути Qt. Проверяется тем, что известно аналитически, -- площадью, периметром
// и габаритом, а не покоординатным сличением.

#include "geo/polygon.h"
#include "geo/util.h"

#include <QPainterPath>
#include <QTest>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

using namespace Geo;

namespace {
constexpr double eps = 1e-9;

bool near(double a, double b, double tolerance = eps) { return std::abs(a - b) <= tolerance; }
} // namespace

class UtilTest : public QObject {
    Q_OBJECT

private slots:
    void primitivesMatchAnalyticShapes();
    void arcSplitsSweepAndClosesFullCircle();
    void bulgeMathRoundTrips();
    void transformKeepsShapeAndFlipsOnMirror();
    void similarityRejectsNonUniformScale();
    void closeAndReversedKeepArcs();
    void fromPathRecognisesArcsOfEllipse();
    void transformedPolygonKeepsOrientationCanon();
    void decimatedKeepsShapeWithinTolerance();
};

void UtilTest::primitivesMatchAnalyticShapes() {
    constexpr double d = 8.0, r = d / 2.0;
    const Polyline c = circle(d, {3.0, -2.0});
    QVERIFY(c.isClosed());
    QCOMPARE(c.size(), std::size_t(2)); // полный оборот прогибом не выражается -- две полуокружности
    QVERIFY(near(c.area(), pi * r * r, 1e-9));
    QVERIFY(near(c.perimeter(), 2.0 * pi * r, 1e-9));
    QVERIFY(c.isPositive()); // канон внешней границы -- против часовой стрелки
    const QRectF box = c.boundingRect();
    QVERIFY(near(box.width(), d, 1e-9) && near(box.height(), d, 1e-9));

    const Polyline rect = rectangle(10.0, 4.0, {1.0, 1.0});
    QVERIFY(rect.isClosed() && rect.isPositive());
    QVERIFY(near(rect.area(), 40.0));
    QVERIFY(near(rect.perimeter(), 28.0));

    // Стадион: прямая часть плюс два полукруга.
    constexpr double w = 10.0, h = 4.0, rr = h / 2.0;
    const Polyline ob = obround(w, h);
    QVERIFY(ob.isClosed() && ob.isPositive());
    QVERIFY(near(ob.area(), (w - h) * h + pi * rr * rr, 1e-9));
    QVERIFY(near(ob.perimeter(), 2.0 * (w - h) + 2.0 * pi * rr, 1e-9));
    // Вырожденный случай -- окружность, а не прямоугольник с нулевыми прямыми.
    QVERIFY(near(obround(6.0, 6.0).area(), pi * 9.0, 1e-9));
}

void UtilTest::arcSplitsSweepAndClosesFullCircle() {
    constexpr double r = 5.0;

    const Polyline quarter = arc({}, r, 0.0, pi / 2.0);
    QVERIFY(!quarter.isClosed());
    QVERIFY(near(quarter.perimeter(), r * pi / 2.0, 1e-9));

    // Больше полуокружности -- режется, иначе прогиб уходит в бесконечность.
    const Polyline threeQuarters = arc({}, r, 0.0, 1.5 * pi);
    QVERIFY(threeQuarters.size() > 2);
    QVERIFY(near(threeQuarters.perimeter(), r * 1.5 * pi, 1e-9));
    for(const Vertex& v: threeQuarters)
        QVERIFY(std::abs(v.bulge) <= 1.0 + eps); // ни один кусок не длиннее полуокружности

    const Polyline full = arc({}, r, 0.0, 2.0 * pi);
    QVERIFY(full.isClosed());
    QVERIFY(near(full.area(), pi * r * r, 1e-9));
    QVERIFY(full.isPositive());

    const Polyline reversedFull = arc({}, r, 0.0, -2.0 * pi);
    QVERIFY(reversedFull.isClosed());
    QVERIFY(!reversedFull.isPositive()); // отрицательный размах -- обход по часовой

    // Полный оборот начинается ТАМ, ГДЕ ЗАКАЗАНО: вызывающий стыкует с началом
    // обхода остальной контур (в Gerber окружность приходит серединой пути), и
    // подменять его точкой на оси нельзя.
    for(const double start: {0.0, pi / 3.0, -2.0}) {
        const Polyline turn = arc({3.0, -1.0}, r, start, 2.0 * pi);
        const QPointF expected{3.0 + r * std::cos(start), -1.0 + r * std::sin(start)};
        QVERIFY(near(QPointF(turn.front()).x(), expected.x(), 1e-9));
        QVERIFY(near(QPointF(turn.front()).y(), expected.y(), 1e-9));
        QVERIFY(near(turn.area(), pi * r * r, 1e-9));
    }
}

void UtilTest::bulgeMathRoundTrips() {
    const QPointF p1{0.0, 0.0}, p2{10.0, 0.0};
    constexpr double bulge = 0.4;

    const auto a = arcOf(p1, p2, bulge);
    QVERIFY(a.has_value());
    QVERIFY(near(a->theta, 4.0 * std::atan(bulge)));
    QVERIFY(near(a->radius * std::abs(a->theta), Polyline{
                                                     Vertex{p1, bulge},
                                                     Vertex{p2}
    }
                                                     .perimeter(),
        1e-9));
    // Концы дуги лежат на ней самой.
    QVERIFY(near(a->pointAt(0.0).x(), p1.x(), 1e-9) && near(a->pointAt(0.0).y(), p1.y(), 1e-9));
    QVERIFY(near(a->pointAt(1.0).x(), p2.x(), 1e-9) && near(a->pointAt(1.0).y(), p2.y(), 1e-9));

    // Обратная задача: по центру и направлению получается тот же прогиб.
    QVERIFY(near(bulgeOf(p1, p2, a->center, a->dir()), bulge, 1e-9));

    QVERIFY(!arcOf(p1, p2, 0.0).has_value());   // прямая
    QVERIFY(!arcOf(p1, p1, bulge).has_value()); // вырожденная хорда
}

void UtilTest::transformKeepsShapeAndFlipsOnMirror() {
    Polyline poly = obround(10.0, 4.0);
    const double area = poly.area(), perimeter = poly.perimeter();

    rotate(poly, 37.0, {2.0, 3.0});
    QVERIFY(near(poly.area(), area, 1e-9));
    QVERIFY(near(poly.perimeter(), perimeter, 1e-9));
    QVERIFY(poly.isPositive()); // поворот ориентацию не трогает

    Polyline moved = rectangle(4.0, 2.0);
    translate(moved, {5.0, -7.0});
    QVERIFY(near(moved.boundingRect().center().x(), 5.0, 1e-9));
    QVERIFY(near(moved.boundingRect().center().y(), -7.0, 1e-9));

    // Зеркало сохраняет форму, но разворачивает обход дуг.
    Polyline mirrored = obround(10.0, 4.0);
    const double bulgeBefore = mirrored[1].bulge;
    transform(mirrored, QTransform::fromScale(-1.0, 1.0));
    QVERIFY(near(mirrored.area(), area, 1e-9));
    QVERIFY(near(mirrored.perimeter(), perimeter, 1e-9));
    QVERIFY(near(mirrored[1].bulge, -bulgeBefore, 1e-9));
    QVERIFY(!mirrored.isPositive());

    // Равномерный масштаб: площадь по квадрату коэффициента, периметр -- линейно.
    Polyline scaled = obround(10.0, 4.0);
    transform(scaled, QTransform::fromScale(3.0, 3.0));
    QVERIFY(near(scaled.area(), area * 9.0, 1e-8));
    QVERIFY(near(scaled.perimeter(), perimeter * 3.0, 1e-9));
}

void UtilTest::similarityRejectsNonUniformScale() {
    QVERIFY(isSimilarity(QTransform{}));
    QVERIFY(isSimilarity(QTransform::fromScale(2.0, 2.0)));
    QVERIFY(isSimilarity(QTransform::fromScale(-1.0, 1.0))); // зеркало -- тоже подобие
    QVERIFY(isSimilarity(QTransform{}.rotate(30.0)));
    QVERIFY(isSimilarity(QTransform::fromTranslate(5.0, 5.0)));
    QVERIFY(!isSimilarity(QTransform::fromScale(2.0, 3.0))); // дуга стала бы эллипсом
    QVERIFY(!isSimilarity(QTransform{}.shear(0.5, 0.0)));

    QVERIFY(hasArcs(circle(4.0)));
    QVERIFY(!hasArcs(rectangle(4.0, 4.0)));
}

void UtilTest::closeAndReversedKeepArcs() {
    // Контур с повторённой замыкающей точкой -- ровно то, что приносят парсеры.
    Polyline poly{
        Vertex{0.0, 0.0},
        Vertex{10.0, 0.0},
        Vertex{10.0, 10.0, 1.0},
        Vertex{0.0, 0.0}
    };
    poly.close();
    QVERIFY(poly.isClosed());
    QCOMPARE(poly.size(), std::size_t(3));
    QVERIFY(near(poly[2].bulge, 1.0)); // прогиб замыкающего сегмента остался на месте

    const double area = poly.area();
    const Polyline back = poly.reversed();
    QVERIFY(near(back.area(), area, 1e-9));
    QVERIFY(back.isPositive() != poly.isPositive());
    QVERIFY(near(poly[2].bulge, 1.0)); // reversed() не трогает исходник
}

void UtilTest::fromPathRecognisesArcsOfEllipse() {
    constexpr double r = 6.0;
    QPainterPath path;
    path.addEllipse(QPointF{1.0, 2.0}, r, r);

    const Polylines contours = fromPath(path);
    QCOMPARE(contours.size(), std::size_t(1));
    const Polyline& c = contours.front();
    QVERIFY(c.isClosed());
    // Каждый кубик Qt опознан как дуга, а не раздроблен на отрезки.
    QCOMPARE(c.size(), std::size_t(4));
    QVERIFY(hasArcs(c));
    QVERIFY(near(c.area(), pi * r * r, 1e-3));
    QVERIFY(near(c.perimeter(), 2.0 * pi * r, 1e-3));

    QPainterPath rectPath;
    rectPath.addRect(QRectF{0.0, 0.0, 8.0, 3.0});
    const Polylines rectContours = fromPath(rectPath);
    QCOMPARE(rectContours.size(), std::size_t(1));
    QVERIFY(rectContours.front().isClosed());
    QCOMPARE(rectContours.front().size(), std::size_t(4));
    QVERIFY(near(rectContours.front().area(), 24.0, 1e-9));
}

void UtilTest::transformedPolygonKeepsOrientationCanon() {
    Polyline outer = rectangle(20.0, 20.0);
    Polyline hole = circle(8.0);
    hole.reverse(); // дырка -- по часовой стрелке
    const Polygon polygon{outer, {hole}};
    const double area = polygon.area();
    QVERIFY(near(area, 400.0 - pi * 16.0, 1e-6));

    const Polygon rotated = transformed(polygon, QTransform{}.rotate(23.0));
    QVERIFY(near(rotated.area(), area, 1e-6));

    // Зеркало меняет знак обхода у КАЖДОГО контура, и без восстановления канона
    // внешняя граница стала бы дыркой, а дырка -- телом.
    const Polygon mirrored = transformed(polygon, QTransform::fromScale(-1.0, 1.0));
    QVERIFY(near(mirrored.area(), area, 1e-6));
    QVERIFY(mirrored.outer().isPositive());
    QCOMPARE(mirrored.holes().size(), std::size_t(1));
    QVERIFY(!mirrored.holes().front().isPositive());

    const Polygons region = transformed(Polygons{polygon}, QTransform::fromScale(-1.0, 1.0));
    QVERIFY(near(region.area(), area, 1e-6));
}

void UtilTest::decimatedKeepsShapeWithinTolerance() {
    constexpr double tolerance = 0.1;

    // Дрожащий квадрат: каждая сторона порезана на полсотни сегментов с
    // поперечным шумом мельче половины допуска. Прореживание обязано снять
    // весь шум, не сдвинув границу дальше допуска.
    Polyline noisy;
    noisy.closed = true;
    const QPointF corners[]{{0.0, 0.0}, {20.0, 0.0}, {20.0, 20.0}, {0.0, 20.0}};
    for(int side{}; side < 4; ++side) {
        const QPointF from = corners[side], to = corners[(side + 1) % 4];
        const QPointF dir = (to - from) / 50.0;
        const QPointF normal{-dir.y() / 0.4, dir.x() / 0.4}; // единичная поперечная
        for(int k{}; k < 50; ++k)
            noisy.emplace_back(from + dir * k + normal * (0.02 * std::sin(k * 12.9898)), 0.0);
    }
    const Polyline thinned = decimated(noisy, tolerance);
    QVERIFY(thinned.closed);
    QVERIFY(thinned.size() < noisy.size() / 10); // шум снят, осталась горстка вершин
    QVERIFY(near(thinned.area(), 400.0, tolerance * noisy.perimeter()));
    // Каждая исходная вершина лежит не дальше допуска от прореженной границы
    // (все сегменты прямые -- расстояние до хорды считается напрямую).
    for(const Vertex& v: noisy) {
        double best = std::numeric_limits<double>::max();
        for(const auto& [a, b]: segments(thinned)) {
            const QPointF ab = QPointF(b) - QPointF(a);
            const double len2 = ab.x() * ab.x() + ab.y() * ab.y();
            const double t = std::clamp(
                ((v.x() - a.x()) * ab.x() + (v.y() - a.y()) * ab.y()) / len2, 0.0, 1.0);
            best = std::min(best, distance(v, QPointF(a) + ab * t));
        }
        QVERIFY(best <= tolerance + eps);
    }

    // Мелкие дуги спрямляются: скругление углов с сагиттой мельче половины
    // допуска (четверть окружности r = 0.05: сагитта ~0.015) теряет прогиб,
    // форма остаётся в допуске.
    constexpr double r = 0.05;
    const double b = std::tan(pi / 8.0); // четверть окружности против часовой
    Polyline rounded{
        Vertex{r, 0.0},               // низ
        Vertex{20.0 - r, 0.0, b},     // скругление (20, 0)
        Vertex{20.0, r},              // правая сторона
        Vertex{20.0, 20.0 - r, b},    // скругление (20, 20)
        Vertex{20.0 - r, 20.0},       // верх
        Vertex{r, 20.0, b},           // скругление (0, 20)
        Vertex{0.0, 20.0 - r},        // левая сторона
        Vertex{0.0, r, b},            // скругление (0, 0), замыкает в (r, 0)
    };
    rounded.closed = true;
    const Polyline straightened = decimated(rounded, tolerance);
    QVERIFY(!hasArcs(straightened));
    QVERIFY(near(straightened.area(), rounded.area(), tolerance * rounded.perimeter()));

    // Крупные дуги неприкосновенны: у стадиона сагитта полукругов -- целый
    // радиус, прореживанию там делать нечего.
    const Polyline stadium = obround(10.0, 4.0);
    const Polyline kept = decimated(stadium, tolerance);
    QCOMPARE(kept.size(), stadium.size());
    QVERIFY(near(kept.area(), stadium.area(), eps));

    // Полная окружность (две вершины) возвращается как есть.
    const Polyline full = circle(8.0);
    QCOMPARE(decimated(full, tolerance).size(), full.size());

    // Вырождающийся результат откатывается к исходному: треугольник мельче
    // допуска прореживание схлопнуло бы в отрезок.
    Polyline tiny{Vertex{0.0, 0.0}, Vertex{0.03, 0.0}, Vertex{0.0, 0.03}};
    tiny.closed = true;
    QCOMPARE(decimated(tiny, 1.0).size(), tiny.size());

    // Нулевой допуск -- прореживание выключено.
    QCOMPARE(decimated(noisy, 0.0).size(), noisy.size());
}

QTEST_APPLESS_MAIN(UtilTest)
#include "test_util.moc"
