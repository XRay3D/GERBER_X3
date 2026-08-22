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

#include <geo/inflatepasses.h>

#include <gi_dbg.h>

#include <algorithm>
#include <chrono>
#include <numbers>
#include <span>

#include <QFile>
#include <QTextStream>
// #include <QStringBuilder>

namespace PocketOffset {

// //dbgPaths(clipFrame, u"clipFrame %1"_s.arg(tIdx), Qt::red);

void Creator::create() {
    assert(gcp.side() != GCode::On);

    // Силуэт кэшируется на прогон, а closedSrc к этому моменту уже новый:
    // Creator::reset() не виртуален и о нашем кэше не знает.
    solidCache_.reset();

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

// По-витковая телеметрия и эталонный дамп для A/B-сверки путей расчёта --
// только при GX3_POCKET_DUMP=<файл>: в журнал уходит время, число петель и
// вершин каждого витка, в файл -- по строке на петлю (вершины, периметр,
// габарит), чтобы сверять геометрию сопоставлением, без привязки к порядку.
class PassLog {
public:
    PassLog() {
        if(const QByteArray path = qgetenv("GX3_POCKET_DUMP"); !path.isEmpty()) {
            file_.setFileName(QString::fromUtf8(path));
            file_.open(QIODevice::Append | QIODevice::Text);
        }
    }
    void operator()(int pass, std::chrono::steady_clock::time_point start, const Geo::Polylines& contours) {
        using ms = std::chrono::duration<double, std::milli>;
        const double elapsed = ms{std::chrono::steady_clock::now() - start}.count();
        std::size_t vertices{};
        for(const Geo::Polyline& contour: contours) vertices += contour.size();
        if(!file_.isOpen()) return;
        qDebug("pocket pass %3d: %8.2f ms, %4zu loops, %7zu pts", pass, elapsed, contours.size(), vertices);
        QTextStream out{&file_};
        // Время -- отдельной #-строкой: на Windows журнал GUI-приложения
        // уходит в OutputDebugString, и файл -- единственный надёжный канал;
        // сверка же геометрии остаётся байтовой после grep -v '^#'.
        out << "# pass " << pass << ": " << QString::number(elapsed, 'f', 2) << " ms, "
            << vertices << " pts\n";
        out << "pass " << pass << " loops " << contours.size() << '\n';
        for(const Geo::Polyline& contour: contours) {
            const QRectF r = contour.boundingRect();
            out << contour.size() << ' ' << QString::number(contour.perimeter(), 'f', 6) << ' '
                << QString::number(r.x(), 'f', 6) << ' ' << QString::number(r.y(), 'f', 6) << ' '
                << QString::number(r.width(), 'f', 6) << ' ' << QString::number(r.height(), 'f', 6) << '\n';
        }
    }
private:
    QFile file_;
};

// Концентрические петли от region с шагом step: первая -- сам region, каждая
// следующая смещена на step. delta у Geo::Inflate -- ПОЛНАЯ ширина, а контур
// уезжает на её половину, отсюда удвоение. Отрицательный шаг идёт внутрь и
// кончается сам, когда сжимать нечего; положительный не кончится никогда, там
// limit обязателен.
//
// Изнутри детали и в многоинструментальном режиме витки считаются ЭТИМ
// монолитным офсетом области; снаружи поле идёт пофигурно через
// Geo::InflatePasses (см. createStdFull/createFixedSteps) -- там это же
// принятие базы работает по каждой кляксе отдельно и включается с первых
// витков, а не с дюжины.
//
// Витки считаются ПО ОЧЕРЕДИ -- и это не упущение.
//
// Формально они независимы: виток i -- офсет исходной области на 2*step*i, и
// от витка i-1 не зависит ничем. Считать их пачкой по числу ядер, однако,
// нельзя, и попытка была: Geo::Polygons материализует bulge-вид ЛЕНИВО и
// кэширует его в себе (Polygons::Impl::view), а точный PolySet достраивает
// свои структуры поиска при первом же обращении. Одновременное чтение одной
// и той же области из нескольких потоков -- гонка на этих самых кэшах:
// прогон переставал быть воспроизводимым (1145 петель, потом 1269, потом
// 1825 на тех же входных данных).
//
// Копия области на поток гонку снимает, но не окупается: замер показал рост
// загрузки лишь до 317% при 12 ядрах и 710 МБ памяти -- цепочка упирается в
// аллокатор точных чисел (GMP), а не в счёт. Быстрее от этого не стало вовсе.
//
// Каждая петля считается офсетом БАЗЫ на суммарную от неё глубину, а НЕ
// офсетом предыдущей петли В ТОЧНОМ ДОМЕНЕ. Цепочка точных результатов --
// катастрофа, и попытка была: у результата офсета дуг уже вдвое больше
// (нарезка свипа плюс диски стыков), и растёт это геометрически. На меди
// gerber1.gbr фрезой 0.5 мм седьмой виток занимал 8.8 с и 2.6 ГБ (первый --
// 0.4 с), а восьмой падал с bad_alloc; от исходной области все 22 витка шли
// по 80-130 мс при ровных 150 МБ.
//
// База при этом НЕ обязана всю дорогу оставаться исходной областью: витки
// сами стремительно упрощаются (изнутри область тает, снаружи мелочь границы
// зарастает), и считать сороковой виток от границы во все её 3000 кривых --
// выброшенная работа. Когда МАТЕРИАЛИЗОВАННАЯ петля (bulge-вид после сварки,
// чистки коллинеарного и сшивки дуг -- см. Cgal::toPolyline) оказывается
// проще базы, регион пересобирается из её контуров и становится новой базой
// -- дальше витки идут уже от неё. Это цепочка, но НЕ та: точный домен
// покидается через полную чистку, геометрический рост дуг обнуляется, а цена
// перехода -- одно округление в double, сдвиг границы в пределах сварочного
// допуска; суммарно и на полсотне переходов это на порядок мельче шага витка.
//
// Пересборка обязана СОХРАНИТЬ все контуры: контур, ставший после округления
// самокасающимся или вырожденным, конструктор региона молча выбросил бы
// (потерянная дырка -- зарез по островку меди). Несовпадение числа контуров
// -- переход просто не состоялся, витки продолжаются от прежней базы. Ровно
// это и происходит на ранних витках: пока область полна обмылков на грани
// вырождения, цепочка не цепляется, и первые витки честно идут от исходной
// области -- замер на coldfire-плате: переходы начинаются с 12-го витка из
// полусотни, и хвост дешевеет с сотен миллисекунд до единиц.
//
// ПЕРВЫЙ виток точный, последующие -- черновые. Чистовую поверхность задаёт
// только виток 0 (он и есть сам region), витки дальше выбирают припуск, и
// точнее допуска coarse им быть незачем: Inflate с допуском недобора coarse
// прореживает капсулы границы тем сильнее, чем дальше виток от базы (см.
// Geo::boundaryBand). Ошибка чернового офсета направлена всегда К детали
// (недобор, не перебор) и не копится: виток i -- офсет базы, а не витка i-1.
Geo::Polylines concentricLoops(const Geo::Polygons& region, double step, double coarse, int limit = {}) {
    Timer t{"concentricLoops"};
    Geo::Polylines loops;

    Geo::Polygons adopted;                    // владение принятой базой
    const Geo::Polygons* base = &region;      // текущая база витков
    int basePass{};                           // номер витка, где база принята
    std::size_t baseVertices{};               // её сложность; 0 -- ещё не мерена
    auto verticesOf = [](const Geo::Polylines& contours) {
        std::size_t total{};
        for(const Geo::Polyline& contour: contours) total += contour.size();
        return total;
    };

    PassLog passLog;
    for(int i{}; !limit || i < limit; ++i) {
        // Точка отмены: витков заранее не знает никто, и один Inflate большой
        // области -- уже заметное время. Прогресс здесь не двигают, его шаг --
        // тело целиком.
        Geo::checkCancelled();
        const auto passStart = std::chrono::steady_clock::now();
        const Geo::Polygons loop = i
            ? Geo::Inflate(*base, 2.0 * step * (i - basePass), coarse)
            : region;
        if(loop.empty()) break;
        // contours() отдаёт уже без вырожденного: схлопнувшиеся в точку
        // куски офсета Geo отсеивает при материализации сама.
        Geo::Polylines contours = loop.contours();
        passLog(i, passStart, contours);

        if(i) {
            if(!baseVertices) baseVertices = verticesOf(base->contours());
            const std::size_t loopVertices = verticesOf(contours);
            if(loopVertices < baseVertices) {
                Geo::Polygons candidate{contours};
                if(candidate.contours().size() == contours.size()) {
                    adopted = std::move(candidate);
                    base = &adopted;
                    basePass = i;
                    baseVertices = loopVertices;
                }
            }
        }

        loops.append_range(std::move(contours));
    }
    return loops;
}

// Допуск прореживания области для витков после первого. Бюджет ошибки задают
// два отказа: зарез (виток ушёл к детали ближе своего отступа i*step, минимум
// на первом -- step) и гребешок (соседние витки разошлись дальше диаметра:
// step + 2*допуск > toolDiameter). Четверть меньшего из бюджетов оставляет
// четырёхкратный запас; допуск мельче сварочного порога выхода не имеет
// смысла -- такой шум материализация снимает и так, прореживание выключается.
double coarseTolerance(double stepOver, double toolDiameter) {
    const double tolerance = 0.25 * std::min(stepOver, toolDiameter - stepOver);
    return tolerance < Geo::exitWeldTolerance * 2.0 ? 0.0 : tolerance;
}

// Мелочь, которую фреза всё равно не выберет: обрывок, у которого и площадь, и
// периметр меньше самой фрезы, -- это не карман, а шум офсета.
void removeSmall(Geo::Polygons& region, double dOffset) {
    const double minArea      = dOffset * dOffset * std::numbers::pi;
    const double minPerimeter = dOffset * 4.0;
    std::vector<Geo::Polygon> kept;
    for(const Geo::Polygon& polygon: region.all())
        if(std::abs(polygon.area()) >= minArea || polygon.perimeter() >= minPerimeter)
            kept.push_back(polygon);
    region = kept.empty() ? Geo::Polygons{} : Geo::Polygons{std::span<const Geo::Polygon>{kept}};
}

// Контуры региона без мелочи (см. removeSmall) -- БЕЗ пересборки региона:
// витку нужны только контуры, а пересборка стоила бы объединения, в котором
// гигантское тело поля проходит полный свип валидации CGAL заново.
Geo::Polylines contoursWithoutSmall(const Geo::Polygons& region, double dOffset) {
    const double minArea      = dOffset * dOffset * std::numbers::pi;
    const double minPerimeter = dOffset * 4.0;
    Geo::Polylines contours;
    for(const Geo::Polygon& polygon: region.all()) {
        // Габарит -- дешёвая отсечка: периметр не меньше удвоенного
        // полупериметра габарита, и у крупного точные мерки не считаются.
        const QRectF box = polygon.boundingRect();
        if(2.0 * (box.width() + box.height()) >= minPerimeter
            || std::abs(polygon.area()) >= minArea || polygon.perimeter() >= minPerimeter)
            contours.append_range(polygon.contours());
    }
    return contours;
}

// Витки ПОФИГУРНОГО движка (Geo::InflatePasses): виток i -- pass на
// полуширине start + step*i; первый точный, остальные черновые (см.
// coarseTolerance), как и у concentricLoops. Отсев мелочи -- по витку:
// общей области, из которой её можно было бы убрать один раз, здесь нет.
// Конец -- пустой виток либо limit.
Geo::Polylines fieldLoops(Geo::InflatePasses& passes, double start, double step, double coarse,
    int limit = {}, double minFeature = 0.0) {
    Timer t{"fieldLoops"};
    Geo::Polylines loops;
    PassLog passLog;
    for(int i{}; !limit || i < limit; ++i) {
        Geo::checkCancelled();
        const auto passStart = std::chrono::steady_clock::now();
        const std::vector<Geo::Polygons> parts = passes.passParts(start + step * i, i ? coarse : 0.0);
        Geo::Polylines contours;
        for(const Geo::Polygons& part: parts)
            contours.append_range(minFeature > 0.0 ? contoursWithoutSmall(part, minFeature) : part.contours());
        if(contours.empty()) break;
        passLog(i, passStart, contours);
        loops.append_range(std::move(contours));
    }
    return loops;
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

    const Geo::Polygons& solid = solidBodies();

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
//
// Считается ОДИН раз на прогон и запоминается: силуэт нужен и pocketBodies, и
// outerContourPass, а сборка региона из полутысячи контуров -- полноценное
// объединение в точном домене, и звалось оно дважды подряд с одним и тем же
// результатом. Кэш сбрасывает Creator::reset() вместе с closedSrc.
const Geo::Polygons& Creator::solidBodies() const {
    if(!solidCache_) {
        Geo::Polylines outers;
        for(const Geo::Polygon& body: closedSrc.all())
            outers.push_back(body.outer());
        solidCache_.emplace(outers);
    }
    return *solidCache_;
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

// Заливка одного тела концентрическими петлями с вычетом уже выбранного.
// forbidden -- куда нельзя вести ЦЕНТР фрезы; minFeature -- порог отсева
// мелочи (0 -- не отсеивать).
Geo::Polylines Creator::fillBody(const Geo::Polygon& body, const Geo::Polygons& forbidden, double minFeature) {
    Geo::Polygons region = Geo::Inflate(Geo::Polygons{body}, -toolDiameter);
    if(!forbidden.empty()) region -= forbidden;
    if(minFeature > 0.0) removeSmall(region, minFeature);
    if(region.empty()) return {};
    return concentricLoops(region, -stepOver, coarseTolerance(stepOver, toolDiameter));
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
    dOffset      = toolDiameter * 0.5;
    stepOver     = tool.stepover();

    if(gcp.side() == GCode::Inner) {
        const std::vector<Geo::Polygon>& bodies = groupedPaths(GCode::Grouping::Copper);
        setMax(bodies.size());
        setCurrent();
        for(const Geo::Polygon& body: bodies) {
            returnPs.append_range(concentricLoops(
                Geo::Inflate(Geo::Polygons{body}, -toolDiameter), -stepOver,
                coarseTolerance(stepOver, toolDiameter), steps));
            incCurrent();
        }
    } else {
        setMax(1);
        setCurrent();
        // Снаружи петли идут ОТ детали наружу и сами кончиться не могут:
        // ограничение по числу шагов здесь единственное. Считаются
        // пофигурно: каждое тело меди раздувается само по себе, виток --
        // объединение (см. Geo::InflatePasses).
        Geo::InflatePasses passes;
        passes.addSolids(closedSrc);
        returnPs.append_range(fieldLoops(passes, dOffset, stepOver,
            coarseTolerance(stepOver, toolDiameter), steps));
        incCurrent();
    }

    if(returnPs.empty()) return; // пустой файл отправит createGc
    finishPocket(tool, {});      // заливку рисует createGiPocket -- см. createStdFull
}

void Creator::createStdFull(const Tool& tool, const double depth) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On) return;

    toolDiameter = tool.getDiameter(depth);
    dOffset      = toolDiameter * 0.5;
    stepOver     = tool.stepover();

    // Снаружи первым идёт обход детали по контуру, и заливка начинается уже за
    // заметённой им полосой.
    Geo::Polygons cutArea         = outerContourPass();
    const Geo::Polygons forbidden = cutArea.empty()
        ? Geo::Polygons{}
        : Geo::Inflate(cutArea, -toolDiameter);

    // dbgPaths(cutArea.contours(), {});

    // Первая петля заливки отстоит от границы на РАДИУС: центр круглой фрезы
    // ближе не подойдёт. delta -- полная ширина, отсюда целый диаметр.
    const std::vector<Geo::Polygon>& bodies = pocketBodies(depth);
    setMax(bodies.size() + 1);
    setCurrent(1); // контурный проход позади

    // Мелочь отсеивается только там, где поле резал контурный проход: после
    // него у самой полосы остаются обрывки в волос толщиной.
    const double minFeature = forbidden.empty() ? 0.0 : dOffset * 0.5;

    Geo::Polylines fill;
    if(gcp.side() == GCode::Outer) {
        // Снаружи поле -- одно тело на всю плату с дыркой на каждый медный
        // кластер, и монолитный офсет такого тела ворочает всю плату на
        // каждом витке (см. concentricLoops). Витки считаются ПОФИГУРНО
        // (Geo::InflatePasses): внешний контур каждого тела поля усаживается,
        // каждая его дырка раздувается сама по себе -- в своём потоке, -- а
        // виток собирается объединением и разностью; слившиеся кляксы
        // принимаются базой разом, обмелевшие группы схлопываются в своё
        // поле и дальше усаживаются как монолит. Замер на coldfire-плате,
        // фреза 0.5 мм (одна сборка, A/B): слой VDD 11.7 -> 8.6 с, Top
        // 15.9 -> 13.4 с. Запретная полоса входит в расписание со сдвигом:
        // на первом витке вычитается как есть, дальше растёт шагом витков
        // -- ровно как если бы её вычли из области один раз.
        Geo::InflatePasses passes;
        std::vector<std::size_t> groups;
        for(const Geo::Polygon& body: bodies) groups.push_back(passes.addField(body));
        // Запретная полоса -- в группу того тела поля, в котором лежит: там
        // она сольётся с кластерами меди, а не будет вычитаться из всей
        // области на каждом витке. Ни в одном теле (на самом шве) --
        // глобально.
        for(const Geo::Polygon& band: forbidden.all()) {
            std::size_t group = Geo::InflatePasses::global;
            if(!band.outer().empty()) {
                const QPointF probe = band.outer().front();
                for(std::size_t k = 0; k < bodies.size(); ++k)
                    if(bodies[k].contains(probe)) {
                        group = groups[k];
                        break;
                    }
            }
            passes.addSolid(band, group, -dOffset);
        }
        fill = fieldLoops(passes, dOffset, stepOver, coarseTolerance(stepOver, toolDiameter), {}, minFeature);
        setCurrent(bodies.size() + 1);
    } else
        for(const Geo::Polygon& body: bodies) {
            fill.append_range(fillBody(body, forbidden, minFeature));
            incCurrent();
        }

    // Заметённая заливка НЕ считается: раздувание всей траектории в точном
    // домене -- самая дорогая одиночная операция прогона (диск на вершину и
    // капсула на ребро по КАЖДОМУ витку, затем объединение всех), а нужна
    // она была лишь для картинки. Рисует её теперь File::createGiPocket --
    // обводкой траектории пером в диаметр фрезы, ровно как Profile и Raster
    // (см. gc_file.cpp).
    //
    // cutArea при этом остаётся полосой контурного прохода: она уходит в
    // returnPs как настоящая траектория, а не как заливка.
    if(!fill.empty()) returnPs.append_range(std::move(fill));

    // dbgPaths(returnPs, {});

    if(returnPs.empty()) return; // пустой файл отправит createGc
    finishPocket(tool, {});
}

void Creator::createMultiTool(const std::vector<Tool>& tools, double depth) {
    Timer t{__FUNCTION__};
    if(gcp.side() == GCode::On) return;

    const auto& bodies = pocketBodies(depth);

    setMax((bodies.size() + 1) * tools.size());
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
        file_        = nullptr;
        toolDiameter = tool.getDiameter(depth);
        dOffset      = toolDiameter * 0.5;
        stepOver     = tool.stepover();

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

        Geo::Polylines fill;
        for(const Geo::Polygon& body: bodies) {
            // Последней фрезе мелочь оставляем: дочищать после неё уже некому.
            fill.append_range(fillBody(body, forbidden, tIdx + 1 == size ? dOffset * 0.5 : dOffset * 2.0));
            incCurrent();
        }

        // Здесь заметённая заливка нужна ПО СУЩЕСТВУ, а не для картинки: из
        // неё складывается cleared, а из него -- forbidden следующей фрезы.
        // Кроме последней: после неё дочищать уже некому, и считать её
        // заливку незачем -- нарисуется обводкой, как в createStdFull.
        const bool lastTool = tIdx + 1 == size;
        if(!fill.empty()) {
            if(!lastTool) cutArea |= Geo::Inflate(fill, toolDiameter);
            returnPs.append_range(std::move(fill));
        }

        if(!returnPs.empty()) {
            stacking(returnPs);
            if(!returnPss.empty()) {
                if(!lastTool) cleared |= cutArea;
                // Параметры КОПИРУЮТСЯ, а не переносятся: прежний код делал
                // File{std::move(gcp)} прямо в цикле, и второму инструменту
                // доставался уже выпотрошенный gcp -- ни инструментов, ни
                // исходной геометрии.
                GCode::Params toolGcp                         = gcp;
                toolGcp.params[GCode::Params::MultiToolIndex] = static_cast<ssize_t>(tIdx);
                toolGcp.toolPathss                            = std::move(returnPss);
                // Показывать заливку и здесь незачем: рисуется обводкой.
                toolGcp.setPocketAreaCurves({});
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
