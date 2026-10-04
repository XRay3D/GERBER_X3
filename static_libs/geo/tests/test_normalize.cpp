// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
        Polyline shifted = rect(0, 0, 20, 10, false);                     // встречный обход...
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

    // Область Gerber с дыркой -- один контур с врезкой: перемычка пройдена
    // туда и обратно.
    void splitCutIn() {
        Polyline contour{
            {0, 0}, {90, 0}, {90, 65}, {0, 65},
            {0, 30}, {25, 30}, // врезка к углу дырки
            {45, 30}, {45, 50}, {25, 50}, {25, 30},
            {0, 30}, {0, 0}, // обратно по перемычке и в старт
        };
        contour.close();
        QVERIFY(!isExactContour(contour));

        const Polylines loops = splitSelfTouching(contour);
        QCOMPARE(loops.size(), std::size_t{2});

        const Polygons region = evenOdd(loops);
        QCOMPARE(region.size(), std::size_t{1});
        QCOMPARE(region.begin()->holes().size(), std::size_t{1});
        QCOMPARE(region.area(), 90.0 * 65.0 - 20.0 * 20.0);
    }

    // Та же врезка в настоящем файле: область G36 из Proba-F_Cu.gbr (KiCad
    // 10) вершина в вершину -- заливка зоны со срезанными углами и одним
    // окном. Файл открывался одними «путями апертур».
    void splitRealKiCadZone() {
        Polyline contour{
            {190.443039, -50.019685}, {190.488794, -50.072489}, {190.5, -50.124},
            {190.5, -114.876}, {190.480315, -114.943039}, {190.427511, -114.988794},
            {190.376, -115.0}, {99.624, -115.0}, {99.556961, -114.980315},
            {99.511206, -114.927511}, {99.5, -114.876},
            {99.5, -85.5}, {125.0, -85.5}, // врезка
            {144.5, -85.5}, {144.5, -65.0}, {125.0, -65.0}, {125.0, -85.5}, // окно
            {99.5, -85.5}, // обратно
            {99.5, -50.124}, {99.519685, -50.056961}, {99.572489, -50.011206},
            {99.624, -50.0}, {190.376, -50.0}, {190.443039, -50.019685},
        };
        contour.close();

        const Polylines loops = splitSelfTouching(contour);
        QCOMPARE(loops.size(), std::size_t{2});
        const double window = 19.5 * 20.5;
        QVERIFY(qFuzzyCompare(loops[0].area(), window));

        const Polygons region = evenOdd(loops);
        QCOMPARE(region.size(), std::size_t{1});
        QCOMPARE(region.begin()->holes().size(), std::size_t{1});
        QVERIFY(qFuzzyCompare(region.area(), loops[1].area() - window));
        QVERIFY(region.area() > 91.0 * 65.0 - window - 1.0); // срезы углов -- доли мм²
    }

    // Две дырки, каждая на своей врезке от внешнего контура.
    void splitTwoCutIns() {
        Polyline contour{
            {0, 0}, {100, 0}, {100, 50},
            {80, 50}, {80, 40}, {70, 40}, {70, 30}, {80, 30}, {80, 40}, {80, 50},
            {20, 50}, {20, 40}, {10, 40}, {10, 30}, {20, 30}, {20, 40}, {20, 50},
            {0, 50},
        };
        contour.close();
        const Polygons region = evenOdd(splitSelfTouching(contour));
        QCOMPARE(region.size(), std::size_t{1});
        QCOMPARE(region.begin()->holes().size(), std::size_t{2});
        QCOMPARE(region.area(), 5000.0 - 200.0);
    }

    // Цепочка: с середины обхода первой дырки -- ко второй и обратно.
    void splitChainFromMiddle() {
        Polyline contour{
            {0, 0}, {100, 0}, {100, 50}, {0, 50},
            {0, 20}, {10, 20}, // врезка к дырке 1
            {30, 20}, {30, 30}, // часть дырки 1
            {60, 30}, // перемычка к дырке 2
            {80, 30}, {80, 40}, {60, 40}, {60, 30}, // дырка 2
            {30, 30}, // обратно
            {10, 30}, {10, 20}, // остаток дырки 1
            {0, 20},
        };
        contour.close();
        const Polylines loops = splitSelfTouching(contour);
        QCOMPARE(loops.size(), std::size_t{3});
        const Polygons region = evenOdd(loops);
        QCOMPARE(region.size(), std::size_t{1});
        QCOMPARE(region.begin()->holes().size(), std::size_t{2});
        QCOMPARE(region.area(), 5000.0 - 200.0 - 200.0);
    }

    // Цепочка: дырка обойдена целиком, и из той же вершины -- к следующей.
    // Обратный ход по перемычке несёт лишнюю вершину.
    void splitChainFromSameVertex() {
        Polyline contour{
            {0, 0}, {100, 0}, {100, 50}, {0, 50},
            {0, 20}, {10, 20},
            {30, 20}, {30, 30}, {10, 30}, {10, 20}, // дырка 1 целиком
            {60, 5}, // перемычка к дырке 2 из той же вершины
            {80, 5}, {80, 15}, {60, 15}, {60, 5}, // дырка 2
            {10, 20},
            {5, 20}, // лишняя вершина на обратном ходе
            {0, 20},
        };
        contour.close();
        const Polylines loops = splitSelfTouching(contour);
        QCOMPARE(loops.size(), std::size_t{3});
        const Polygons region = evenOdd(loops);
        QCOMPARE(region.size(), std::size_t{1});
        QCOMPARE(region.begin()->holes().size(), std::size_t{2});
        QCOMPARE(region.area(), 5000.0 - 200.0 - 200.0);
    }

    // Остров внутри дырки на собственной перемычке -- тело, а не пустота.
    void splitIslandInHole() {
        Polyline contour{
            {0, 0}, {100, 0}, {100, 100}, {0, 100},
            {0, 20}, {20, 20}, // врезка к дырке
            {20, 80}, {80, 80}, {80, 20}, {20, 20}, // дырка 60x60, по часовой
            {40, 40}, // перемычка к острову
            {60, 40}, {60, 60}, {40, 60}, {40, 40}, // остров 20x20
            {20, 20}, {0, 20},
        };
        contour.close();
        const Polygons region = evenOdd(splitSelfTouching(contour));
        QCOMPARE(region.size(), std::size_t{2});
        QCOMPARE(region.area(), 10000.0 - 3600.0 + 400.0);
    }

    // Врезка к круглой дырке: прогибы петли проходят разбор нетронутыми.
    void splitKeepsArcs() {
        Polyline contour{
            {0, 0}, {40, 0}, {40, 40}, {0, 40},
            {0, 20}, {15, 20, 1.0}, {25, 20, 1.0}, {15, 20},
            {0, 20},
        };
        contour.close();
        const Polylines loops = splitSelfTouching(contour);
        QCOMPARE(loops.size(), std::size_t{2});
        const Polygons region = evenOdd(loops);
        QCOMPARE(region.size(), std::size_t{1});
        QCOMPARE(region.begin()->holes().size(), std::size_t{1});
        QVERIFY(qFuzzyCompare(region.area(), 1600.0 - pi * 25.0));
    }

    // Микрошип KiCad: шаг на единицу файла (10 нм) назад по той же прямой.
    // Петля -- тело термобарьера из korsakov_filter-F_Cu.gbr вершина в
    // вершину; с шипом точная геометрия её не берёт.
    void splitTrimsSpike() {
        Polyline contour{
            {24.303, -19.606}, {24.66864, -19.606}, {24.66863, -19.606}, // шип
            {24.70549, -19.6031}, {24.8632, -19.55728}, {25.00455, -19.47368},
            {25.00456, -19.47368}, {25.12068, -19.35756}, {25.12068, -19.35755},
            {25.20428, -19.2162}, {25.2501, -19.05849}, {25.253, -19.02163},
            {25.253, -18.856}, {24.303, -18.856},
        };
        contour.close();
        QVERIFY(!isExactContour(contour));

        const Polylines loops = splitSelfTouching(contour);
        QCOMPARE(loops.size(), std::size_t{1});
        QCOMPARE(loops.front().size(), std::size_t{13});
        QVERIFY(isExactContour(loops.front()));
        QVERIFY(qFuzzyCompare(loops.front().area(), contour.area()));
    }

    // Шип через стык замыкания и шип, чей срез открывает следующий.
    void splitTrimsSpikeAtSeam() {
        Polyline seam{{0, 0}, {10, 0}, {10, 10}, {0, 10}, {0, -2}};
        seam.close();
        const Polylines a = splitSelfTouching(seam);
        QCOMPARE(a.size(), std::size_t{1});
        QCOMPARE(a.front().area(), 100.0);
        QVERIFY(isExactContour(a.front()));

        // Срез {14,0} делает шипом {12,0}: дальше контур идёт в {11,0}.
        Polyline nested{{0, 0}, {10, 0}, {14, 0}, {12, 0}, {11, 0}, {10, 10}, {0, 10}};
        nested.close();
        const Polylines b = splitSelfTouching(nested);
        QCOMPARE(b.size(), std::size_t{1});
        QVERIFY(isExactContour(b.front()));
        QVERIFY(qFuzzyCompare(b.front().area(), 105.0));
    }

    // Простой контур разбор не трогает; повтор стартовой вершины в конце
    // (так область закрывает Gerber) петлёй не считается.
    void splitLeavesSimpleAlone() {
        Polyline contour = rect(0, 0, 20, 10, true);
        contour.push_back(contour.front());
        const Polylines loops = splitSelfTouching(contour);
        QCOMPARE(loops.size(), std::size_t{1});
        QCOMPARE(loops.front().size(), std::size_t{4});
        QCOMPARE(loops.front().signedArea(), 200.0);
    }
};

QTEST_MAIN(TestNormalize)
#include "test_normalize.moc"
