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
    void boundingRectFollowsArcNotSupportingCircle();
    void invertNegatesMembership();
    void doubleInvertIsIdentity();
    void invertOfBodyWithHoleGivesBodyPerHole();
    void intersectionWithInvertedIsDifference();
    void unboundedRegionHasNoDrawableOuter();
    void polygonFlagReversesContoursAndMembership();
    void flaggedPolygonSubtractsWhenRegionIsBuilt();
    void polygonFlagSurvivesSerializationAndTransform();
    void regionWithIslandInHoleSurvivesSerialization();
    void denseRegionWithArcIslandsSurvivesSerialization();
    void smallHolesSurviveSerialization();
    void spanConstructorKeepsBodyNestedInsideHole();
    void translationPreservesAreaOfComplexArcBoundary();
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

// Габарит точного полигона считается по САМИМ дугам, а не по их опорным
// окружностям: CGAL-овский bbox() у всякой дуги растянут до края круга, и
// четверть дуги отдавала бы габарит целого круга.
void PolygonTest::boundingRectFollowsArcNotSupportingCircle() {
    // Сектор: два радиуса и четверть дуги радиуса 10 в первом квадранте.
    // Опорная окружность занимает [-10, 10] по обеим осям, сам сектор -- [0, 10].
    constexpr double r = 10.0;
    Polyline sector{
        Vertex{{0.0, 0.0}},
        Vertex{{r, 0.0}, std::tan((pi / 2.0) / 4.0)},
        Vertex{{0.0, r}},
    };
    sector.closed = true;

    const Polygon polygon{sector};
    QVERIFY(near(polygon.area(), pi * r * r / 4.0, 1e-6));

    const QRectF box = polygon.boundingRect();
    QVERIFY(near(box.left(), 0.0, 1e-6) && near(box.bottom(), r, 1e-6));
    QVERIFY(near(box.width(), r, 1e-6) && near(box.height(), r, 1e-6));

    // А у полного круга макушка и донышко в габарит попадают, как и должны.
    const QRectF disc = Polygon{circle(2.0 * r)}.boundingRect();
    QVERIFY(near(disc.width(), 2.0 * r, 1e-6) && near(disc.height(), 2.0 * r, 1e-6));
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
    const Polygons inverted = squareWithHole().complement();

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

void PolygonTest::polygonFlagReversesContoursAndMembership() {
    Polygon polygon{rectangle(10.0, 10.0)};
    QVERIFY(!polygon.isInverted());
    QVERIFY(polygon.outer().isPositive()); // тело обходится против часовой
    QVERIFY(polygon.contains({0.0, 0.0}));

    polygon.invert();

    QVERIFY(polygon.isInverted());
    QVERIFY(!polygon.outer().isPositive()); // пустота -- обход обратный
    QVERIFY(!polygon.contains({0.0, 0.0})); // точка тела больше не принадлежит
    QVERIFY(polygon.contains({100.0, 100.0}));

    // Граница та же самая, поэтому мерки не меняются.
    QVERIFY(near(polygon.area(), 100.0));
    QVERIFY(near(polygon.perimeter(), 40.0));
    QVERIFY(near(polygon.boundingRect().width(), 10.0));

    polygon.invert(); // обратимо без потерь
    QVERIFY(!polygon.isInverted());
    QVERIFY(polygon.outer().isPositive());
    QVERIFY(polygon.contains({0.0, 0.0}));
}

void PolygonTest::flaggedPolygonSubtractsWhenRegionIsBuilt() {
    const Polygon body{rectangle(20.0, 20.0)};
    Polygon hole{rectangle(10.0, 10.0)};
    hole.invert();

    // Пометка читается сборкой региона: тело объединяется, пустота вычитается.
    const Polygons region{body, hole};
    QVERIFY(near(region.area(), 400.0 - 100.0, 1e-6));
    QVERIFY(region.contains({9.0, 9.0}));
    QVERIFY(!region.contains({0.0, 0.0}));

    // Тот же ответ через плоский список контуров -- ради этого флаг и виден
    // в contours(): особого случая там не нужно вовсе.
    Polylines contours = body.contours();
    contours.append_range(hole.contours());
    QVERIFY(near(Polygons{contours}.area(), region.area(), 1e-6));

    // Регион из одной лишь пустоты пуст: вычитать её не из чего.
    QVERIFY(Polygons{hole}.empty());

    // А вот честное дополнение -- это другое: оно неограничено.
    QVERIFY(hole.complement().isUnbounded());
}

// Тело с дыркой, а в дырке -- отдельный островок. Ровно та расстановка, на
// которой плоский список контуров теряет островок вместе с дыркой.
void PolygonTest::regionWithIslandInHoleSurvivesSerialization() {
    Polygons source = Polygons{squareWithHole()} | square(4.0);
    QCOMPARE(source.all().size(), 2u);

    QByteArray buffer;
    {
        QDataStream out{&buffer, QIODevice::WriteOnly};
        out << source;
    }
    Polygons restored;
    {
        QDataStream in{&buffer, QIODevice::ReadOnly};
        in >> restored;
    }

    QCOMPARE(restored.all().size(), source.all().size());
    QVERIFY(near(restored.area(), source.area(), 1e-6));
}

// Плотный случай, ближе к реальной плате: решётка круглых дырок, в каждой --
// круглый островок. Границы дугами, вложенность на два уровня.
void PolygonTest::denseRegionWithArcIslandsSurvivesSerialization() {
    Polygons region = Polygons{Polylines{rectangle(100.0, 100.0)}};
    for(int x{}; x < 4; ++x)
        for(int y{}; y < 4; ++y) {
            const QPointF at{x * 22.0 - 33.0, y * 22.0 - 33.0};
            region -= Polygons{Polylines{circle(10.0, at)}}; // дырка
            region |= Polygons{Polylines{circle(4.0, at)}};  // островок в ней
        }

    // 1 тело + 16 островов.
    QCOMPARE(region.all().size(), 17u);

    QByteArray buffer;
    {
        QDataStream out{&buffer, QIODevice::WriteOnly};
        out << region;
    }
    Polygons restored;
    {
        QDataStream in{&buffer, QIODevice::ReadOnly};
        in >> restored;
    }

    QCOMPARE(restored.all().size(), region.all().size());
    QVERIFY(near(restored.area(), region.area(), 1e-6));
}

// Граница разрешения кругового обхода. Запись идёт через Dxf-вид (double +
// прогиб), чтение -- обратно в точный домен, и достаточно мелкая деталь по
// дороге сваривается в точку.
//
// Замерено: дырка диаметром 0.01 мм цела, 0.001 мм пропадает. Порог -- около
// микрона, то есть на порядки мельче всего, что бывает на плате: типовой зазор
// 0.1-0.2 мм, самая мелкая вскрышка -- десятки микрон. Тест держит именно
// плато-реалистичный диапазон; если порог уползёт вверх, он это заметит.
void PolygonTest::smallHolesSurviveSerialization() {
    for(double d: {1.0, 0.1, 0.05, 0.01}) {
        Polygons region = Polygons{Polylines{rectangle(20.0, 20.0)}};
        region -= Polygons{Polylines{circle(d)}};

        QByteArray buffer;
        {
            QDataStream out{&buffer, QIODevice::WriteOnly};
            out << region;
        }
        Polygons restored;
        {
            QDataStream in{&buffer, QIODevice::ReadOnly};
            in >> restored;
        }

        QVERIFY2(restored.all().size() == 1 && restored.all().front().holes().size() == 1,
            qPrintable(QStringLiteral("дырка d=%1 не пережила сериализацию").arg(d)));
        QVERIFY(near(restored.area(), region.area(), 1e-6));
    }
}

void PolygonTest::polygonFlagSurvivesSerializationAndTransform() {
    Polygon source = squareWithHole();
    source.invert();

    QByteArray buffer;
    {
        QDataStream out{&buffer, QIODevice::WriteOnly};
        out << source;
    }
    Polygon restored;
    {
        QDataStream in{&buffer, QIODevice::ReadOnly};
        in >> restored;
    }

    QVERIFY(restored.isInverted());
    QVERIFY(near(restored.area(), source.area(), 1e-6));
    QCOMPARE(restored.holes().size(), source.holes().size());
    // Контуры пишутся каноническими, поэтому обход после чтения не
    // разворачивается вторично.
    QCOMPARE(restored.outer().isPositive(), source.outer().isPositive());

    const Polygon moved = transformed(source, QTransform::fromTranslate(50.0, 0.0));
    QVERIFY(moved.isInverted());
    QVERIFY(near(moved.area(), source.area(), 1e-6));
    QVERIFY(near(moved.boundingRect().center().x(), 50.0, 1e-6));
}

// Регрессия: репер платы (кольцо, а в его дырке -- отдельная окружность)
// собирается двумя ГОТОВЫМИ ПОЛИГОНАМИ (Polygons(span<Polygon>)), а не
// плоским списком контуров. У плоского списка вложенность выражена одной
// ориентацией, и дырка кольца вычитала бы чужое тело (окружность) внутри
// себя -- обе фигуры пропадали разом. У готовых полигонов вложенность
// известна точно и вычитать нечего.
void PolygonTest::spanConstructorKeepsBodyNestedInsideHole() {
    // circle() принимает ДИАМЕТР: наружный диаметр 20 (радиус 10), дырка
    // диаметром 12 (радиус 6), точка внутри дырки диаметром 4 (радиус 2).
    Polyline hole = circle(12.0);
    hole.reverse(); // дырка -- навстречу телу
    const Polygon ring{circle(20.0), {hole}};
    const Polygon dot{circle(4.0)}; // лежит внутри дырки кольца

    const std::vector<Polygon> parts{ring, dot};
    const Polygons region{std::span<const Polygon>{parts}};

    QVERIFY(near(region.area(), ring.area() + dot.area(), 1e-6));
    QCOMPARE(region.size(), std::size_t(2));
    QVERIFY(region.contains({8.0, 0.0}));  // тело кольца (радиус 6..10)
    QVERIFY(!region.contains({5.0, 0.0})); // дырка кольца, вне точки (радиус 2..6)
    QVERIFY(region.contains({0.0, 0.0}));  // окружность внутри дырки (радиус 0..2)
}

// Регрессия: точный перенос сложного (составленного из нескольких дуг)
// контура через МАТЕРИАЛИЗАЦИЮ (bulge-вид в double) и обратную сборку
// (arcMidpoint + дуга по трём точкам) на редких контурах брал не ту ветвь
// окружности -- большую вместо малой, -- удваивая площадь почти вдвое.
// Geo::transformed() для одного лишь переноса срезает материализацию
// вовсе (Geo::translated, точный домен целиком), так что площадь обязана
// совпасть с точностью до бита, а не до знака после запятой.
void PolygonTest::translationPreservesAreaOfComplexArcBoundary() {
    // "Бублик с прорезями": кольцо (наружный минус внутренний круг) минус
    // крестообразная прорезь -- ровно тот класс контуров (несколько дуг
    // разного радиуса подряд), на котором ломалась сборка по трём точкам.
    Polygons thermal{Polylines{circle(0.2)}};
    thermal -= Polygons{Polylines{circle(0.1)}};
    thermal -= Polygons{Polylines{rectangle(0.02, 0.2), rectangle(0.2, 0.02)}};

    const double before = thermal.area();
    QVERIFY(before > 0.0);

    for(const QPointF offset: {QPointF{0.4, 1.4}, QPointF{-3.0, 7.5}, QPointF{1000.0, -1000.0}}) {
        const Polygons moved = transformed(thermal, QTransform::fromTranslate(offset.x(), offset.y()));
        QVERIFY(near(moved.area(), before, 1e-9));
        QVERIFY(near(moved.boundingRect().center().x() - thermal.boundingRect().center().x(), offset.x(), 1e-6));
    }
}

QTEST_APPLESS_MAIN(PolygonTest)
#include "test_polygon.moc"
