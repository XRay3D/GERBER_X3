// Точный домен: полигон с отверстиями, регион и булевы над ним. Отдельное
// внимание -- инверсии: она единственная из булевых уводит регион в
// неограниченный, и почти всё остальное ведёт себя на нём иначе.

#include "geo/polygon.h"
#include "geo/util.h"

#include <QPainterPath>
#include <QTest>

#include <cmath>
#include <numbers>

using namespace Geo;

namespace {

bool near(double a, double b, double tolerance = 1e-6) { return std::abs(a - b) <= tolerance; }

// Квадрат со стороной `side` с центром в начале координат.
Polygons square(double side) { return Polygons{Polylines{rectangle(side, side)}}; }

// Квадрат 20x20 с круглым отверстием диаметра 8 посередине.
Polygon squareWithHole() {
    Polyline hole = circle(8.0);
    hole.reverse(); // дырка -- навстречу телу
    return Polygon{rectangle(20.0, 20.0), {hole}};
}
} // namespace

class PolygonTest : public QObject {
    Q_OBJECT

private slots:
    void polygonWithHoleMeasuresBodyOnly();
    void invertNegatesMembership();
    void doubleInvertIsIdentity();
    void invertOfBodyWithHoleGivesBodyPerHole();
    void intersectionWithInvertedIsDifference();
    void unboundedRegionHasNoDrawableOuter();
};

void PolygonTest::polygonWithHoleMeasuresBodyOnly() {
    const Polygon polygon = squareWithHole();
    QVERIFY(!polygon.empty());
    QVERIFY(!polygon.isUnbounded());
    QVERIFY(near(polygon.area(), 400.0 - pi * 16.0, 1e-6));
    QVERIFY(near(polygon.perimeter(), 80.0 + 2.0 * pi * 4.0, 1e-6)); // внешняя плюс дырка
    QCOMPARE(polygon.holes().size(), std::size_t(1));
    QVERIFY(polygon.contains({9.0, 0.0}));  // в теле
    QVERIFY(!polygon.contains({0.0, 0.0})); // в дырке
    QVERIFY(!polygon.contains({50.0, 0.0}));
}

void PolygonTest::invertNegatesMembership() {
    Polygons region = square(10.0);
    QVERIFY(!region.isUnbounded());
    QVERIFY(region.contains({0.0, 0.0}));
    QVERIFY(!region.contains({100.0, 100.0}));

    region.invert();

    QVERIFY(region.isUnbounded());
    QVERIFY(!region.contains({0.0, 0.0}));
    QVERIFY(region.contains({100.0, 100.0}));

    // Свободный оператор -- та же операция, но копией.
    const Polygons byOperator = ~square(10.0);
    QVERIFY(byOperator.isUnbounded());
    QVERIFY(byOperator.contains({100.0, 100.0}));
}

void PolygonTest::doubleInvertIsIdentity() {
    // Инверсия точная, поэтому дважды применённая обязана дать РОВНО исходный
    // регион, а не близкий к нему.
    Polygons region = square(10.0);
    const double area = region.area();
    const QRectF box = region.boundingRect();

    region.invert().invert();

    QVERIFY(!region.isUnbounded());
    QVERIFY(near(region.area(), area));
    QVERIFY(near(region.boundingRect().width(), box.width()));
    QVERIFY(near(region.boundingRect().height(), box.height()));
    QVERIFY(region.contains({0.0, 0.0}));
    QVERIFY(!region.contains({100.0, 100.0}));

    // Пустой регион и вся плоскость -- взаимные дополнения.
    Polygons empty;
    QVERIFY(empty.empty());
    empty.invert();
    QVERIFY(!empty.empty());
    QVERIFY(empty.isUnbounded());
    QVERIFY(empty.contains({12345.0, -6789.0}));
}

void PolygonTest::invertOfBodyWithHoleGivesBodyPerHole() {
    // Дополнение тела с дыркой -- это неограниченный кусок ПЛЮС сама дырка,
    // ставшая отдельным телом. Ровно поэтому Polygon::inverted() отдаёт
    // регион, а не полигон.
    const Polygons inverted = squareWithHole().inverted();

    QVERIFY(inverted.isUnbounded());
    QCOMPARE(inverted.size(), std::size_t(2));
    QVERIFY(inverted.contains({0.0, 0.0}));   // бывшая дырка стала телом
    QVERIFY(!inverted.contains({9.0, 0.0}));  // бывшее тело стало пустотой
    QVERIFY(inverted.contains({50.0, 0.0})); // снаружи

    // Площадь считается по ограниченным кускам: неограниченный даёт ноль.
    QVERIFY(near(inverted.area(), pi * 16.0, 1e-6));
}

void PolygonTest::intersectionWithInvertedIsDifference() {
    const Polygons a = square(20.0);
    const Polygons b = square(10.0);

    const Polygons viaComplement = a & ~b;
    const Polygons viaDifference = a - b;

    QVERIFY(!viaComplement.isUnbounded()); // пересечение с конечным вернуло в конечное
    QVERIFY(near(viaComplement.area(), viaDifference.area(), 1e-9));
    QVERIFY(near(viaComplement.area(), 400.0 - 100.0, 1e-9));
    QVERIFY(viaComplement.contains({9.0, 9.0}));
    QVERIFY(!viaComplement.contains({0.0, 0.0}));
}

void PolygonTest::unboundedRegionHasNoDrawableOuter() {
    // У неограниченного куска внешней границы нет вовсе, поэтому ни габарита,
    // ни пути он не даёт -- об этом и предупреждает isUnbounded().
    const Polygons inverted = ~square(10.0);
    QCOMPARE(inverted.size(), std::size_t(1));
    QVERIFY(inverted.all().front().isUnbounded());
    QVERIFY(inverted.all().front().outer().empty());
    QVERIFY(inverted.boundingRect().isNull());
    QVERIFY(inverted.toPath().isEmpty());
    QVERIFY(near(inverted.area(), 0.0));

    // А вот дырки у него есть -- бывшее тело, и оно доступно контурами.
    QCOMPARE(inverted.all().front().holes().size(), std::size_t(1));
    QVERIFY(near(inverted.all().front().holes().front().area(), 100.0, 1e-6));
}

QTEST_APPLESS_MAIN(PolygonTest)
#include "test_polygon.moc"
