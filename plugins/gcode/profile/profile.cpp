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
#include "profile.h"
#include "app.h"
#include "gc_pathutils.h"
#include "gi_gcpath.h"
#include "gi_point.h"
#include "profile_jsapi.h"
#include "project.h"
#include "settings.h"

#include "geo/boolean.h"
#include "geo/util.h"

#include <map>
#include <optional>
#include <span>

namespace Profile {

namespace {

// Жил в myclipper.h вместе с целочисленным клиппером; здесь он нужен сам по
// себе -- половина стороны квадрата, вписанного в пятно фрезы.
constexpr double sqrt1_2 = sqrt2 * 0.5;

// Контур отдельной группой: одна врезка -- один проход. Через emplace_back, а
// не список инициализации: тот всегда копирует.
constexpr auto intoOwnGroup = [](Geo::Polyline&& path) {
    Geo::Polylines group;
    group.emplace_back(std::move(path));
    return group;
};

// Укорачивает полилинию с НАЧАЛА на `length` вдоль пути. Длина считается по
// дуге, а не по хорде, поэтому на скруглении обрезка не промахивается.
// Возвращает false, если укорачивать оказалось нечего -- путь короче.
bool trimFront(Geo::Polyline& path, double length) {
    if(path.size() < 2) return false;
    if(length <= 0.0) return true;

    // Сколько вершин уходит целиком и сколько длины остаётся добрать на
    // сегменте, где лимит исчерпался.
    std::size_t drop{};
    double rest = length;
    for(; drop + 1 < path.size(); ++drop) {
        const double len = Geo::segmentLength(path[drop], path[drop + 1]);
        if(rest < len) break;
        rest -= len;
    }

    if(drop + 1 >= path.size()) return false; // съели весь путь

    const Geo::Vertex from = path[drop];
    const Geo::Vertex to = path[drop + 1];
    const double len = Geo::segmentLength(from, to);
    const double t = len > 0.0 ? rest / len : 0.0;

    Geo::Vertex cut{from};
    if(auto arc = Geo::arcOf(from, to, from.bulge)) {
        // У оставшегося куска дуги свой прогиб: угол сократился в (1 - t) раз.
        static_cast<QPointF&>(cut) = arc->pointAt(t);
        cut.bulge = Geo::bulgeOf(arc->theta * (1.0 - t));
    } else {
        static_cast<QPointF&>(cut) = from + (to - from) * t;
        cut.bulge = 0.0;
    }

    path.erase(path.begin(), path.begin() + drop);
    path.front() = cut;
    return path.size() >= 2;
}

#if 0 // TODO касательный подвод/отвод чистового прохода
// Отложено. Геометрия отлажена и проверена численно (см. ниже), но на реальной
// плате остаются вырожденные перемычки: там зазор между черновым и целевым
// контуром уже припуска, параллельного соответствия между ними нет вовсе, и
// дуга подвода начинается в материале. Прежде чем включать -- нужна проверка
// кандидата и пропуск таких контуров.
//
// Что уже подтверждено замерами (scratchpad/side.cpp, lead.cpp):
//   * сторона: канонически Outer -- вправо по ходу, Inner -- влево, одинаково
//     для тел и для дырок; с учётом разворота в orderContours это ровно
//     left == gcp.convent();
//   * касательность дуги в точке входа и длина её хорды (== припуск) точны до
//     1e-16, перебег == pi*A/2 точно;
//   * врезаться надо в СЕРЕДИНЕ сегмента (rotateByLength): в вершине с изломом
//     нормали нет, и дуга уходит не туда.

// Направление движения в вершине: у дугового сегмента хорда врёт, его задаёт
// касательная. `atEnd` -- касательная в конце сегмента, иначе в начале.
QPointF tangentOf(const Geo::Vertex& from, const Geo::Vertex& to, bool atEnd) {
    QPointF dir;
    if(auto arc = Geo::arcOf(from, to, from.bulge)) {
        const double s = arc->theta > 0.0 ? 1.0 : -1.0;
        const double a = atEnd ? arc->endAngle() : arc->startAngle;
        dir = {-std::sin(a) * s, std::cos(a) * s};
    } else
        dir = to - from;

    const double len = std::hypot(dir.x(), dir.y());
    return len > 0.0 ? dir / len : QPointF{};
}

// Единичная нормаль к ходу: влево при left, вправо иначе. Влево -- это поворот
// касательной на +90 в сырых координатах.
QPointF normalOf(QPointF tangent, bool left) {
    return left ? QPointF{-tangent.y(), tangent.x()} : QPointF{tangent.y(), -tangent.x()};
}

// Замкнутый контур, начинающийся в точке на расстоянии `length` вдоль него от
// прежнего начала.
//
// Нужен, чтобы врезаться в СЕРЕДИНЕ сегмента. В вершине контур может иметь
// излом, а у излома нормали нет: параллельная кривая там не параллельна, у
// выпуклого угла она скругляется, у вогнутого срезается. Дуга подвода,
// построенная по нормали одного из сегментов, в обоих случаях начинается не на
// черновом контуре, а в материале.
Geo::Polyline rotateByLength(const Geo::Polyline& contour, double length) {
    const std::size_t count = contour.size();
    if(count < 2 || length <= 0.0) return contour;

    double rest = length;
    for(std::size_t i{}; i < count; ++i) {
        const Geo::Vertex& from = contour[i];
        const Geo::Vertex& to   = contour[(i + 1) % count];

        const double len = Geo::segmentLength(from, to);
        if(rest >= len) {
            rest -= len;
            continue;
        }

        // Точка реза внутри сегмента i: он разваливается на хвост (from -> cut)
        // и голову (cut -> to), между которыми и проходит весь остальной контур.
        const double t = len > 0.0 ? rest / len : 0.0;
        Geo::Vertex cut{from};
        double tailBulge{};
        if(auto arc = Geo::arcOf(from, to, from.bulge)) {
            static_cast<QPointF&>(cut) = arc->pointAt(t);
            cut.bulge                  = Geo::bulgeOf(arc->theta * (1.0 - t));
            tailBulge                  = Geo::bulgeOf(arc->theta * t);
        } else
            static_cast<QPointF&>(cut) = from + (to - from) * t;

        Geo::Polyline out;
        out.reserve(count + 1);
        out.closed = true;
        out.width  = contour.width;

        out.push_back(cut);
        for(std::size_t k = i + 1; k < count; ++k) out.push_back(contour[k]);
        for(std::size_t k = 0; k <= i; ++k) out.push_back(contour[k]);
        out.back().bulge = tailBulge; // замыкающий сегмент from -> cut

        return out;
    }

    return contour;
}

// Первые `length` длины ЗАМКНУТОГО контура, начиная с его первой вершины.
// Зеркало trimFront: тот отрезает начало, этот его и берёт.
Geo::Polyline headOf(const Geo::Polyline& contour, double length) {
    Geo::Polyline head;
    if(contour.size() < 2 || length <= 0.0) return head;

    double rest = length;
    for(auto&& [from, to]: Geo::segments(contour)) {
        const double len = Geo::segmentLength(from, to);
        if(rest >= len) { // сегмент целиком, вместе с его прогибом
            head.push_back(from);
            rest -= len;
            continue;
        }
        // Лимит исчерпался внутри сегмента -- режем его.
        const double t = len > 0.0 ? rest / len : 0.0;
        if(auto arc = Geo::arcOf(from, to, from.bulge)) {
            head.emplace_back(static_cast<const QPointF&>(from), Geo::bulgeOf(arc->theta * t));
            head.emplace_back(arc->pointAt(t));
        } else {
            head.push_back(from);
            head.emplace_back(from + (to - from) * t);
        }
        return head;
    }

    // Длины контура не хватило -- отдаём весь виток, замкнув его явной вершиной.
    head.emplace_back(static_cast<const QPointF&>(contour.front()));
    return head;
}
#endif

} // namespace

void Creator::create() {
    createProfile(gcp.tools.front(), gcp.params[GCode::Params::Depth].toDouble());
}

void Creator::createProfile(const Tool& tool, const double depth) {
    // Сигнал о готовности испускает сам Creator::createGc -- прежний Finaly
    // здесь давал вторую отправку того же файла.
    toolDiameter = tool.getDiameter(depth);

    const double allowance = gcp.params.contains(Allowance) ? gcp.params[Allowance].toDouble() : 0.0;
    // Припуску есть где жить только там, где вообще делается офсет, и только
    // пока чистовой снимает не больше радиуса фрезы. Ноль -- ничего не делаем:
    // всё ниже вырождается в прежнее поведение.
    const bool finishing = allowance > 0.0
        && gcp.side() != GCode::On
        && allowance <= toolDiameter * 0.5;

    if(gcp.side() == GCode::On) {
        if(gcp.params[TrimmingOpenPaths].toBool())
            trimmingOpenPaths(openSrcPaths);
        returnPs = closedSrc.contours();
    } else {
        // delta у Geo::Inflate -- ПОЛНАЯ ширина, контур уезжает на её
        // половину, то есть ровно на радиус фрезы. Прежний Inflate64 смещал
        // контур НА delta, и с закомментированным `* 0.5` профиль уходил на
        // целый диаметр -- вдвое дальше нужного.
        //
        // С припуском черновой уходит ещё на allowance: его кромка встаёт не на
        // целевой контур, а на allowance дальше от детали, оставляя этот слой
        // чистовому проходу.
        const double sign = gcp.side() == GCode::Outer ? +1.0 : -1.0;
        const double delta = sign * (toolDiameter + 2.0 * (finishing ? allowance : 0.0));

        // Разбор на тела с отверстиями (groupedPaths) больше не нужен:
        // раздувается сразу весь регион, вложенность у него своя.
        if(!closedSrc.empty())
            returnPs = Geo::Inflate(closedSrc, delta).contours();

        // У линии площади нет: снаружи она обрастает штрихом шириной в
        // диаметр, а внутрь сжимать нечего -- см. geo/boolean.h. Припуск к ней
        // не применяется: у разомкнутого пути нет стороны, с которой заходить.
        if(openSrcPaths.size() && gcp.side() == GCode::Outer)
            returnPs.append_range(Geo::Inflate(openSrcPaths, toolDiameter).contours());
    }

    if(returnPs.empty() && openSrcPaths.empty()) return;

    reorder();

    if(finishing && !closedSrc.empty())
        makeFinishing();

    if(gcp.side() == GCode::On && openSrcPaths.size()) {
        GCode::mergePolylines(openSrcPaths, App::project().glue());
        GCode::sortByProximity(openSrcPaths, App::home().pos() + App::zero().pos());
        // Каждый открытый путь -- своя группа: одна врезка, один проход.
        returnPss.append_range(openSrcPaths | v::as_rvalue | v::transform(intoOwnGroup));
    }

    // Мостики траекторию здесь НЕ трогают: контур остаётся целым, а над
    // мостами фреза приподнимается горбом уже при выводе УП -- см.
    // File::saveMillingProfileBridges. Форма кладёт центры мостов в
    // gcp.supportCurvess, длину и высоту -- в params.

    if(gcp.params.contains(TrimmingCorners) && gcp.params[TrimmingCorners].toInt())
        cornerTrimming();

    if(returnPss.empty()) return;

    gcp.toolPathss = std::move(returnPss);

    file_ = new File{std::move(gcp)};
    file_->setFileName(tool.nameEnc());
}

void Creator::trimmingOpenPaths(Geo::Polylines& paths) {
    // Прежде то же самое делалось морфологически: путь раздувался в капсулу
    // (Miter/Butt), сжимался обратно и обрезал сам себя как открытый субъект.
    // Ни таких стыков, ни обрезки открытого пути в Geo нет, да и незачем --
    // трюк сводился к «укоротить на радиус с каждого конца».
    const double radius = toolDiameter * 0.5;

    auto trimBothEnds = [radius, this](Geo::Polyline& path) {
        if(path.perimeter() <= toolDiameter) return false; // после обрезки не осталось бы ничего
        if(!trimFront(path, radius)) return false;
        path.reverse();
        const bool ok = trimFront(path, radius);
        path.reverse();
        return ok;
    };

    // Обрезка и выбрасывание -- разными проходами: предикату erase_if менять
    // элементы не положено, он вызывается через remove_if.
    for(Geo::Polyline& path: paths)
        if(!trimBothEnds(path)) path.clear();
    std::erase_if(paths, [](const Geo::Polyline& path) { return path.empty(); });
}

void Creator::cornerTrimming() {
    const double trimDepth = (toolDiameter - toolDiameter * sqrt1_2) * sqrt1_2;
    const double sqareSide = toolDiameter * sqrt1_2 * 0.5;

    // Угол ВНУТРЕННЕГО стыка в том направлении обхода, которое выставил
    // orderContours, то есть в согласии с Params::reversedTravel. Перевернёшь
    // разворот там -- углы здесь станут дополнительными, и подрезка перестанет
    // находить хоть что-то, МОЛЧА. Знак нырка идёт за testAngle, а не за
    // convent сам по себе: 90 -- поворот налево, кость ныряет направо.
    const double testAngle = gcp.convent() ? 90.0 : 270.0;
    const double trimAngle = gcp.convent() ? -45.0 : +45.0;

    // Вершина нырка или nullopt, если угол не тот или кость в него не влезает.
    auto dogBone = [=](const Geo::Vertex& prev, const Geo::Vertex& corner,
                       const Geo::Vertex& next) -> std::optional<QPointF> {
        // Стык с дугой пропускается: QLineF по вершинам даёт угол ХОРДЫ, а не
        // касательной, и нырок ушёл бы не туда. Прежде дуг здесь не было вовсе
        // -- всё приходило ломаными из целочисленного клиппера.
        if(prev.isArc() || corner.isArc()) return {};

        QLineF l1{prev, corner};
        QLineF l2{corner, next};
        if(std::abs(l1.angleTo(l2) - testAngle) > 1.e-3) return {};       // угол не прямой
        if(l1.length() < sqareSide || l2.length() < sqareSide) return {}; // кость не влезает

        l2.setAngle(l1.angle() + trimAngle);
        l2.setLength(trimDepth);
        return l2.p2();
    };

    // Прежний обход шёл сырыми указателями по path.data() и вставлял вершины в
    // середину, держась на заранее выданном reserve. Здесь путь собирается
    // заново -- то же самое, но без зависимости от перевыделения.
    for(Geo::Polyline& path: returnPss | v::join) {
        const std::size_t count = path.size();
        if(count < 3) continue;

        Geo::Polyline out;
        out.closed = path.closed;
        out.width = path.width;
        out.reserve(count * 3);

        // У разомкнутого пути крайние вершины углами не бывают: соседа с
        // одной стороны просто нет.
        const std::size_t first = path.closed ? 0 : 1;
        const std::size_t last = path.closed ? count : count - 1;

        // Индексами, а не рэнджами: соседей у замкнутого пути надо брать по
        // кругу, и v::pairwise/adjacent последнюю пару (back -> front) не дают.
        for(std::size_t i{}; i < count; ++i) {
            const Geo::Vertex& corner = path[i];
            std::optional<QPointF> trim;
            if(first <= i && i < last)
                trim = dogBone(path[(i + count - 1) % count], corner, path[(i + 1) % count]);
            if(trim) // дойти до угла, нырнуть и вернуться (прогиб у них нулевой)
                out.append_range(std::array{corner, Geo::Vertex{*trim}});
            out.push_back(corner); // прогиб исходящего сегмента остаётся на ней
        }

        path = std::move(out);
    }
}

Geo::Polylines Creator::orderContours(Geo::Polylines contours) {
    if(contours.empty()) return contours;

    // Вложенность концентрических контуров -- то, ради чего прежде строился
    // PolyTree: объединение по EvenOdd вместе с синтетической рамкой вокруг
    // всего. Рамка была нужна лишь затем, чтобы у реальных контуров глубина
    // считалась от единицы; у nestingForest она 0-базовая, так что ни рамки,
    // ни отбрасывания её ветки (`nesting > 2`) больше нет.
    const GCode::NestingForest forest = GCode::nestingForest(contours);

    Geo::Polylines ordered;
    ordered.reserve(contours.size());

    if(!settings.sort) { // Grouping by nesting
        std::map<int, Geo::Polylines> byDepth;
        for(auto&& [i, path]: v::enumerate(contours))
            byDepth[forest.depth[i]].emplace_back(std::move(path));

        const QPointF start = App::home().pos() + App::zero().pos();
        for(auto& [depth, paths]: byDepth) {
            if(paths.size() > 1)
                GCode::sortByProximity(paths, start);
            ordered.append_range(paths | v::as_rvalue); // именно as_rvalue: иначе копия
        }
    } else { // Grouping by nesting depth
        QPointF from = App::settings().mkrZeroOffset();
        auto walk = [&](this auto&& walk, std::size_t idx) -> void {
            Geo::Polyline path = std::move(contours[idx]);
            // Врезаться дешевле там, где инструмент уже оказался.
            GCode::rotateToNearest(path, from);
            from = path.back();
            ordered.emplace_back(std::move(path));
            for(std::size_t child: forest.children[idx])
                walk(child);
        };
        for(std::size_t root: forest.roots)
            walk(root);
    }

    // Изнутри наружу: обход строился от объемлющих контуров к вложенным, а
    // снимать материал начинают с внутренних.
    r::reverse(ordered);

    if(gcp.reversedTravel())
        r::for_each(ordered, &Geo::Polyline::reverse);

    // Чистки StripDuplicates/SimplifyPath не стало: они убирали шум
    // целочисленного клиппера, которого у точного домена нет.
    r::for_each(ordered, &Geo::Polyline::close); // замкнутость -- ФЛАГ, а не повтор вершины
    return ordered;
}

void Creator::reorder() {
    returnPs = orderContours(std::move(returnPs));
    returnPss.append_range(returnPs | v::as_rvalue | v::transform(intoOwnGroup));
    returnPs.clear();
}

void Creator::makeFinishing() {
    // Целевой контур -- тот самый, что раньше был единственным: офсет ровно на
    // радиус фрезы. Порядок и направление те же, что у чернового, иначе проходы
    // не будут соответствовать друг другу.
    const double sign = gcp.side() == GCode::Outer ? +1.0 : -1.0;
    Geo::Polylines target = orderContours(Geo::Inflate(closedSrc, sign * toolDiameter).contours());
    if(target.empty()) return;

    // Пока чистовой -- обычный проход, ничем не отличающийся от чернового: своя
    // группа на контур, а значит спираль и финальная подчистка дна достаются ему
    // от saveMillingProfile даром. Дописывается ПОСЛЕ чернового, потому и
    // фрезеруется после него.
    //
    // Касательный подвод/отвод отложен -- см. #if 0 в начале файла.
    returnPss.append_range(target | v::as_rvalue | v::transform(intoOwnGroup));
}

File::File()
    : GCode::File() { }

File::File(GCode::Params&& newGcp)
    : GCode::File{std::move(newGcp)} {
    if(gcp.tools.front().diameter()) {
        regenerate();
    }
}

//------------------------------------------------------------------------------
// Мостики (табы): в отмеченных мостами местах деталь остаётся прихваченной к
// заготовке перемычкой высотой BridgeHeight от дна реза. Контур остаётся ОДНИМ
// непрерывным проходом: на проходах ниже верха таба фреза над мостом
// приподнимается до него и тут же спускается -- XY-подача не прерывается,
// вертикальных врезаний нет. Горб короткого моста -- треугольник, длинного --
// трапеция со скосами в диаметр фрезы (см. splitBridge). Сам порядок проходов
// пишет profile.js через BridgesApi; здесь -- то, что требует C++: круги реза
// (радиус зависит от припуска чистового прохода) и верх таба.

QObject* File::createJsExtension(GCode::GcFileProxy& proxy, QJSEngine& /*engine*/) {
    return new BridgesApi{this, &proxy};
}

bool File::hasBridges() const {
    return toolType != Tool::Laser
        && !gcp.supportCurvess.empty()
        && gcp.params.contains(Creator::BridgeLen)
        && gcp.params.contains(Creator::BridgeHeight);
}

Geo::Polygons File::bridgeRegion(const QPointF& offset) {
    if(!hasBridges()) return {};
    const double toolDiam = gcp.getToolDiameter();

    // Припуск, если чистовой проход вообще был, -- условие то же, что в
    // Creator::createProfile: черновая траектория дальше от контура ровно на
    // него.
    double allowance{};
    if(auto it = gcp.params.find(Creator::Allowance); it != gcp.params.end())
        if(const double value = it->second.toDouble();
            value > 0.0 && gcp.side() != GCode::On && value <= toolDiam * 0.5)
            allowance = value;

    // Радиус круга реза: хорда на самой ДАЛЬНЕЙ траектории (черновой) должна
    // быть len + toolDiam -- материала между следами фрезы остаётся ровно len.
    // На чистовой, что ближе к контуру, хорда выходит чуть шире: перемычка с
    // запасом, а не срезанная.
    const double len = gcp.params.at(Creator::BridgeLen).toDouble();
    const double dist = gcp.side() == GCode::On ? 0.0 : toolDiam * 0.5 + allowance;
    const double radius = std::hypot((len + toolDiam) * 0.5, dist);

    // Круги реза -- одним объединением: соседние мосты могут пересекаться.
    // Зеркало нижней стороны и раскладка применяются к центрам той же
    // машинерией, что и к траекториям.
    Geo::Polylines centers;
    for(const Geo::Polylines& group: gcp.supportCurvess)
        centers.append_range(group);
    std::vector<Geo::Polygon> discs;
    for(const Geo::Polyline& pts: mirrorAndOffsetCurves(offset, std::move(centers)))
        for(const Geo::Vertex& pt: pts)
            discs.emplace_back(Geo::circle(radius * 2.0, pt));
    return Geo::Polygons{std::span<const Geo::Polygon>{discs}};
}

double File::bridgeTabTop() {
    if(!hasBridges()) return 0.0;
    // Верх таба отсчитывается от дна реза. К поверхности не прижимается
    // вплотную: ровно ноль для savePath -- «глубины нет», рампа не построится.
    const std::vector<double> depths = getDepths();
    return std::min(
        depths.back() + gcp.params.at(Creator::BridgeHeight).toDouble(),
        -Geo::exitWeldTolerance);
}

void File::createGi() {
    Gi::Item* item;

    // Полоса снятого материала: тот же путь, но пером в диаметр фрезы.
    for(const Geo::Polylines& paths: gcp.toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPen(QPen{Qt::black, gcp.getToolDiameter(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin});
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::CutArea));
        itemGroup()->push_back(item);
    }

    // Поверх неё -- сама траектория, волосяной линией.
    for(const Geo::Polylines& paths: gcp.toolPathss) {
        item = new Gi::GcPath{paths, this};
        item->setPenColorPtr(&App::settings().guiColor(GuiColors::ToolPath));
        itemGroup()->push_back(item);
    }

    // Переезды: от конца одного прохода к началу следующего.
    for(auto&& [from, to]: gcp.toolPathss | v::join | v::pairwise)
        g0path_.emplace_back(QPolygonF{from.back(), to.front()});

    item = new Gi::GcPath{g0path_, this};
    item->setPenColorPtr(&App::settings().guiColor(GuiColors::G0));
    itemGroup()->push_back(item);

    itemGroup()->setVisible(true);
}

} // namespace Profile
