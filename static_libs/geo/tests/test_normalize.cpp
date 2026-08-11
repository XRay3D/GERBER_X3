// Нормализация «чертёжной» геометрии: склейка обрывков и even-odd.
//
// Проверяется ровно то, чем чертёж отличается от гербера: ориентация обхода
// произвольна, а контур разрезан на отдельные сущности. Канон Geo::Polygons
// того и другого не переживает, и слой DXF доходил до g-кода одними
// окружностями -- их Geo::circle строит против часовой.

#include "geo/boolean.h"
#include "geo/util.h"

#include <QTest>

using namespace Geo;

namespace {

// Прямоугольник заданного обхода: ccw -- против часовой, иначе по часовой.
Polyline rect(double x, double y, double w, double h, bool ccw) {
    Polyline p{
        {x,     y    },
        {x + w, y    },
        {x + w, y + h},
        {x,     y + h}
    };
    if(!ccw) p.reverse();
    p.close();
    return p;
}

// Слой "0" из EL-4134Ex_MS_V5a_ascii.dxf -- так, как его отдаёт разбор: три
// окружности (в bulge-виде это две вершины с прогибом 1) и четыре замкнутые
// LWPOLYLINE.
//
// Обход РАЗНЫЙ: окружности против часовой (их строит Geo::circle), все четыре
// полилинии -- по часовой, как нарисовал чертёжник. Ровно эта смесь и давала
// «профиль игнорирует всё кроме окружностей».
//
// Расположение: рамка 71x32.5 содержит все три окружности и рамку 11x5; два
// малых прямоугольника лежат отдельно, выше по Y.
Polylines dxfLayerZero() {
    Polylines contours{
        Polyline{{53.0, 10.25, 1.0}, {56.0, 10.25, 1.0}}, // окружность d=3
        Polyline{{72.6, 35.5, 1.0}, {75.4, 35.5, 1.0}}, // окружность d=2.8
        Polyline{{7.599999999999985, 35.5, 1.0}, {10.399999999999986, 35.5, 1.0}}, // окружность d=2.8
        Polyline{{77.0, 38.5}, {77.0, 6.0},
                 {5.999999999999986, 6.0}, {5.999999999999986, 38.5}}, // рамка 71 x 32.5
        Polyline{{69.49999999999997, 7.75}, {58.49999999999997, 7.75},
                 {58.49999999999997, 12.75}, {69.49999999999997, 12.75}}, // рамка 11 x 5
        Polyline{{51.98649167310371, 73.92468045718454},
                 {51.98649167310371, 69.92468045718454},
                 {48.05, 69.92468045718454}, {48.05, 73.92468045718454}},
        Polyline{{38.98649167310371, 73.92468045718454},
                 {38.98649167310371, 69.92468045718454},
                 {35.05, 69.92468045718454}, {35.05, 73.92468045718454}},
    };
    for(Polyline& contour: contours) contour.close();
    return contours;
}

} // namespace

class TestNormalize : public QObject {
    Q_OBJECT

private slots:
    // Канон читает контур по часовой как пустоту: вычитать её не из чего, и
    // регион выходит пустым. Это и есть исходный дефект.
    void canonLosesClockwise() {
        const Polyline cw = rect(0, 0, 20, 10, false);
        QVERIFY(cw.signedArea() < 0);
        QCOMPARE(Polygons{Polylines{cw}}.size(), std::size_t{0});
    }

    void evenOddIgnoresWinding() {
        for(bool ccw: {true, false}) {
            const Polygons region = evenOdd(Polylines{rect(0, 0, 20, 10, ccw)});
            QCOMPARE(region.size(), std::size_t{1});
            QCOMPARE(region.area(), 200.0);
        }
    }

    // Окружность в bulge-виде -- ДВЕ вершины с прогибом 1. Любой порог «меньше
    // трёх вершин -- не контур» выкидывает именно её.
    void evenOddKeepsCircle() {
        const Polygons region = evenOdd(Polylines{circle(10.0)});
        QCOMPARE(region.size(), std::size_t{1});
        QVERIFY(qFuzzyCompare(region.area(), pi * 25.0));
    }

