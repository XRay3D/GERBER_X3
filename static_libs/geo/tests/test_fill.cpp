// Змейка (Geo::zigzagFill): покрытие строками, дырки, поворот штриховки.
//
// Покрытие меряется суммой длин ГОРИЗОНТАЛЬНЫХ сегментов: строки идут через
// step, и их суммарная длина обязана сходиться к площади региона, делённой
// на шаг. Куски границы в эту сумму почти не попадают -- они вертикальные.

#include "geo/boolean.h"
#include "geo/fill.h"
#include "geo/util.h"

#include <QTest>

#include <cmath>

using namespace Geo;

namespace {

Polyline rect(double x0, double y0, double x1, double y1, bool ccw = true) {
    Polyline poly{Vertex(x0, y0), Vertex(x1, y0), Vertex(x1, y1), Vertex(x0, y1)};
    poly.closed = true;
    if(!ccw) poly.reverse();
    return poly;
}

// Длина сегментов, отклонённых от направления angle° не больше чем на волос.
double directedLength(const Polylines& paths, double angle) {
    const QPointF dir{std::cos(qDegreesToRadians(angle)), std::sin(qDegreesToRadians(angle))};
    double sum{};
    for(const Polyline& p: paths)
        for(size_t i{}; i + 1 < p.size(); ++i) {
            const QPointF d = p[i + 1] - p[i];
            const double len = std::hypot(d.x(), d.y());
            if(len > 0.0 && std::abs(d.x() * dir.x() + d.y() * dir.y()) / len > 1.0 - 1e-9)
                sum += len;
        }
    return sum;
}

} // namespace

class FillTest : public QObject {
    Q_OBJECT

private slots:
    void emptyInputs();
    void squareCoverage();
    void holeSplitsRows();
    void angleRotatesRows();
};

void FillTest::emptyInputs() {
    QVERIFY(zigzagFill(Polygons{}, 1.0).empty());
    const Polygons square{Polylines{rect(-9, -9, 9, 9)}};
    QVERIFY(zigzagFill(square, 0.0).empty());
    // Шаг много шире региона: одна строка посередине, а не крэш и не пустота.
    const Polylines one = zigzagFill(square, 1000.0);
    QCOMPARE(one.size(), 1u);
    QVERIFY(directedLength(one, 0.0) > 17.0);
}

void FillTest::squareCoverage() {
    const Polygons square{Polylines{rect(-9, -9, 9, 9)}}; // 18 x 18
    const Polylines fill = zigzagFill(square, 1.0);
    QVERIFY(!fill.empty());

    // Строк ~18-20 (центровка добавляет краевые), каждая длиной 18:
    // покрытие площадь/шаг = 324 с запасом на краевые строки.
    const double rows = directedLength(fill, 0.0);
    QVERIFY2(rows > 300.0 && rows < 380.0, qPrintable(QString::number(rows)));
}

void FillTest::holeSplitsRows() {
    const Polygons body{
        Polylines{rect(-10, -10, 10, 10), rect(-3, -3, 3, 3, false)}
    };
    const Polylines fill = zigzagFill(body, 1.0);
    QVERIFY(!fill.empty());

    // 400 - 36 = 364 площади: строки рвутся о дырку, но покрытие сходится.
    const double rows = directedLength(fill, 0.0);
    QVERIFY2(rows > 330.0 && rows < 440.0, qPrintable(QString::number(rows)));
}

void FillTest::angleRotatesRows() {
    const Polygons square{Polylines{rect(-9, -9, 9, 9)}};
    const Polylines fill = zigzagFill(square, 1.0, 90.0);
    QVERIFY(!fill.empty());

    // Строки встали вертикально; горизонтальных осталось всего ничего.
    QVERIFY(directedLength(fill, 90.0) > 300.0);
    QVERIFY(directedLength(fill, 0.0) < 50.0);
}

QTEST_APPLESS_MAIN(FillTest)
#include "test_fill.moc"
