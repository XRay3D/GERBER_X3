/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "pocketoffset.h"
#include "project.h"

#include <geo/parallel.h>
#include <gi_dbg.h>

#include <limits>
#include <numbers>
#include <span>
#include <thread>
// #include <QStringBuilder>

namespace PocketOffset {

// //dbgPaths(clipFrame, u"clipFrame %1"_s.arg(tIdx), Qt::red);

void Creator::create() {
    assert(gcp.side() != GCode::On);

    const double depth = gcp.params[GCode::Params::Depth].toDouble();

    // Число шагов офсета берётся только у одной фрезы: с несколькими create()
    // всегда уходит в createMultiTool. FIXME inside steps
    if(gcp.tools.size() > 1)
        createMultiTool(gcp.tools, depth);
    else if(gcp.params.contains(OffsetSteps) && gcp.params[OffsetSteps].toInt() > 0)
        createFixedSteps(gcp.tools.front(), depth, gcp.params[OffsetSteps].toInt());
    else
        createStdFull(gcp.tools.front(), depth);
}

namespace {

// Концентрические петли от region с шагом step: петля i -- офсет region на
// base + 2*step*i (delta у Geo::Inflate -- ПОЛНАЯ ширина, а контур уезжает
// на её половину, отсюда удвоение; при base == 0 первая петля -- сам
// region). Отрицательный шаг идёт внутрь и кончается сам, когда сжимать
// нечего; положительный не кончится никогда, там limit обязателен.
//
// Каждая петля считается офсетом ИСХОДНОЙ области на суммарную глубину, а НЕ
// офсетом предыдущей петли. Результат тот же -- офсеты диском складываются,
// (X ⊖ D(a)) ⊖ D(b) = X ⊖ D(a+b), -- а цена разная, и разная катастрофически:
// цепочка каждый раз подаёт на вход результат прошлого офсета, у которого дуг
// уже вдвое больше, и растёт это геометрически. На меди gerber1.gbr фрезой
// 0.5 мм седьмой виток занимал 8.8 с и 2.6 ГБ (первый -- 0.4 с), а восьмой
// падал с bad_alloc; от исходной области все 22 витка идут по 80-130 мс при
// ровных 150 МБ.
//
// Тот же факт независимости петель кормит и потоки: серия считается ВОЛНАМИ
// по Geo::InflateSeries -- по офсету на ядро, -- потому что одиночный
// Inflate распараллелен лишь частично, и последовательный цикл упирался в
// его серийный хвост. base для того здесь и есть: он вносит в серию первый
// отступ на радиус фрезы, который иначе считался бы отдельным офсетом ДО
// начала волн. Волна может пересчитать пару-тройку петель ЗА концом заливки
// (пустых), лимит по габариту держит этот перехлёст малым: эрозия глубже
// полуширины габарита пуста заведомо.
Geo::Polylines concentricLoops(const Geo::Polygons& region, double step, int limit = {}, double base = {}) {
    Geo::Polylines loops;
    if(region.empty()) return loops;

    int maxLoops = limit ? limit : std::numeric_limits<int>::max();
    if(step < 0.0) {
        const QRectF box = region.boundingRect();
        const double reach = std::min(box.width(), box.height()) * 0.5;
        const int cap = 1 + std::max(0, static_cast<int>(std::ceil((reach + base * 0.5) / -step)));
        maxLoops = std::min(maxLoops, cap);
    }

    const int wave = std::max(2u, std::thread::hardware_concurrency());
    for(int i{}; i < maxLoops;) {
        // Точка отмены: волн заранее не знает никто, и одна волна большой
        // области -- уже заметное время. Прогресс здесь не двигают, его шаг --
        // тело целиком.
        Geo::checkCancelled();
        const int n = std::min(wave, maxLoops - i);
        std::vector<double> deltas(static_cast<std::size_t>(n));
        for(int k{}; k < n; ++k) deltas[static_cast<std::size_t>(k)] = base + 2.0 * step * (i + k);
        for(const Geo::Polygons& loop: Geo::InflateSeries(region, deltas)) {
            if(loop.empty()) return loops;
            // contours() отдаёт уже без вырожденного: схлопнувшиеся в точку
            // куски офсета Geo отсеивает при материализации сама.
            loops.append_range(loop.contours());
        }
        i += n;
    }
    return loops;
}

// Мелочь, которую фреза всё равно не выберет: обрывок, у которого и площадь, и
// периметр меньше самой фрезы, -- это не карман, а шум офсета.
void removeSmall(Geo::Polygons& region, double dOffset) {
    const double minArea = dOffset * dOffset * std::numbers::pi;
    const double minPerimeter = dOffset * 4.0;
    std::vector<Geo::Polygon> kept;
    for(const Geo::Polygon& polygon: region.all())
        if(std::abs(polygon.area()) >= minArea || polygon.perimeter() >= minPerimeter)
            kept.push_back(polygon);
    region = kept.empty() ? Geo::Polygons{} : Geo::Polygons{std::span<const Geo::Polygon>{kept}};
}

} // namespace

// Что выбирает фреза: изнутри -- саму медь, снаружи -- всё, что медью НЕ
// занято, то есть поле вокруг детали ВМЕСТЕ с дырками в ней. Отсюда и общий
// вид «граница минус медь»: разность сама разложит вложенность, и остров в
// дырке останется нетронутым.
//
// Различаются случаи только самой границей.
//
// Наружных контуров НЕСКОЛЬКО -- граница габаритная рамка с отступом в диаметр
// САМОЙ ШИРОКОЙ фрезы набора (инструменты приходят отсортированными по
// убыванию диаметра): ей врезаться в поле первой, и меньшего отступа ей не
// хватит. Запас чуть больше диаметра -- иначе поле пережимается в ноль там,
// где деталь касается своего же габарита.
//
// Контур ОДИН -- рамки нет вовсе: материал за деталью не кончается, границу
// там задавать нечем и незачем. Снаружи остаётся один проход вокруг детали
// (outerContourPass), а карман сводится к её дыркам -- границей служит её же
// внешний контур.
//
// groupedPaths(Grouping::Cutoff) для этого не годится, хотя прежде звался
// именно он: тот выбрасывает поле, стоит появиться хоть одной дырке в меди.
const std::vector<Geo::Polygon>& Creator::pocketBodies(const double depth) {
    if(gcp.side() != GCode::Outer)
        return groupedPaths(GCode::Grouping::Copper);

    groupedPss.clear();
    if(closedSrc.empty()) return groupedPss;

    const Geo::Polygons solid = solidBodies();

    Geo::Polylines bound;
    if(solid.all().size() == 1)
        bound.push_back(solid.all().front().outer());
    else {
        const double margin = gcp.tools.front().getDiameter(depth) * 1.005;
        bound.push_back(GCode::boundingFrame(closedSrc.boundingRect(), margin));
    }

    // Вычитается медь КАК ЕСТЬ, со всей своей вложенностью: остров в дырке --
    // материал, и трогать его нельзя.
    groupedPss = (Geo::Polygons{bound} - closedSrc).all();
    return groupedPss;
}

// Деталь снаружи -- это её силуэт: дырка границей детали не является, а остров
// в дырке -- не наружный контур. Объединение сплошных границ разбирает и то, и
// другое разом: контуры острова и его носителя сливаются в один.
Geo::Polygons Creator::solidBodies() const {
    Geo::Polylines outers;
    for(const Geo::Polygon& body: closedSrc.all())
        outers.push_back(body.outer());
    return Geo::Polygons{outers};
}

// Явный проход вокруг детали -- ровно на радиус от её внешнего контура.
// Заливка сама его не даёт: снаружи поле зажато между деталью и рамкой, а та
// отстоит всего на диаметр -- после сжатия на радиус с каждой стороны от поля
// остаётся волосяной след, при единственном же контуре рамки нет вовсе.
// Фиксированные шаги -- другое дело: там витки и так обходят деталь кругом,
// туда этот проход не идёт.
//
// Возвращает заметённую полосу: её вычитают из поля, чтобы фреза не прошла по
// одному месту дважды. Изнутри детали проходу этому места нет -- там первая
// петля заливки и есть обход по контуру.
Geo::Polygons Creator::outerContourPass() {
    if(gcp.side() != GCode::Outer || closedSrc.empty()) return {};

    Geo::Polylines loops;
    for(const Geo::Polygon& body: Geo::Inflate(solidBodies(), +toolDiameter).all())
        loops.push_back(body.outer());
    if(loops.empty()) return {};

    // Заметённое -- полоса шириной в диаметр вдоль самой траектории. Раздуть
    // петлю как область здесь нельзя: она ТЕЛО, и внутри неё лежит деталь, к
    // которой фреза не прикасалась.
    Geo::Polygons band = Geo::Polygons{loops}.boundaryBand(dOffset);

    // Материал у этой петли ВНУТРИ: это обход детали, а не стенка кармана. Для
    // stacking() сторону материала выражает ориентация контура (см. там же),
    // отсюда разворот -- и попутный ход получается тот же, что у Profile
    // снаружи.
    for(Geo::Polyline& loop: loops)
        loop.reverse();

    returnPs.append_range(std::move(loops));
    return band;
}

// Общая область заливки: тела, врезанные на радиус, минус уже выбранное, с
// отсевом мелочи. forbidden -- куда нельзя вести ЦЕНТР фрезы; minFeature --
// порог отсева (0 -- не отсеивать).
//
// Врезка -- по телам в несколько потоков: тела независимы, и каждый поток
// работает ТОЛЬКО со своим -- у потоков нет ни одного общего региона.
// Общий регион CGAL потокам давать нельзя вовсе: даже его разложение на
// полигоны лишь снаружи const (BFS-обходчик метит грани флагом visited), а
// конкурентные overlay поверх одного арранжемента роняют разность.
//
// Вычет выбранного -- ОДНОЙ разностью из объединения, а не по телу: тела
// не пересекаются, поэтому (⋃A) - F = ⋃(A - F), но одна разность против
// большого F дешевле десятков таких же. Отсев мелочи от переноса на
// объединение не меняется: он попольгонный.
Geo::Polygons Creator::fillRegion(const std::vector<Geo::Polygon>& bodies, const Geo::Polygons& forbidden, double minFeature) {
    std::vector<Geo::Polygons> eroded(bodies.size());
    Geo::parallelFor(bodies.size(), [&](std::size_t i) {
        eroded[i] = Geo::Inflate(Geo::Polygons{bodies[i]}, -toolDiameter);
        eroded[i].all(); // материализация тут же, в своём потоке
    });

    std::vector<Geo::Polygon> parts;
    for(const Geo::Polygons& region: eroded)
        parts.append_range(region.all());

    Geo::Polygons all{std::span<const Geo::Polygon>{parts}};
    if(!forbidden.empty()) all -= forbidden;
    if(minFeature > 0.0) removeSmall(all, minFeature);
    return all;
}

// Область, которую фреза заметает вдоль траектории. Прежде она собиралась
// отдельными офсетами внутрь и наружу; в точном домене это ровно раздувание
// самой траектории на диаметр.
void Creator::finishPocket(const Tool& tool, Geo::Polygons&& cutArea) {
    stacking(returnPs);
    if(returnPss.empty()) return; // пустой файл отправит createGc
    gcp.toolPathss = std::move(returnPss);
    gcp.setPocketAreaCurves(std::move(cutArea));
    file_ = new File{std::move(gcp)};
    file_->setFileName(tool.nameEnc());
}

void Creator::createFixedSteps(const Tool& tool, const double depth, int steps) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On) return;

    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter * 0.5;
    stepOver = tool.stepover();

    if(gcp.side() == GCode::Inner) {
        const std::vector<Geo::Polygon>& bodies = groupedPaths(GCode::Grouping::Copper);
        setMax(bodies.size());
        setCurrent();
        for(const Geo::Polygon& body: bodies) {
            // Отступ на радиус -- это base серии: офсеты диском складываются,
            // и первый уходит в общую волну, а не считается до неё.
            returnPs.append_range(concentricLoops(
                Geo::Polygons{body}, -stepOver, steps, -toolDiameter));
            incCurrent();
        }
    } else {
        setMax(1);
        setCurrent();
        // Снаружи петли идут ОТ детали наружу и сами кончиться не могут:
        // ограничение по числу шагов здесь единственное.
        returnPs.append_range(concentricLoops(
            closedSrc, +stepOver, steps, +toolDiameter));
        incCurrent();
    }

    if(returnPs.empty()) return; // пустой файл отправит createGc
    finishPocket(tool, Geo::Inflate(returnPs, toolDiameter));
}