    // Отверстие в теле: у окружности внутри прямоугольника ориентация та же,
    // отличить дырку можно только по расположению.
    //
    // Концентричность тут -- ВЫРОЖДЕННЫЙ случай: он симметричен, и ошибка,
    // которая проявится на любом другом расположении, на нём не видна. Поэтому
    // отверстие сдвигается на единицу в каждую сторону, а проверяется инвариант
    // «сколько контуров подано -- столько и вышло»: непересекающиеся контуры
    // even-odd не режет и не сливает, он лишь раскладывает их на тело и дырки.
    void evenOddCircleAsHole() {
        for(QPointF offset: {
                QPointF{0,  0 },
                {1,  0 },
                {-1, 0 },
                {0,  1 },
                {0,  -1},
                {1,  1 },
                {-1, -1}
        }) {
            Polyline hole = circle(10.0);
            translate(hole, offset);
            const Polylines input{rect(-10, -10, 20, 20, false), std::move(hole)};

            const Polygons ring = evenOdd(input);
            QCOMPARE(ring.size(), std::size_t{1});
            QCOMPARE(ring.contours().size(), input.size());
            QVERIFY(qFuzzyCompare(ring.area(), 400.0 - pi * 25.0));
        }
    }

    // Вложенность -- из взаимного расположения: внутренний контур накрывает
    // точку второй раз, значит она снаружи. Обходы обоих одинаковые, по
    // ориентации дырки было бы не отличить.
    void evenOddNestsByContainment() {
        for(QPointF offset: {
                QPointF{0,  0 },
                {1,  0 },
                {-1, 0 },
                {0,  1 },
                {0,  -1}
        }) {
            Polyline hole = rect(5, 3, 10, 4, false);
            translate(hole, offset);
            const Polylines input{rect(0, 0, 20, 10, false), std::move(hole)};

            const Polygons ring = evenOdd(input);
            QCOMPARE(ring.size(), std::size_t{1});
            QCOMPARE(ring.contours().size(), input.size());
            QCOMPARE(ring.area(), 200.0 - 40.0);
        }
    }

    // Геометрия реального слоя, а не выдуманная: синтетика проверяет свойства
    // по одному, а тут вся картина разом -- окружности из двух вершин, обход по
    // часовой у всех семи контуров, вложенность вперемешку с раздельными телами.
    //
    // Инвариант тот же: контуры не пересекаются, значит even-odd их не режет и
    // не сливает -- сколько подано, столько и вышло. Раскладываются они в три
    // тела: рамка с четырьмя дырками (три отверстия и внутренняя рамка) плюс
    // два отдельных прямоугольника выше по Y.
    void realDxfLayerZero() {
        const Polylines input = dxfLayerZero();
        QCOMPARE(input.size(), std::size_t{7});
        // Обход разный: окружности против часовой, полилинии по часовой.
        std::size_t ccw{};
        for(const Polyline& contour: input) ccw += contour.signedArea() > 0;
        QCOMPARE(ccw, std::size_t{3});

        // Канон теряет слой ЦЕЛИКОМ, вместе с окружностями: рамка 71x32.5
        // обойдена по часовой, то есть читается пустотой, и вычитает всё, что
        // внутри неё. По одному контуру за раз (как строилась заливка каждого
        // объекта) окружности ещё проходили -- отсюда и вид «остались только
        // они», хотя на самом деле не выживал никто.
        QVERIFY(Polygons{input}.empty());

        const Polygons region = evenOdd(input);
        QCOMPARE(region.contours().size(), input.size());
        QCOMPARE(region.size(), std::size_t{3});

        // Все четыре дырки -- у рамки, и именно ДЫРКАМИ её полигона, а не
        // потерянными телами: пообъектная сборка слоя (тело за телом, потом
        // union) их проглатывала -- рамка выходила сплошной.
        for(const Polygon& body: region)
            QCOMPARE(body.holes().size(), body.area() > 100.0 ? std::size_t{4} : std::size_t{0});

        constexpr double small = 3.93649167310371 * 4.0; // два одинаковых прямоугольника
        const double expected = 71.0 * 32.5              // рамка
            - 11.0 * 5.0                                 // внутренняя рамка -- дырка
            - pi * (1.5 * 1.5 + 1.4 * 1.4 + 1.4 * 1.4)   // три отверстия
            + small * 2.0;
        QVERIFY(qFuzzyCompare(region.area(), expected));
    }

