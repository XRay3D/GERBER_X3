// Вспомогательная обвязка: примитивы, математика дуг, трансформации, разбор
// пути Qt. Проверяется тем, что известно аналитически, -- площадью, периметром
// и габаритом, а не покоординатным сличением.

#include "geo/polygon.h"
#include "geo/util.h"

#include <QPainterPath>
#include <QTest>

#include <cmath>
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
}

void UtilTest::bulgeMathRoundTrips() {
    const QPointF p1{0.0, 0.0}, p2{10.0, 0.0};
    constexpr double bulge = 0.4;

    const auto a = arcOf(p1, p2, bulge);
    QVERIFY(a.has_value());
    QVERIFY(near(a->theta, 4.0 * std::atan(bulge)));
    QVERIFY(near(a->radius * std::abs(a->theta), Polyline{Vertex{p1, bulge}, Vertex{p2}}.perimeter(), 1e-9));
    // Концы дуги лежат на ней самой.
    QVERIFY(near(a->pointAt(0.0).x(), p1.x(), 1e-9) && near(a->pointAt(0.0).y(), p1.y(), 1e-9));
    QVERIFY(near(a->pointAt(1.0).x(), p2.x(), 1e-9) && near(a->pointAt(1.0).y(), p2.y(), 1e-9));

    // Обратная задача: по центру и направлению получается тот же прогиб.
    QVERIFY(near(bulgeOf(p1, p2, a->center, a->dir()), bulge, 1e-9));

    QVERIFY(!arcOf(p1, p2, 0.0).has_value());  // прямая
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
    QVERIFY(isSimilarity(QTransform::fromScale(-1.0, 1.0)));   // зеркало -- тоже подобие
    QVERIFY(isSimilarity(QTransform{}.rotate(30.0)));
    QVERIFY(isSimilarity(QTransform::fromTranslate(5.0, 5.0)));
    QVERIFY(!isSimilarity(QTransform::fromScale(2.0, 3.0)));   // дуга стала бы эллипсом
    QVERIFY(!isSimilarity(QTransform{}.shear(0.5, 0.0)));

    QVERIFY(hasArcs(circle(4.0)));
    QVERIFY(!hasArcs(rectangle(4.0, 4.0)));
}

void UtilTest::closeAndReversedKeepArcs() {
    // Контур с повторённой замыкающей точкой -- ровно то, что приносят парсеры.
    Polyline poly{Vertex{0.0, 0.0}, Vertex{10.0, 0.0}, Vertex{10.0, 10.0, 1.0}, Vertex{0.0, 0.0}};
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

QTEST_APPLESS_MAIN(UtilTest)
#include "test_util.moc"
