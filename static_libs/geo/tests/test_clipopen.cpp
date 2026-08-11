// Клиппинг путей регионом (Geo::clipOpen): стороны разреза, сохранение
// длины, дуги и замкнутый вход.
//
// Проверяются оба лица примитива: Difference и Intersection дополняют друг
// друга до исходного пути (суммарная длина сохраняется), а куски дуг
// остаются на своей опорной окружности -- резка не имеет права стягивать
// дугу в хорду.

#include "geo/boolean.h"
#include "geo/util.h"

#include <QTest>

#include <cmath>

using namespace Geo;

namespace {

Polyline rect(double x0, double y0, double x1, double y1) {
    Polyline poly{Vertex(x0, y0), Vertex(x1, y0), Vertex(x1, y1), Vertex(x0, y1)};
    poly.closed = true;
    return poly;
}

Polygons rectRegion(double x0, double y0, double x1, double y1) {
    return Polygons{Polylines{rect(x0, y0, x1, y1)}};
}

Polyline openSeg(QPointF a, QPointF b) {
    return Polyline{Vertex{a}, Vertex{b}};
}

double totalLength(const Polylines& polylines) {
    double sum{};
    for(const Polyline& p: polylines) sum += p.perimeter();
    return sum;
}

constexpr double lengthTolerance = 1e-9;

// Тот же ли это путь -- вершина в вершину, с прогибами и замкнутостью.
// Своё сравнение, потому что у Vertex операторного равенства нет.
bool samePath(const Polylines& actual, const Polylines& expected) {
    if(actual.size() != expected.size()) return false;
    for(std::size_t i{}; i < actual.size(); ++i) {
        const Polyline& a = actual[i];
        const Polyline& b = expected[i];
        if(a.size() != b.size() || a.closed != b.closed) return false;
        for(std::size_t j{}; j < a.size(); ++j)
            if(distance(a[j], b[j]) > lengthTolerance
                || std::abs(a[j].bulge - b[j].bulge) > lengthTolerance) return false;
    }
    return true;
}

} // namespace

class ClipOpenTest : public QObject {
    Q_OBJECT

private slots:
    void segmentThroughSquare();
    void segmentFullyInsideOrOutside();
    void emptyRegion();
    void endpointOnBoundary();
    void segmentAlongEdge();
    void tangentLineToDisc();
    void arcCrossingSquare();
    void closedRingCutByStrip();
    void closedRingUntouched();
    void closedRingInsideRegion();
};

void ClipOpenTest::segmentThroughSquare() {
    const Polygons square = rectRegion(-10, -10, 10, 10);
    const Polylines path{openSeg({-20, 0}, {20, 0})};

    const Polylines outside = clipOpen(BooleanOp_::Difference, path, square);
    const Polylines inside = clipOpen(BooleanOp_::Intersection, path, square);

    QCOMPARE(outside.size(), 2u);
    QCOMPARE(inside.size(), 1u);
    QVERIFY(std::abs(totalLength(outside) - 20.0) < lengthTolerance);
    QVERIFY(std::abs(totalLength(inside) - 20.0) < lengthTolerance);
    // Difference и Intersection дополняют друг друга до исходного пути.
    QVERIFY(std::abs(totalLength(outside) + totalLength(inside) - 40.0) < lengthTolerance);
}

void ClipOpenTest::segmentFullyInsideOrOutside() {
    const Polygons square = rectRegion(-10, -10, 10, 10);

    const Polylines in{openSeg({-5, 0}, {5, 0})};
    QVERIFY(clipOpen(BooleanOp_::Difference, in, square).empty());
    QVERIFY(samePath(clipOpen(BooleanOp_::Intersection, in, square), in));

    const Polylines out{openSeg({-20, 15}, {20, 15})};
    QVERIFY(samePath(clipOpen(BooleanOp_::Difference, out, square), out));
    QVERIFY(clipOpen(BooleanOp_::Intersection, out, square).empty());
}

void ClipOpenTest::emptyRegion() {
    const Polylines path{openSeg({-1, -1}, {1, 1})};
    QVERIFY(samePath(clipOpen(BooleanOp_::Difference, path, Polygons{}), path));
    QVERIFY(clipOpen(BooleanOp_::Intersection, path, Polygons{}).empty());
}

void ClipOpenTest::endpointOnBoundary() {
    // Конец пути лежит ТОЧНО на границе: щепки у конца сливаются, путь
    // остаётся одним куском по свою сторону.
    const Polygons square = rectRegion(-10, -10, 10, 10);
    const Polylines path{openSeg({-20, 0}, {-10, 0})};

    const Polylines outside = clipOpen(BooleanOp_::Difference, path, square);
    QCOMPARE(outside.size(), 1u);
    QVERIFY(std::abs(totalLength(outside) - 10.0) < lengthTolerance);
    QVERIFY(clipOpen(BooleanOp_::Intersection, path, square).empty());
}

