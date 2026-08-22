// Пофигурные витки офсета (Geo::InflatePasses): совпадение с монолитным
// Inflate, контракт чернового допуска, сдвиг расписания bias и отмена.
//
// Совпадение проверяется ТОЧНЫМИ разностями регионов, а не растром: у
// точного витка (coarse == 0) тождество эрозии обязано давать тот же
// регион с точностью до сварочного допуска материализации баз.

#include "geo/boolean.h"
#include "geo/cancel.h"
#include "geo/inflatepasses.h"

#include <QTest>

#include <cmath>
#include <numbers>
#include <stop_token>

using namespace Geo;

namespace {

Polyline rectangle(double x0, double y0, double x1, double y1, bool counterClockwise = true) {
    Polyline poly{Vertex(x0, y0), Vertex(x1, y0), Vertex(x1, y1), Vertex(x0, y1)};
    poly.closed = true;
    if(!counterClockwise) poly.reverse();
    return poly;
}

// Зубчатое кольцо -- то же, что в test_inflate: мелочь границы, на которой
// прореживание и принятие базы отрабатывают в полную силу.
Polyline toothedRing(double radius, double tooth, int teeth = 90) {
    Polyline ring;
    ring.closed = true;
    for(int k = 0; k < teeth; ++k) {
        const double a = 2.0 * std::numbers::pi * k / teeth;
        const double r = radius + (k % 2 ? tooth : -tooth);
        ring.emplace_back(QPointF{r * std::cos(a), r * std::sin(a)}, 0.0);
    }
    return ring;
}

// Регионы совпадают: обе разности пусты (допуск -- сварочный шум округления
// вершин при пересборке баз из bulge-вида).
bool sameRegion(const Polygons& a, const Polygons& b) {
    constexpr double noise = 1e-6;
    return (a - b).area() < noise && (b - a).area() < noise;
}

} // namespace

class InflatePassesTest : public QObject {
    Q_OBJECT

private slots:
    void solidsAloneMatchMonolithicInflate();
    void ambientMinusSolidsMatchesFieldErosion();
    void coarsePassStaysWithinContract();
    void adoptionKeepsPassesWithinContract();
    void nestedFieldBodiesStayIndependent();
    void biasShiftsSolidSchedule();
    void ambientEatenGivesEmptyPass();
    void cancelAbortsPass();
};

// Без объемлющих виток -- объединение раздутых тел: ровно Inflate региона
// наружу. Тела разнесены и слиты в один регион заранее, так что тождество
// точное.
void InflatePassesTest::solidsAloneMatchMonolithicInflate() {
    // Кольцо у нуля цепляет оба квадрата -- и пусть: регион уже объединён,
    // а его тела заново разбирает сам конструктор.
    const Polygons region{Polylines{rectangle(-8, -8, -2, -2), rectangle(2, 2, 8, 8),
        toothedRing(3, 0.15)}};
    InflatePasses passes;
    passes.addSolids(region);
    for(const double d: {0.5, 1.7, 3.0})
        QVERIFY(sameRegion(passes.pass(d), Inflate(region, 2.0 * d)));
}

// Поле «рамка минус тела»: пофигурный виток обязан совпасть с эрозией поля
// целиком -- морфологическое тождество erosion(A\B) = erosion(A) \ dilation(B).
void InflatePassesTest::ambientMinusSolidsMatchesFieldErosion() {
    const Polyline frame = rectangle(-15, -15, 15, 15);
    const Polygons copper{Polylines{rectangle(-10, -10, -4, -4), rectangle(4, -10, 10, -4),
        rectangle(-10, 4, 10, 10), toothedRing(2, 0.15)}};
    const Polygons field = Polygons{Polylines{frame}} - copper;

    InflatePasses passes;
    passes.addAmbient(frame);
    passes.addSolids(copper);
    for(const double d: {0.4, 1.1, 2.3})
        QVERIFY(sameRegion(passes.pass(d), Inflate(field, -2.0 * d)));
}

// Черновой виток НА СВЕЖЕЙ БАЗЕ: регион лежит МЕЖДУ честным и честным,
// раздутым на 2*tol, -- контур уезжает только к границе поля и не дальше
// tol (контракт Inflate, унаследованный пофигурной сборкой). Движок на
// каждый d свой: история принятий здесь не участвует.
void InflatePassesTest::coarsePassStaysWithinContract() {
    const Polyline frame = rectangle(-15, -15, 15, 15);
    const Polygons copper{Polylines{toothedRing(6, 0.15), rectangle(9, 9, 13, 13)}};
    const Polygons field = Polygons{Polylines{frame}} - copper;

    constexpr double tol = 0.2;
    for(const double d: {1.0, 2.0, 3.5}) {
        InflatePasses passes;
        passes.addAmbient(frame);
        passes.addSolids(copper);
        const Polygons coarse = passes.pass(d, tol);
        const Polygons exact = Inflate(field, -2.0 * d);
        QVERIFY((exact - coarse).area() < 1e-6);                     // только к границе...
        QVERIFY((coarse - Inflate(exact, 2.0 * tol)).area() < 1e-6); // ...и не дальше tol
    }
}