void Creator::createStdFull(const Tool& tool, const double depth) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On) return;

    toolDiameter = tool.getDiameter(depth);
    dOffset = toolDiameter * 0.5;
    stepOver = tool.stepover();

    // Снаружи первым идёт обход детали по контуру, и заливка начинается уже за
    // заметённой им полосой.
    Geo::Polygons cutArea = outerContourPass();
    const Geo::Polygons forbidden = cutArea.empty()
        ? Geo::Polygons{}
        : Geo::Inflate(cutArea, -toolDiameter);

    // dbgPaths(cutArea.contours(), {});

    // Первая петля заливки отстоит от границы на РАДИУС: центр круглой фрезы
    // ближе не подойдёт. delta -- полная ширина, отсюда целый диаметр.
    const std::vector<Geo::Polygon>& bodies = pocketBodies(depth);
    setMax(bodies.size() + 1);
    setCurrent(1); // контурный проход позади

    // Витки идут телом за телом, а не одной серией по всей области: серия
    // объединения математически та же (эрозия объединения есть объединение
    // эрозий), но её мелкие витки таскают за собой ВСЮ область, и волна
    // ждёт самый дорогой из них. По-тельные серии короче и ровнее.
    Geo::Polylines fill;
    if(forbidden.empty()) {
        // Ни вычета, ни отсева -- серия целиком выражается офсетами самого
        // тела (первая петля на радиус -- это base), и врезка уходит в
        // общую волну вместо отдельного счёта до неё.
        for(const Geo::Polygon& body: bodies) {
            fill.append_range(concentricLoops(Geo::Polygons{body}, -stepOver, {}, -toolDiameter));
            incCurrent();
        }
    } else {
        // Мелочь отсеивается потому, что поле резал контурный проход: после
        // него у самой полосы остаются обрывки в волос толщиной.
        const Geo::Polygons region = fillRegion(bodies, forbidden, dOffset * 0.5);
        setMax(region.all().size() + 1);
        setCurrent(1);
        for(const Geo::Polygon& part: region.all()) {
            fill.append_range(concentricLoops(Geo::Polygons{part}, -stepOver));
            incCurrent();
        }
    }

    if(!fill.empty()) {
        cutArea |= Geo::Inflate(fill, toolDiameter);
        returnPs.append_range(std::move(fill));
    }

    // dbgPaths(returnPs, {});

    if(returnPs.empty()) return; // пустой файл отправит createGc
    finishPocket(tool, std::move(cutArea));
}