    // Контур, обведённый дважды, -- обычный мусор чертежа: XOR превратил бы
    // пару в пустоту (два накрытия), но это всё одна линия. Из совпадающих
    // остаётся один -- даже когда дубликат начат с другого узла или обойдён
    // навстречу.
    void evenOddDropsDuplicates() {
        Polyline direct = rect(0, 0, 20, 10, true);
        Polyline shifted = rect(0, 0, 20, 10, false); // встречный обход...
        std::rotate(shifted.begin(), shifted.begin() + 2, shifted.end()); // ...и другой старт

        const Polygons region = evenOdd(Polylines{direct, shifted, direct}); // тройная копия
        QCOMPARE(region.size(), std::size_t{1});
        QCOMPARE(region.area(), 200.0);

        // Окружность в bulge-виде дедупликация тоже обязана узнавать.
        const Polygons disc = evenOdd(Polylines{circle(10.0), circle(10.0)});
        QCOMPARE(disc.size(), std::size_t{1});
        QVERIFY(qFuzzyCompare(disc.area(), pi * 25.0));
    }

    // Дубликат не должен ломать вложенность остальных: слой "0" с обведённой
    // второй раз рамкой обязан разложиться ровно так же, как без неё.
    void evenOddDuplicateKeepsNesting() {
        Polylines input = dxfLayerZero();
        input.push_back(input[3].reversed()); // рамка 71x32.5 ещё раз, навстречу

        const Polygons region = evenOdd(input);
        QCOMPARE(region.size(), std::size_t{3});
        for(const Polygon& body: region)
            QCOMPARE(body.holes().size(), body.area() > 100.0 ? std::size_t{4} : std::size_t{0});
    }

    // Габарит платы в DXF -- десяток отдельных LINE и ARC.
    void stitchClosesCutContour() {
        Polylines pieces{
            Polyline{{0, 0},   {0, 10} },
            Polyline{{20, 10}, {20, 0} }, // порядок и направление вразнобой
            Polyline{{0, 10},  {20, 10}},
            Polyline{{20, 0},  {0, 0}  },
        };
        const Normalized norm = normalize(pieces, exitWeldTolerance);
        QVERIFY(norm.open.empty());
        QCOMPARE(norm.region.size(), std::size_t{1});
        QCOMPARE(norm.region.area(), 200.0);
    }

    // Клей -- допуск, а не ноль: у чертежа концы соседних сущностей сходятся
    // не побитово.
    void stitchHonoursGlue() {
        auto pieces = [](double gap) {
            return Polylines{
                Polyline{{0, 0},    {0, 10} },
                Polyline{{0, 10},   {20, 10}},
                Polyline{{20, 10},  {20, 0} },
                Polyline{{20, gap}, {0, 0}  }, // разрыв gap в стыке
            };
        };
        QCOMPARE(normalize(pieces(0.05), 0.1).region.size(), std::size_t{1});
        QVERIFY(normalize(pieces(0.05), 0.001).region.empty());
    }

    // Что замкнуть не вышло -- не теряется: профиль по разомкнутой линии такая
    // же траектория.
    void openStaysOpen() {
        const Normalized norm = normalize(Polylines{
                                              Polyline{{0, 0}, {10, 0}}
        },
            exitWeldTolerance);
        QVERIFY(norm.region.empty());
        QCOMPARE(norm.open.size(), std::size_t{1});
    }
};

QTEST_MAIN(TestNormalize)
#include "test_normalize.moc"