void ClipOpenTest::segmentAlongEdge() {
    // Путь ВДОЛЬ ребра региона: настоящих пересечений нет, середина лежит на
    // границе, а contains строг -- граница считается снаружи. Difference
    // такой путь оставляет, Intersection выбрасывает.
    const Polygons square = rectRegion(-10, -10, 10, 10);
    const Polylines path{openSeg({-5, 10}, {5, 10})};

    QVERIFY(samePath(clipOpen(BooleanOp_::Difference, path, square), path));
    QVERIFY(clipOpen(BooleanOp_::Intersection, path, square).empty());
}

void ClipOpenTest::tangentLineToDisc() {
    // Касательная к кругу: точка касания может дать разрез, но оба куска по
    // одну сторону -- и Difference обязан вернуть путь КАК ЕСТЬ, без лишней
    // вершины в точке касания.
    const Polygons disc{Polylines{circle(10.0)}}; // радиус 5
    const Polylines path{openSeg({-10, 5}, {10, 5})};

    QVERIFY(samePath(clipOpen(BooleanOp_::Difference, path, disc), path));
    QVERIFY(clipOpen(BooleanOp_::Intersection, path, disc).empty());
}

void ClipOpenTest::arcCrossingSquare() {
    // Полуокружность радиуса 10 через верх (0, 10), квадрат режет её на
    // подходе к макушке. Куски обязаны остаться на опорной окружности --
    // и вершинами, и прогибами.
    Polyline half{Vertex(10, 0, 1.0), Vertex(-10, 0)}; // ccw через (0, 10)
    const Polylines path{half};
    const Polygons square = rectRegion(-2, 8, 2, 12);

    const Polylines outside = clipOpen(BooleanOp_::Difference, path, square);
    const Polylines inside = clipOpen(BooleanOp_::Intersection, path, square);

    QCOMPARE(outside.size(), 2u);
    QCOMPARE(inside.size(), 1u);
    QVERIFY(std::abs(totalLength(outside) + totalLength(inside) - pi * 10.0) < 1e-6);

    auto onCircle = [](const Polylines& pieces) {
        for(const Polyline& piece: pieces) {
            for(const Vertex& v: piece)
                if(std::abs(std::hypot(v.x(), v.y()) - 10.0) > 1e-9) return false;
            for(std::size_t i{}; i + 1 < piece.size(); ++i) {
                if(!piece[i].isArc()) return false; // хорда вместо дуги -- потеря
                const auto arc = arcOf(piece[i], piece[i + 1], piece[i].bulge);
                if(!arc) return false;
                if(std::hypot(arc->center.x(), arc->center.y()) > 1e-9) return false;
                if(std::abs(arc->radius - 10.0) > 1e-9) return false;
            }
        }
        return true;
    };
    QVERIFY(onCircle(outside));
    QVERIFY(onCircle(inside));
}

void ClipOpenTest::closedRingCutByStrip() {
    // Кольцо, разрезанное вертикальной полосой: по куску слева и справа,
    // ОТКРЫТЫХ. Шов кольца лежит внутри одного из кусков -- если бы склейка
    // через стартовую вершину не работала, кусков было бы три.
    const Polylines ring{circle(20.0)}; // радиус 10
    const Polygons strip = rectRegion(-2, -20, 2, 20);

    const Polylines outside = clipOpen(BooleanOp_::Difference, ring, strip);
    const Polylines inside = clipOpen(BooleanOp_::Intersection, ring, strip);

    QCOMPARE(outside.size(), 2u);
    QCOMPARE(inside.size(), 2u);
    for(const Polyline& piece: outside) QVERIFY(!piece.closed);
    for(const Polyline& piece: inside) QVERIFY(!piece.closed);
    QVERIFY(std::abs(totalLength(outside) + totalLength(inside) - 2.0 * pi * 10.0) < 1e-6);
}

void ClipOpenTest::closedRingUntouched() {
    const Polylines ring{circle(20.0)};
    const Polygons far = rectRegion(30, 30, 40, 40);

    const Polylines outside = clipOpen(BooleanOp_::Difference, ring, far);
    QVERIFY(samePath(outside, ring));
    QVERIFY(outside.front().closed);
    QVERIFY(clipOpen(BooleanOp_::Intersection, ring, far).empty());
}

void ClipOpenTest::closedRingInsideRegion() {
    const Polylines ring{circle(20.0)};
    const Polygons big = rectRegion(-20, -20, 20, 20);

    QVERIFY(clipOpen(BooleanOp_::Difference, ring, big).empty());
    const Polylines inside = clipOpen(BooleanOp_::Intersection, ring, big);
    QVERIFY(samePath(inside, ring));
    QVERIFY(inside.front().closed);
}

QTEST_APPLESS_MAIN(ClipOpenTest)
#include "test_clipopen.moc"