void Creator::createMultiTool(const std::vector<Tool>& tools, double depth) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On) return;

    const auto& bodies = pocketBodies(depth);

    setMax(2 * tools.size());
    setCurrent();

    // Выбранное предыдущими, более крупными фрезами: инструменты приходят
    // отсортированными по убыванию диаметра, и каждый следующий дорабатывает
    // только то, куда предыдущий не дотянулся.
    Geo::Polygons cleared;

    for(size_t tIdx{}, size = tools.size(); const Tool& tool: tools) {
        returnPs.clear();
        returnPss.clear();
        // Файл прошлой фрезы больше не наш: не обнулив его, пустая последняя
        // итерация отправила бы его вторично -- createGc в конце шлёт file_
        // как есть.
        file_ = nullptr;
        toolDiameter = tool.getDiameter(depth);
        dOffset = toolDiameter * 0.5;
        stepOver = tool.stepover();

        // Обход детали по контуру достаётся САМОЙ ШИРОКОЙ фрезе -- она идёт
        // первой, и поле у самой детали её же и ждёт. Прочим там делать
        // нечего: заметённую полосу они обходят, как и всё уже выбранное.
        Geo::Polygons cutArea = tIdx == 0 ? outerContourPass() : Geo::Polygons{};
        incCurrent();

        // Куда нельзя вести ЦЕНТР этой фрезы: вглубь уже выбранного, отступив
        // от его края на радиус. Ближе к краю фреза ещё снимает материал.
        const Geo::Polygons obstacles = cleared | cutArea;
        const Geo::Polygons forbidden = obstacles.empty()
            ? Geo::Polygons{}
            : Geo::Inflate(obstacles, -toolDiameter);

        // Последней фрезе мелочь оставляем: дочищать после неё уже некому.
        Geo::Polylines fill;
        const Geo::Polygons region = fillRegion(bodies, forbidden, tIdx + 1 == size ? dOffset * 0.5 : dOffset * 2.0);
        for(const Geo::Polygon& part: region.all())
            fill.append_range(concentricLoops(Geo::Polygons{part}, -stepOver));
        incCurrent();

        if(!fill.empty()) {
            cutArea |= Geo::Inflate(fill, toolDiameter);
            returnPs.append_range(std::move(fill));
        }

        if(!returnPs.empty()) {
            stacking(returnPs);
            if(!returnPss.empty()) {
                cleared |= cutArea;
                // Параметры КОПИРУЮТСЯ, а не переносятся: прежний код делал
                // File{std::move(gcp)} прямо в цикле, и второму инструменту
                // доставался уже выпотрошенный gcp -- ни инструментов, ни
                // исходной геометрии.
                GCode::Params toolGcp = gcp;
                toolGcp.params[GCode::Params::MultiToolIndex] = static_cast<ssize_t>(tIdx);
                toolGcp.toolPathss = std::move(returnPss);
                toolGcp.setPocketAreaCurves(std::move(cutArea));
                file_ = new File{std::move(toolGcp)};
                file_->setFileName(tool.nameEnc());
            }
        }

        // Последний файл -- хоть готовый, хоть пустой -- отправит сам
        // Creator::createGc.
        if(++tIdx < size) emit fileReady(file_);
    }
}

File::File()
    : GCode::File() { }

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        regenerate();
    }
}

void File::createGi() {
    // switch (gcp.gcType) {
    // case GCode::Raster:
    // createGiRaster();
    // break;
    // case GCode::Pocket:
    createGiPocket();
    // break;
    // default: break;
    // }

    itemGroup()->setVisible(true);
}

} // namespace PocketOffset
