#include "geo/inflatepasses.h"

#include "cgal.h"
#include "geo/boolean.h"
#include "geo/cancel.h"
#include "phasestats.h"

#include <QRectF>

#include <algorithm>
#include <atomic>
#include <exception>
#include <optional>
#include <span>
#include <stop_token>
#include <thread>
#include <utility>

namespace Geo {

namespace {

// Источник от стольких вершин -- крупный: считается с внутренним
// параллелизмом Inflate, в пуле на bigPool потоков.
constexpr std::size_t bigSource = 64;
constexpr std::size_t bigPool = 6;

std::size_t verticesOf(const Polylines& contours) {
    std::size_t total{};
    for(const Polyline& contour: contours) total += contour.size();
    return total;
}

} // namespace

struct InflatePasses::Source {
    Polygons base;         // текущая база офсета; принадлежит своему потоку
    double baseDistance{}; // полуширина, на которой база принята
    std::size_t baseVertices{};
    double bias{};
    std::size_t group{InflatePasses::global}; // объемлющее, из которого вычитается тело
    bool grow{};   // тело раздувается; объемлющее усаживается
    bool dead{};   // объемлющее съедено усадкой -- витков больше не даст
    bool shared{}; // база -- точная копия тела с ОБЩИМИ числами: только
                   // последовательный счёт в вызывающем потоке
    QRectF lastBox;  // габарит последнего результата -- для смерти по габариту

