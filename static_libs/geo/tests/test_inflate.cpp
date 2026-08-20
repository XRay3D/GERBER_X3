// Офсет региона: наружу, внутрь и замыкание.
//
// Площадь меряется РАСТРОМ, а не разбором точных контуров: проверять надо
// именно то, что видно на экране, а растр ловит и перепутанную ориентацию
// дырки, и потерянный кусок региона -- то, чего сумма площадей контуров не
// заметила бы.

#include "geo/boolean.h"
#include "geo/util.h"

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QTest>

#include <algorithm>
#include <cmath>
#include <numbers>

using namespace Qt::StringLiterals;

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
    void arcSmallerThanOffsetStaysOneArc();
    void shrunkDiscKeepsTwoVertices();
    void coarseOffsetUndershootsWithinTolerance();
};

// Черновой офсет (coarse > 0): контур результата лежит НЕ ДАЛЬШЕ от границы,
// чем честный, и не ближе к ней, чем честный минус coarse. Проверяются оба
// края контракта разностями регионов -- они точные, растр здесь не нужен:
// перебор ловится пустотой (coarse - exact), недобор -- накрытием честного
// черновым, раздутым на 2*coarse.
void InflateTest::coarseOffsetUndershootsWithinTolerance() {
    // Кольцо с мелкой зубчаткой: сегменты по ~0.7 -- на большом офсете
    // прореживание выбрасывает их тела и почти все диски.
    Polyline ring;
    ring.closed = true;
    constexpr int teeth = 90;
    for(int k = 0; k < teeth; ++k) {
        const double a = 2.0 * std::numbers::pi * k / teeth;
        const double r = 10.0 + (k % 2 ? 0.15 : -0.15);
        ring.emplace_back(QPointF{r * std::cos(a), r * std::sin(a)}, 0.0);
    }
    const Polygons region{Polylines{ring}};

    constexpr double delta = 8.0; // d = 4: шаг дисков sqrt(4*4*0.2) = 1.8 > зуба
    constexpr double tol = 0.2;
    for(const double sign: {+1.0, -1.0}) {
        const Polygons exact = Inflate(region, sign * delta);
        const Polygons coarse = Inflate(region, sign * delta, tol);
        // Недобор полосы двигает контур к границе: наружу черновой регион
        // лежит ВНУТРИ честного, внутрь -- наоборот, честный внутри чернового.
        const Polygons& inner = sign > 0 ? coarse : exact;
        const Polygons& outer = sign > 0 ? exact : coarse;
        QVERIFY((inner - outer).area() < 1e-9);
        // И не дальше tol: внешний накрывается внутренним, раздутым на 2*tol.
        QVERIFY((outer - Inflate(inner, 2.0 * tol)).area() < 1e-9);
    }
}

// Дуга РАДИУСОМ МЕНЬШЕ офсета -- случай, на котором внутренняя окружность
// вырождается. Прежде её резали на хорды и раздували каждую отдельно: вместо
// одной дуги на выходе была цепочка колпачков, разделённых прямыми. Проверяем
// обе стороны дела -- и форму (площадь полукруга считается точно), и то, что
// дуга осталась ОДНОЙ.
void InflateTest::arcSmallerThanOffsetStaysOneArc() {
    constexpr double R = 1.0, delta = 6.0, r = delta / 2; // r = 3 > R
    // Полукруг: две вершины, дуга сверху (прогиб +1 -- полуокружность) и
    // прямая обратно.
    Polyline half{Vertex(-R, 0.0, 1.0), Vertex(R, 0.0)};
    half.closed = true;
    const Polygons base{Polylines{half}};
    QVERIFY(std::abs(rasterArea(base) - std::numbers::pi * R * R / 2) < areaTolerance);

    const Polygons grown = Inflate(base, +delta);

    // Раздутый полукруг: круг радиуса R + r сверху, снизу -- прямоугольник
    // 2R x r и два четвертькруга по концам диаметра.
    const double expected = std::numbers::pi * (R + r) * (R + r) / 2 // верх
        + 2 * R * r                                                  // полоса под диаметром
        + std::numbers::pi * r * r / 2;                              // два конца
    QVERIFY(std::abs(rasterArea(grown) - expected) < areaTolerance);

    // Дуг в контуре ровно столько, сколько их в фигуре: одна снаружи и по
    // одной на конец диаметра. Цепочка хорд дала бы их десятки.
    const Polylines contours = grown.contours(); // отдаётся ЗНАЧЕНИЕМ
    QCOMPARE(contours.size(), 1u);
    const auto arcs = std::ranges::count_if(contours.front(), &Vertex::isArc);
    QVERIFY2(arcs <= 3, qPrintable(QString::number(arcs)));
}

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

// Круглая дырка, которую заливает pocket концентрическими витками: каждый
// виток -- Inflate ИСХОДНОГО диска внутрь на свою глубину. Свип режет
// окружность на несколько x-монотонных кусков, и при материализации они
// сшиваются обратно в одну дугу; размах кусков считается по округлённым
// концам и в сумме недобирает до 2*pi на ~1e-8 -- порог полного оборота это
// пропускало, и окружность схлопывалась в ОДНУ вершину с прогибом ~1e8, а
// contours() её потом браковал как вырожденную. В pocket так пропадали все
// витки заливки, кроме первого. Окружность обязана оставаться двумя вершинами.
void InflateTest::shrunkDiscKeepsTwoVertices() {
    // Диск с дыркой -- как приходит из чертежа; дырка -- как её отдаёт
    // разность «граница минус медь» (Polygon без дырок).
    const Polygons disc{Polylines{circle(6.0)}};
    const Polygons ring = Polygons{Polylines{circle(30.0)}} - disc;
    const Polygons hole = Polygons{Polylines{circle(30.0)}} - ring;
    QCOMPARE(hole.all().size(), 1u);

    const Polygons region = Inflate(hole, -0.5); // r = 2.75
    for(int i = 1; i <= 10; ++i) {
        const Polygons loop = Inflate(region, -0.5 * i);
        const double r = 2.75 - 0.25 * i;
        QVERIFY2(!loop.empty(), qPrintable(u"виток %1 пуст"_s.arg(i)));
        QVERIFY(std::abs(loop.area() - std::numbers::pi * r * r) < 1e-6);
        const Polylines contours = loop.contours();
        QCOMPARE(contours.size(), 1u);
        // Канон окружности -- две половины (прогиб +-1): такой её пишет
        // одной командой вывод УП, и такой её пересобирает поворот начала.
        const Polyline& loop0 = contours.front();
        QVERIFY2(loop0.size() == 2, qPrintable(u"виток %1: вершин %2"_s.arg(i).arg(loop0.size())));
        QVERIFY(std::abs(std::abs(loop0.front().bulge) - 1.0) < 1e-9);
        QVERIFY(std::abs(std::abs(loop0.back().bulge) - 1.0) < 1e-9);
        QVERIFY(std::abs(loop0.perimeter() - 2.0 * std::numbers::pi * r) < 1e-6);
    }
}

QTEST_APPLESS_MAIN(InflateTest)
#include "test_inflate.moc"
