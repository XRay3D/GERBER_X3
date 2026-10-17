// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
// Плоский bulge-вид: развороты, площади, габариты.

#include "geo/polyline.h"

#include <QTest>

#include <cmath>
#include <numbers>

using namespace Geo;

class PolylineTest : public QObject {
    Q_OBJECT

private slots:
    void reversePreservesShapeAndFlipsBulgeSign();
    void areaAndPerimeterMatchKnownCircle();
    void signedAreaTellsBodyFromHole();
    void boundingRectFollowsArcNotChord();
    void appendCarriesJunctionBulge();
};

void PolylineTest::reversePreservesShapeAndFlipsBulgeSign() {
    Polyline poly;
    poly.closed = false;
    poly = {Vertex(0.0, 0.0, 0.0), Vertex(10.0, 0.0, 1.0), Vertex(15.0, 5.0, 0.0)};

    const double perimeterBefore = poly.perimeter();
    poly.reverse();

    QVERIFY(std::abs(poly.perimeter() - perimeterBefore) < 1e-9); // разворот не меняет форму контура
    QCOMPARE(poly.size(), std::size_t(3));
    QVERIFY(std::abs(poly[0].x() - 15.0) < 1e-9);
    QVERIFY(std::abs(poly[0].y() - 5.0) < 1e-9);
    // Прогиб переехал на новую исходящую вершину (был на (10,0), теперь на
    // (15,5), которая после разворота идёт первой) и сменил знак.
    QVERIFY(std::abs(poly[0].bulge - (-1.0)) < 1e-9);
    QVERIFY(std::abs(poly[1].bulge) < 1e-9);
    QVERIFY(std::abs(poly[2].bulge) < 1e-9); // последняя вершина открытой полилинии -- без исходящего сегмента

    Polyline closedPoly;
    closedPoly.closed = true;
    closedPoly = {Vertex(0.0, 0.0, 0.3), Vertex(10.0, 0.0, -0.4), Vertex(5.0, 8.0, 0.6)};

    const double areaBefore = closedPoly.area();
    const double perimeterBeforeClosed = closedPoly.perimeter();
    closedPoly.reverse();

    QVERIFY(std::abs(closedPoly.area() - areaBefore) < 1e-9);
    QVERIFY(std::abs(closedPoly.perimeter() - perimeterBeforeClosed) < 1e-9);
}

void PolylineTest::areaAndPerimeterMatchKnownCircle() {
    constexpr double r = 5.0;
    Polyline circle;
    circle.closed = true;
    circle = {Vertex(r, 0.0, 1.0), Vertex(-r, 0.0, 1.0)};

    // Дуговые сегменты учитываются целиком: по одним вершинам формула
    // шнурков дала бы здесь ноль, а длина -- 2r вместо длины окружности.
    QVERIFY(std::abs(circle.area() - std::numbers::pi * r * r) < 1e-6);
    QVERIFY(std::abs(circle.perimeter() - 2.0 * std::numbers::pi * r) < 1e-6);
}

void PolylineTest::signedAreaTellsBodyFromHole() {
    // Знаком Polygons и отличает тело от пустоты внутри чьего-то тела,
    // поэтому знак важен сам по себе, отдельно от модуля.
    Polyline square;
    square.closed = true;
    square = {Vertex(0.0, 0.0), Vertex(4.0, 0.0), Vertex(4.0, 4.0), Vertex(0.0, 4.0)};

    QVERIFY(square.signedArea() > 0.0); // против часовой стрелки -- тело
    QVERIFY(std::abs(square.signedArea() - 16.0) < 1e-9);

    square.reverse();
    QVERIFY(square.signedArea() < 0.0);             // по часовой -- пустота
    QVERIFY(std::abs(square.area() - 16.0) < 1e-9); // модуль от разворота не зависит
}

void PolylineTest::boundingRectFollowsArcNotChord() {
    // Полуокружность радиуса 5 из (5,0) в (-5,0) через верх: по одним
    // вершинам габарит был бы нулевой высоты, а дуга выпирает на 5 вверх.
    Polyline arc;
    arc.closed = false;
    arc = {Vertex(5.0, 0.0, 1.0), Vertex(-5.0, 0.0, 0.0)};

    const QRectF box = arc.boundingRect();
    QVERIFY(std::abs(box.left() - (-5.0)) < 1e-9);
    QVERIFY(std::abs(box.right() - 5.0) < 1e-9);
    QVERIFY(std::abs(box.bottom() - 5.0) < 1e-9); // Y растёт вниз в QRectF, top/bottom здесь по значению
    QVERIFY(std::abs(box.top() - 0.0) < 1e-9);
}

void PolylineTest::appendCarriesJunctionBulge() {
    constexpr double pi = std::numbers::pi;
    const double b = std::tan(pi / 8.0); // четверть окружности R5: (10,0) -> (15,5)

    // Прогиб первой вершины куска описывает его первый сегмент -- после склейки
    // он обязан оказаться на стыковой вершине, а сама точка не дублируется.
    Polyline line{Vertex(0.0, 0.0), Vertex(10.0, 0.0)};
    const Polyline arc{Vertex(10.0, 0.0, b), Vertex(15.0, 5.0)};
    line.append(arc);
    QCOMPARE(line.size(), std::size_t(3));
    QVERIFY(std::abs(line[1].bulge - b) < 1e-12);
    QVERIFY(std::abs(line.back().bulge) < 1e-12);
    QVERIFY(std::abs(line.perimeter() - (10.0 + 2.5 * pi)) < 1e-9);

    // Кусок в форме гербера -- с дублем начала -- приклеенный перевёрнутым:
    // именно так терялись скругления углов платы.
    Polyline gerber{Vertex(10.0, 0.0), Vertex(10.0, 0.0, b), Vertex(15.0, 5.0)};
    gerber.reverse(); // [E(-b), S'(0), S(0)]
    Polyline other{Vertex(20.0, 5.0), Vertex(15.0, 5.0)};
    other.append(gerber);
    QCOMPARE(other.size(), std::size_t(4));
    QVERIFY(std::abs(other[1].bulge + b) < 1e-12);
    QVERIFY(std::abs(other.perimeter() - (5.0 + 2.5 * pi)) < 1e-9);

    // Пустые: к пустой -- копия целиком (с прогибом первой вершины), пустой
    // хвост ничего не меняет.
    Polyline empty;
    empty.append(arc);
    QCOMPARE(empty.size(), std::size_t(2));
    QVERIFY(std::abs(empty.front().bulge - b) < 1e-12);
    empty.append(Polyline{});
    QCOMPARE(empty.size(), std::size_t(2));
}

QTEST_APPLESS_MAIN(PolylineTest)
#include "test_polyline.moc"