// Много витков подряд: базы принимаются и переиспользуются, недобор черновых
// баз переходит в следующие витки. Проверяются обе стороны контракта из
// заголовка: дрейф от честного офсета исходной области -- к границе и не
// больше tol на виток накопительно; расстояние между СОСЕДНИМИ витками --
// шаг ± tol, что бы ни принималось между ними.
void InflatePassesTest::adoptionKeepsPassesWithinContract() {
    const Polyline frame = rectangle(-16, -16, 16, 16);
    const Polygons copper{Polylines{toothedRing(5, 0.15), toothedRing(1.2, 0.1)}};
    const Polygons field = Polygons{Polylines{frame}} - copper;

    constexpr double tol = 0.15;
    constexpr double step = 0.35;
    InflatePasses passes;
    passes.addAmbient(frame);
    passes.addSolids(copper);
    Polygons previous;
    for(int i = 1; i <= 10; ++i) {
        const double d = step * i;
        const Polygons coarse = passes.pass(d, tol);
        const Polygons exact = Inflate(field, -2.0 * d);
        if(exact.empty()) break;
        QVERIFY((exact - coarse).area() < 1e-6);
        QVERIFY((coarse - Inflate(exact, 2.0 * tol * i)).area() < 1e-6);
        if(i > 1) {
            QVERIFY((coarse - Inflate(previous, -2.0 * (step - tol))).area() < 1e-6);
            QVERIFY((Inflate(previous, -2.0 * (step + tol)) - coarse).area() < 1e-6);
        }
        previous = coarse;
    }
}

// Вложенность области: поле внутри медного кольца -- отдельное тело поля,
// лежащее в ДЫРКЕ тела «рамка минус кольцо». Дырка внешнего тела -- это
// оболочка внутреннего, и addField держит их порознь: раздутая оболочка
// вычитается только из своей группы, внутреннее поле остаётся целым.
void InflatePassesTest::nestedFieldBodiesStayIndependent() {
    const Polyline frame = rectangle(-15, -15, 15, 15);
    Polyline ringHole = rectangle(-6, -6, 6, 6, false); // дырка кольца: по часовой
    const Polygons copper{Polylines{rectangle(-10, -10, 10, 10), ringHole,
        toothedRing(1.5, 0.1)}}; // кольцо с пятачком внутри -- три тела поля, два вложены
    const Polygons field = Polygons{Polylines{frame}} - copper;
    QCOMPARE(field.all().size(), std::size_t(2)); // поле снаружи кольца и поле внутри него

    InflatePasses passes;
    for(const Polygon& body: field.all()) passes.addField(body);
    for(const double d: {0.3, 0.9, 1.6})
        QVERIFY(sameRegion(passes.pass(d), Inflate(field, -2.0 * d)));
}

// Тело с bias < 0 входит в расписание позже: на витке d == -bias вычитается
// как есть, дальше растёт на d + bias -- ровно семантика запретной полосы
// кармана (уже выбранного контурным проходом).
void InflatePassesTest::biasShiftsSolidSchedule() {
    const Polyline frame = rectangle(-15, -15, 15, 15);
    const Polygons copper{Polylines{rectangle(-6, -6, 6, 6)}};
    const Polygons forbidden{Polylines{rectangle(-9, -9, 9, -7)}};
    constexpr double b = 0.5;

    InflatePasses passes;
    passes.addAmbient(frame);
    passes.addSolids(copper);
    passes.addSolids(forbidden, InflatePasses::global, -b);

    const Polygons frameRegion{Polylines{frame}};
    for(const double s: {0.0, 0.7, 1.4}) {
        const double d = b + s;
        const Polygons expected = Inflate(frameRegion - copper, -2.0 * d)
            - (s > 0.0 ? Inflate(forbidden, 2.0 * s) : forbidden);
        QVERIFY(sameRegion(passes.pass(d), expected));
    }
}

// Усадка съедает объемлющее целиком -- виток пуст, и дальше тоже: источник
// помечен мёртвым, а не пересчитывается заново.
void InflatePassesTest::ambientEatenGivesEmptyPass() {
    InflatePasses passes;
    passes.addAmbient(rectangle(-2, -2, 2, 2));
    QVERIFY(!passes.pass(1.0).empty());
    QVERIFY(passes.pass(2.5).empty());
    QVERIFY(passes.pass(3.0).empty());
}

// Отмена областная (CancelScope): уже взведённый токен рвёт pass() сразу.
void InflatePassesTest::cancelAbortsPass() {
    InflatePasses passes;
    passes.addSolids(Polygons{Polylines{toothedRing(5, 0.15)}});

    std::stop_source source;
    source.request_stop();
    CancelScope scope{source.get_token()};
    QVERIFY_THROWS_EXCEPTION(Cancelled, passes.pass(1.0));
}

QTEST_APPLESS_MAIN(InflatePassesTest)

#include "test_inflatepasses.moc"
