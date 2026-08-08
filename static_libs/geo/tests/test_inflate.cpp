// Офсет региона: наружу, внутрь и замыкание.
//
// Площадь меряется РАСТРОМ, а не разбором точных контуров: проверять надо
// именно то, что видно на экране, а растр ловит и перепутанную ориентацию
// дырки, и потерянный кусок региона -- то, чего сумма площадей контуров не
// заметила бы.

#include "geo/boolean.h"

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QTest>

#include <cmath>
#include <numbers>

using namespace Geo;

namespace {

constexpr int pixelsPerUnit = 10;
constexpr int rasterSize = 400; // окно [-20, 20] по обеим осям

// Допуск на растр: погрешность идёт по ПЕРИМЕТРУ (полпикселя на краю), и
// на фигурах этого теста укладывается в единицы квадратных единиц.
constexpr double areaTolerance = 0.5;

double rasterArea(const Polygons& region) {
    QImage image(rasterSize, rasterSize, QImage::Format_ARGB32);
    image.fill(Qt::white);
    {
        QPainter painter(&image);
        painter.translate(rasterSize / 2, rasterSize / 2);
        painter.scale(pixelsPerUnit, -pixelsPerUnit); // Y вверх, как на чертеже
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.drawPath(region.toPath());
    }
    long long filled = 0;
    for(int y = 0; y < rasterSize; ++y)
        for(int x = 0; x < rasterSize; ++x)
            if(qGray(image.pixel(x, y)) < 128) ++filled;
    return static_cast<double>(filled) / (pixelsPerUnit * pixelsPerUnit);
}

Polyline rectangle(double x0, double y0, double x1, double y1, bool counterClockwise) {
    Polyline poly{Vertex(x0, y0), Vertex(x1, y0), Vertex(x1, y1), Vertex(x0, y1)};
    poly.closed = true;
    if(!counterClockwise) poly.reverse();
    return poly;
}

// Площадь квадрата со стороной `side`, раздутого на `r`: сам квадрат, по
// прямоугольнику на каждую сторону и полный круг, собранный из четырёх
// четвертей в углах.
double grownSquareArea(double side, double r) {
    return side * side + 4.0 * side * r + std::numbers::pi * r * r;
}

} // namespace

class InflateTest : public QObject {
    Q_OBJECT

private slots:
    void outwardGrowsByMinkowskiDisc();
    void inwardShrinksExactly();
    void holeGrowsInwardAndShrinksOutward();
    void closingRestoresOriginalArea();
    void zeroDeltaIsIdentity();
    void erosionThickerThanBodyEmptiesIt();
};

void InflateTest::outwardGrowsByMinkowskiDisc() {
    constexpr double side = 20.0, delta = 4.0, r = delta / 2;
    const Polygons base{Polylines{rectangle(-10, -10, 10, 10, true)}};

    QVERIFY(std::abs(rasterArea(base) - side * side) < areaTolerance);
    QVERIFY(std::abs(rasterArea(Inflate(base, +delta)) - grownSquareArea(side, r)) < areaTolerance);
}

void InflateTest::inwardShrinksExactly() {
    constexpr double side = 20.0, delta = 4.0, r = delta / 2;
    const Polygons base{Polylines{rectangle(-10, -10, 10, 10, true)}};

    // Эрозия квадрата диском -- снова квадрат, с ОСТРЫМИ углами: скругление
    // при сжатии уходит внутрь и срезает не углы, а ничего.
    const double expected = (side - 2 * r) * (side - 2 * r);
    QVERIFY(std::abs(rasterArea(Inflate(base, -delta)) - expected) < areaTolerance);
}

void InflateTest::holeGrowsInwardAndShrinksOutward() {
    constexpr double side = 20.0, hole = 6.0, delta = 4.0, r = delta / 2;
    // Дырка -- контур обратной ориентации: в плоском списке вложенность
    // выражается только ею.
    const Polygons base{
        Polylines{
                  rectangle(-10, -10, 10, 10, true),
                  rectangle(-3, -3, 3, 3, false)}
    };
    QVERIFY(std::abs(rasterArea(base) - (side * side - hole * hole)) < areaTolerance);

    // Сжатие тела растит дырку -- по тем же правилам, что тело сжимается.
    const double shrunk = (side - 2 * r) * (side - 2 * r) - grownSquareArea(hole, r);
    QVERIFY(std::abs(rasterArea(Inflate(base, -delta)) - shrunk) < areaTolerance);

    // Раздувание, наоборот, дырку ужимает: 6x6 минус по r с каждой стороны.
    const double grown = grownSquareArea(side, r) - (hole - 2 * r) * (hole - 2 * r);
    QVERIFY(std::abs(rasterArea(Inflate(base, +delta)) - grown) < areaTolerance);
}

void InflateTest::closingRestoresOriginalArea() {
    // Замыкание выпуклой фигуры -- тождество: скруглять нечего, затягивать
    // тоже. Заодно проверяется, что регион переживает цепочку операций, не
    // выходя из точного домена.
    constexpr double side = 20.0, delta = 4.0;
    const Polygons base{Polylines{rectangle(-10, -10, 10, 10, true)}};

    const Polygons closed = Inflate(Inflate(base, +delta), -delta);
    QVERIFY(std::abs(rasterArea(closed) - side * side) < areaTolerance);
}

void InflateTest::zeroDeltaIsIdentity() {
    const Polygons base{Polylines{rectangle(-10, -10, 10, 10, true)}};
    const double area = rasterArea(base);

    QVERIFY(std::abs(rasterArea(Inflate(base, 0.0)) - area) < areaTolerance);
}

void InflateTest::erosionThickerThanBodyEmptiesIt() {
    // Полоса шириной 4 при сжатии на 6 (по 3 с каждой стороны) исчезает
    // целиком -- пустой регион здесь законный ответ, а не сбой.
    const Polygons strip{Polylines{rectangle(-10, -2, 10, 2, true)}};
    QVERIFY(rasterArea(strip) > 0.0);

    const Polygons eroded = Inflate(strip, -6.0);
    QCOMPARE(rasterArea(eroded), 0.0);
    QCOMPARE(eroded.all().size(), std::size_t(0));
}

QTEST_APPLESS_MAIN(InflateTest)
#include "test_inflate.moc"