    // Виток источника: офсет базы на остаток до полной полуширины плюс
    // принятие новой базы, когда материализованный результат проще. Пишет
    // только в СЕБЯ и в свой слот -- контракт parallelFor.
    void offsetInto(double d, double coarse, std::vector<Polygon>& out) {
        const double eff = d + bias - baseDistance;
        // Ещё не пора (bias сдвинул расписание вперёд) -- источник как есть.
        Polygons result = eff <= 0.0
            ? base
            : Inflate(base, (grow ? 2.0 : -2.0) * eff, coarse);
        if(result.empty()) {
            if(!grow) dead = true; // раздутое пустым не бывает
            return;
        }

        // Принятие базы -- как у монолитных витков (pocketoffset), только
        // пофигурно: материализованная петля после сварки и сшивки дуг
        // проще -- пересборка БЕЗ ПОТЕРИ контуров становится новой базой, и
        // дальше витки идут от неё. Потерянный контур (вырождение при
        // округлении) -- переход не состоялся, база прежняя.
        const Polylines contours = result.contours();
        const std::size_t vertices = verticesOf(contours);
        if(!baseVertices) baseVertices = verticesOf(base.contours());
        if(vertices < baseVertices) {
            PhaseScope stat{Phase::Adopt};
            Polygons candidate{contours};
            if(candidate.contours().size() == contours.size()) {
                base = std::move(candidate);
                baseDistance = d + bias;
                baseVertices = vertices;
                shared = false; // пересборка -- уже свои числа
            }
        }

        out = result.all();
    }
};

InflatePasses::InflatePasses() = default;
InflatePasses::~InflatePasses() = default;
InflatePasses::InflatePasses(InflatePasses&&) noexcept = default;
InflatePasses& InflatePasses::operator=(InflatePasses&&) noexcept = default;

std::size_t InflatePasses::addField(const Polygon& body) {
    if(body.empty() || body.isInverted()) return global;
    const std::size_t group = addAmbient(body.outer());
    // Дырки тела приходят обратным обходом -- как самостоятельные тела их
    // надо развернуть.
    for(Polyline hole: body.holes()) {
        hole.reverse();
        addSolid(hole, group);
    }
    return group;
}

std::size_t InflatePasses::addAmbient(const Polyline& contour) {
    Polygons base{Polylines{contour}};
    // Пустой объемлющий (вырожденный контур) всё равно занимает номер
    // группы: его тела лягут в пустоту, как и должны.
    const bool dead = base.empty();
    ambients_.push_back({.base = std::move(base), .dead = dead});
    return ambients_.size() - 1;
}

void InflatePasses::addSolid(const Polygon& body, std::size_t group, double bias) {
    if(body.empty() || body.isInverted()) return;
    // База пересобирается из bulge-вида: свежие точные числа, никаких общих
    // ленивых кэшей с регионом вызывающего -- потокобезопасность по
    // построению. Пересборка потеряла контур -- база остаётся точной
    // копией тела, но с общими числами: такой источник считается
    // последовательно (shared).
    const Polylines contours = body.contours();
    Source source{.bias = bias, .group = group, .grow = true};
    Polygons rebuilt{contours};
    if(rebuilt.contours().size() == contours.size())
        source.base = std::move(rebuilt);
    else {
        source.base = Polygons{body};
        source.shared = true;
    }
    solids_.push_back(std::move(source));
}

void InflatePasses::addSolid(const Polyline& contour, std::size_t group, double bias) {
    Polygons base{Polylines{contour}};
    if(base.empty()) return;
    solids_.push_back({.base = std::move(base), .bias = bias, .group = group, .grow = true});
}

void InflatePasses::addSolids(const Polygons& region, std::size_t group, double bias) {
    for(const Polygon& body: region.all()) addSolid(body, group, bias);
}

Polygons InflatePasses::pass(double d, double coarse) {
    return Polygons::fromSeparated(passParts(d, coarse));
}

std::vector<Polygons> InflatePasses::passParts(double d, double coarse) {
    checkCancelled();

    // Раскладка: параллельные и последовательные (shared) источники --
    // отдельно, слот результата у каждого свой.
    std::vector<Source*> tasks, serialTasks;
    for(Source& source: ambients_)
        if(!source.dead) (source.shared ? serialTasks : tasks).push_back(&source);
    const std::size_t ambientTasks = tasks.size() + serialTasks.size();
    for(Source& source: solids_) {
        // Тело съеденной группы считать незачем: вычитать его не из чего.
        if(source.dead || (source.group != global && ambients_[source.group].dead)) continue;
        if(!source.baseVertices) source.baseVertices = verticesOf(source.base.contours());
        (source.shared ? serialTasks : tasks).push_back(&source);
    }

    // Все объемлющие съедены -- область кончилась; без объемлющих виток
    // законно состоит из одних раздутых тел.
    if(!ambients_.empty() && !ambientTasks) return {};

    std::vector<std::vector<Polygon>> taskSlots(tasks.size()), serialSlots(serialTasks.size());
    // Пофигурный офсет. Мелочь -- по источнику на поток общим parallelFor,
    // где вложенный параллелизм внутри Inflate (капсулы, joinAll) сам
    // складывается в последовательный счёт (см. insideParallelWorker в
    // cgal.cpp). Крупные источники (слившиеся кластеры) задают критический
    // путь витка и считаются в маленьком пуле БЕЗ стража вложенности: каждому
    // достаётся внутренний параллелизм Inflate, а одновременных крупных --
    // горстка, так что потоки не размножаются. Выигрыш от этого невелик
    // (наложение и материализация внутри Inflate последовательны), но и
    // не даром.
    std::vector<std::size_t> big, small;
    for(std::size_t i = 0; i < tasks.size(); ++i)
        (tasks[i]->baseVertices >= bigSource ? big : small).push_back(i);
    if(!big.empty()) {
        const std::size_t spawn = std::min<std::size_t>(bigPool, big.size());
        std::atomic<std::size_t> next{0};
        std::vector<std::exception_ptr> failures(spawn);
        const std::stop_token token = cancelToken();
        {
            std::vector<std::jthread> pool;
            pool.reserve(spawn);
            for(std::size_t w = 0; w < spawn; ++w)
                pool.emplace_back([&, w] {
                    CancelScope scope{token};
                    for(std::size_t i = next++; i < big.size(); i = next++) try {
                            checkCancelled();
                            tasks[big[i]]->offsetInto(d, coarse, taskSlots[big[i]]);
                        } catch(...) {
                            failures[w] = std::current_exception();
                            return;
                        }
                });
        }
        for(const std::exception_ptr& failure: failures)
            if(failure) std::rethrow_exception(failure);
    }
    Cgal::parallelFor(small.size(), [&](std::size_t j) {
        tasks[small[j]]->offsetInto(d, coarse, taskSlots[small[j]]);
    });
    for(std::size_t i = 0; i < serialTasks.size(); ++i) {
        checkCancelled();
        serialTasks[i]->offsetInto(d, coarse, serialSlots[i]);
    }

    // Сборка витка. Куски раскладываются по группам: усаженное объемлющее
    // группы минус объединение её раздутых тел; из каждой группы вычитаются
    // ещё и глобальные тела. Каждое объединение -- параллельное дерево
    // joinAll внутри конструктора. Глобальные тела -- группа с номером
    // ambients_.size().
    const std::size_t groups = ambients_.size() + 1;
    auto keyOf = [&](const Source& source) {
        return source.group == global ? ambients_.size() : source.group;
    };
    std::vector<std::vector<Polygon>> shrunk(ambients_.size()), grown(groups);
    auto collect = [&](Source* source, std::vector<Polygon>& slot) {
        source->lastBox = {};
        for(const Polygon& part: slot) source->lastBox |= part.boundingRect();
        auto& target = source->grow ? grown[keyOf(*source)] : shrunk[source - ambients_.data()];
        target.insert(target.end(), std::make_move_iterator(slot.begin()),
            std::make_move_iterator(slot.end()));
    };
    for(std::size_t i = 0; i < tasks.size(); ++i) collect(tasks[i], taskSlots[i]);
    for(std::size_t i = 0; i < serialTasks.size(); ++i) collect(serialTasks[i], serialSlots[i]);

    // СХЛОПЫВАНИЕ группы: пока тел много, виток выигрывает на параллельном
    // пофигурном офсете и слиянии кластеров; когда их осталась горстка,
    // дешевле само поле группы -- оно уже посчитано и проще, чем объемлющее
    // со всеми кляксами вместе (клякса несёт и наружную сторону, полю не
    // нужную). Поле становится новым объемлющим группы, тела выбрасываются,
    // и дальше группа усаживается как монолит от простой базы -- с тем же
    // принятием упростившихся витков. Пересборка обязана сохранить контуры,
    // иначе -- в следующий раз.
    std::vector<std::size_t> liveSolids(groups, 0);
    for(const Source& source: solids_)
        if(!source.dead) ++liveSolids[keyOf(source)];
    std::vector<char> collapsed(groups, 0);
    auto collapseGroup = [&](std::size_t k, const Polygons& body, double at) {
        if(liveSolids[k] == 0 || liveSolids[k] > bigPool) return;
        const Polylines contours = body.contours();
        Polygons rebuilt{contours};
        if(rebuilt.contours().size() != contours.size()) return;
        Source& ambient = ambients_[k];
        ambient.base = std::move(rebuilt);
        ambient.baseDistance = at;
        ambient.baseVertices = verticesOf(contours);
        ambient.shared = false;
        collapsed[k] = 1;
    };

    std::vector<std::optional<Polygons>> grownUnion(groups);
    for(std::size_t k = 0; k < groups; ++k)
        if(!grown[k].empty()) {
            checkCancelled();
            grownUnion[k].emplace(std::span<const Polygon>{grown[k]});
        }

    // Усаженные объемлющие по группам -- они же границы отсечения клякс при
    // кластерном принятии (см. ниже); для глобальных тел -- всё поле разом.
    std::vector<std::optional<Polygons>> ambientRegion(ambients_.size());
    // Группы -- компоненты одной области, усаженные порознь: они разнесены
    // и отдаются порознь, без объединения (вызывающему чаще всего нужны
    // лишь контуры; целый регион -- Polygons::fromSeparated в pass()).
    std::vector<Polygons> field;
    if(ambients_.empty()) {
        if(grownUnion.back()) field.push_back(std::move(*grownUnion.back()));
    } else {
        for(std::size_t k = 0; k < ambients_.size(); ++k) {
            if(shrunk[k].empty()) continue;
            checkCancelled();
            ambientRegion[k].emplace(std::span<const Polygon>{shrunk[k]});
            Polygons body = *ambientRegion[k];
            if(grownUnion[k]) body -= *grownUnion[k];
            if(body.empty()) continue;
            collapseGroup(k, body, d);
            field.push_back(std::move(body));
        }
        if(!field.empty() && grownUnion.back()) {
            // Глобальные тела: отсекаются по всему полю и вычитаются из
            // каждой его компоненты.
            Polygons whole = Polygons::fromSeparated(field);
            ambientRegion.emplace_back(std::move(whole));
            for(Polygons& part: field) part -= *grownUnion.back();
            std::erase_if(field, [](const Polygons& part) { return part.empty(); });
        }
    }

    // СМЕРТЬ ПО ГАБАРИТУ -- тот же довод, что у отсечения ниже: тело, чья
    // клякса сейчас не достаёт до усаженного объемлющего своей группы, не
    // достанет до него и впредь. Проверка по габаритам -- грубая, но
    // дешёвая; точное отсечение догоняет при слиянии кластеров.
    for(Source& source: solids_) {
        const std::size_t k = keyOf(source);
        if(source.dead || k >= ambientRegion.size() || !ambientRegion[k]) continue;
        if(!source.lastBox.intersects(ambientRegion[k]->boundingRect())) source.dead = true;
    }

    // КЛАСТЕРНОЕ принятие: раздутые тела группы сливаются в кляксы, и считать
    // дальше полтысячи пятачков по отдельности -- выброшенная работа: с
    // какого-то витка объединение группы проще суммы её источников. Тогда
    // оно само становится её источниками -- по компоненте, чтобы дальше
    // раздуваться параллельно. Та же сделка, что и у одиночной базы:
    // пересборка из bulge-вида без потери контуров, иначе переход не
    // состоялся.
    std::vector<Source> merged;
    std::vector<char> replaced(groups, 0);
    for(std::size_t k = 0; k < groups; ++k) {
        if(!grownUnion[k] || collapsed[k]) continue;
        std::size_t live = 0, vertices = 0;
        for(Source& source: solids_) {
            if(keyOf(source) != k || source.dead) continue;
            ++live;
            vertices += source.baseVertices;
        }
        if(live < 2) continue;
        // ОТСЕЧЕНИЕ: кляксу стоит держать только внутри усаженного объемлющего
        // её группы. Объемлющее усаживается ровно с той скоростью, с какой
        // клякса растёт, поэтому точка кляксы, лежащая сейчас вне усаженного
        // объемлющего, ни на один следующий виток повлиять не может -- а её
        // граница (вся наружная сторона меди, обращённая прочь от поля)
        // стоила бы капсул на каждом витке.
        // Сперва дешёвые проверки: слияний не было вовсе (тел в объединении
        // столько же, сколько источников) -- и считать нечего; иначе -- по
        // вершинам самого объединения. Отсечение может только упростить, но
        // само стоит наложения: его незачем платить на витке без слияний.
        if(grownUnion[k]->size() >= live) continue;
        if(verticesOf(grownUnion[k]->contours()) >= vertices) continue;
        Polygons whole = *grownUnion[k];
        if(k < ambientRegion.size() && ambientRegion[k]) whole &= *ambientRegion[k];
        if(whole.empty()) {
            // Клякса целиком снаружи -- её телам больше нечего вычитать.
            replaced[k] = 1;
            continue;
        }

        std::vector<Source> next;
        bool intact = true;
        for(const Polygon& component: whole.all()) {
            const Polylines contours = component.contours();
            Polygons rebuilt{contours};
            if(rebuilt.contours().size() != contours.size()) {
                intact = false;
                break;
            }
            // Снимок объединения на витке d: дальше он раздувается на d' - d,
            // и расписание каждого члена (d' + bias = d + bias + (d' - d))
            // в нём уже учтено -- единого bias группе не нужно.
            next.push_back({.base = std::move(rebuilt), .baseDistance = d,
                .baseVertices = verticesOf(contours),
                .group = k == ambients_.size() ? global : k, .grow = true});
        }
        if(!intact) continue;
        replaced[k] = 1;
        merged.insert(merged.end(), std::make_move_iterator(next.begin()),
            std::make_move_iterator(next.end()));
    }
    if(!merged.empty()) {
        std::vector<Source> kept;
        for(Source& source: solids_)
            if(!replaced[keyOf(source)] && !source.dead) kept.push_back(std::move(source));
        kept.insert(kept.end(), std::make_move_iterator(merged.begin()),
            std::make_move_iterator(merged.end()));
        solids_ = std::move(kept);
    }
    // Мёртвые тела, тела съеденных и схлопнутых групп больше не нужны никому.
    std::erase_if(solids_, [&](const Source& source) {
        return source.dead || collapsed[keyOf(source)]
            || (source.group != global && ambients_[source.group].dead);
    });

    return field;
}

} // namespace Geo
