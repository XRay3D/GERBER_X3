#include "cgal.h"
#include "geo/cancel.h"
#include "offsetcapsules.h"

#include <QDebug>
#include <QPainterPath>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <exception>
#include <numbers>
#include <thread>
#include <variant>
#include <vector>

namespace Geo::Cgal {

namespace {

// Точка bulge-дуги на параметре t (0, 1) -- третья точка для построения дуги
// через три точки: окружность через три рациональные точки сама рациональна,
// и кривая ТОЧНО проходит через свои концы (CGAL требует, чтобы концы лежали
// на опорной окружности ровно). Цена -- отклонение самой окружности от
// идеальной в пределах double-эпсилона; дальше всё точно.
//
// t параметризуем не просто из общности: изредка (см. toGPoly) сама тройка
// точек по СЕРЕДИНЕ дуги (t=0.5) заставляет конструктор CGAL собрать
// большую дугу вместо малой -- независимо подтверждённый эффект, причина
// внутри CGAL не найдена. Другое t -- другая тройка точек и другая опорная
// окружность (все верны с точностью до double-эпсилона), и на практике
// достаточно взять НЕ середину, чтобы обойти эту конкретную неудачу.
QPointF arcPointAt(QPointF from, QPointF to, double bulge, double t) {
    const double dx      = to.x() - from.x();
    const double dy      = to.y() - from.y();
    const double chord   = std::hypot(dx, dy);
    const double theta   = 4.0 * std::atan(bulge);
    const double sagitta = bulge * chord / 2.0;
    const double radius  = chord / (2.0 * std::abs(std::sin(theta / 2.0)));
    const QPointF n(dy / chord, -dx / chord);
    const QPointF mid((from + to) / 2.0);
    const QPointF center = mid + n * (sagitta - std::copysign(radius, bulge));
    const double a0      = std::atan2(from.y() - center.y(), from.x() - center.x());
    const double am      = a0 + theta * t;
    return QPointF(center.x() + radius * std::cos(am), center.y() + radius * std::sin(am));
}

// Одна x-монотонная кривая точного контура, пересчитанная в double: концы,
// а у дуги ещё центр, радиус и размах СО ЗНАКОМ (против часовой стрелки --
// положительный). Всё, что о кривой нужно знать снаружи точного домена,
// собрано здесь -- дальше считается уже обычной арифметикой.
struct CurveGeometry {
    QPointF from, to, center;
    double radius = 0.0;
    double sweep  = 0.0;
    bool linear   = true;
};

CurveGeometry geometryOf(const XCurve& xc) {
    auto at = [](const auto& point) {
        return QPointF(CGAL::to_double(point.x()), CGAL::to_double(point.y()));
    };

    CurveGeometry out;
    out.from   = at(xc.source());
    out.to     = at(xc.target());
    out.linear = xc.is_linear();
    if(out.linear) return out;

    // x-монотонность гарантирует размах не больше полуокружности -- отсюда
    // и |bulge| <= 1 у потребителей DXF, и одна ветвь окружности в тесте на
    // принадлежность.
    const auto& circle = xc.supporting_circle();
    out.center         = at(circle.center());
    out.radius         = std::sqrt(CGAL::to_double(circle.squared_radius()));

    const double a0 = std::atan2(out.from.y() - out.center.y(), out.from.x() - out.center.x());
    const double a1 = std::atan2(out.to.y() - out.center.y(), out.to.x() - out.center.x());
    out.sweep       = std::remainder(a1 - a0, 2.0 * std::numbers::pi);
    if(xc.orientation() == CGAL::COUNTERCLOCKWISE && out.sweep < 0.0)
        out.sweep += 2.0 * std::numbers::pi;
    if(xc.orientation() == CGAL::CLOCKWISE && out.sweep > 0.0)
        out.sweep -= 2.0 * std::numbers::pi;
    return out;
}

// Пересекает ли кривая вертикальный луч, пущенный из точки вверх.
// x-монотонность и тут кстати: пересечение не больше одного, а
// полуоткрытый по x интервал не даёт посчитать общую вершину двух кривых
// дважды.
bool crossesRayUp(const CurveGeometry& curve, QPointF point) {
    const double left  = std::min(curve.from.x(), curve.to.x());
    const double right = std::max(curve.from.x(), curve.to.x());
    if(point.x() < left || point.x() >= right) return false;

    if(curve.linear) {
        const double t = (point.x() - curve.from.x()) / (curve.to.x() - curve.from.x());
        return curve.from.y() + t * (curve.to.y() - curve.from.y()) > point.y();
    }

    // Какая из двух ветвей окружности -- видно по ходу: против часовой
    // стрелки вправо идёт только нижняя половина, влево -- только верхняя
    // (по часовой всё наоборот).
    const double dx     = point.x() - curve.center.x();
    const double height = std::sqrt(std::max(0.0, curve.radius * curve.radius - dx * dx));
    const bool upper    = (curve.sweep > 0.0) == (curve.to.x() < curve.from.x());
    return curve.center.y() + (upper ? height : -height) > point.y();
}

// Сколько кусков дерева слияний приходится на поток. Кусков поровну по
// числу контуров, но не по цене -- она зависит от того, сколько их
// наложилось друг на друга, -- поэтому кусков берётся больше, чем ядер, и
// потоки разбирают их на лету.
constexpr std::size_t chunksPerWorker = 2;

// Мельче этого делить незачем: собственное разбиение CGAL справится с
// таким пучком само, а потоки на нём стоят дороже самой работы.
constexpr std::size_t minChunkSize = 16;

// Столько же элементов на поток -- нижняя граница, ниже которой parallelFor
// работает на месте: запуск потока стоит десятки микросекунд, и на горстке
// коротких контуров он их не отобьёт.
constexpr std::size_t minParallelBatch = 8;

unsigned workerCount() {
    const unsigned hardware = std::thread::hardware_concurrency();
    return hardware ? hardware : 1;
}

// Пространственный порядок кусков -- кривая Мортона (Z-порядок) по центру
// габарита. На результат он не влияет вовсе (объединение коммутативно), а
// на цене сказывается сильнее самих потоков: соседние в списке куски
// оказываются соседними и на плоскости, так что лист дерева получает
// связную кляксу вместо россыпи по всему чертежу, а слияние двух соседних
// листьев правит лишь узкую полосу на их стыке. На тестовом чертеже один
// только порядок снимает четверть времени даже в один поток.
//
// Кусок -- контур (GPoly) либо целое тело с дырками (GPolyWH); габарит у
// второго берётся по внешней границе, дырки его расширить не могут.
CGAL::Bbox_2 boxOf(const GPoly& part) { return part.bbox(); }
CGAL::Bbox_2 boxOf(const GPolyWH& part) {
    return part.is_unbounded() ? CGAL::Bbox_2{} : part.outer_boundary().bbox();
}

template <typename Part>
void sortSpatially(std::vector<Part>& parts) {
    std::vector<CGAL::Bbox_2> boxes;
    boxes.reserve(parts.size());
    for(const Part& part: parts) boxes.push_back(boxOf(part));

    CGAL::Bbox_2 total = boxes.front();
    for(const CGAL::Bbox_2& box: boxes) total += box;

    // Центры ложатся на целочисленную сетку: ключ Мортона строится из битов
    // координат, а не из самих double.
    constexpr double gridMax = 0xFFFF;
    const double sx          = total.xmax() > total.xmin() ? gridMax / (total.xmax() - total.xmin()) : 0.0;
    const double sy          = total.ymax() > total.ymin() ? gridMax / (total.ymax() - total.ymin()) : 0.0;

    // Биты числа, растянутые через один: 0b1011 -> 0b01000101.
    auto spread = [](std::uint32_t value) {
        std::uint64_t bits = value;
        bits               = (bits | (bits << 16)) & 0x0000FFFF0000FFFFull;
        bits               = (bits | (bits << 8)) & 0x00FF00FF00FF00FFull;
        bits               = (bits | (bits << 4)) & 0x0F0F0F0F0F0F0F0Full;
        bits               = (bits | (bits << 2)) & 0x3333333333333333ull;
        bits               = (bits | (bits << 1)) & 0x5555555555555555ull;
        return bits;
    };

    std::vector<std::pair<std::uint64_t, std::size_t>> order(parts.size());
    for(std::size_t i = 0; i < parts.size(); ++i) {
        const CGAL::Bbox_2& box = boxes[i];
        const auto x            = static_cast<std::uint32_t>(((box.xmin() + box.xmax()) / 2 - total.xmin()) * sx);
        const auto y            = static_cast<std::uint32_t>(((box.ymin() + box.ymax()) / 2 - total.ymin()) * sy);
        order[i]                = {spread(x) | (spread(y) << 1), i};
    }
    std::ranges::sort(order);

    std::vector<Part> sorted;
    sorted.reserve(parts.size());
    for(const auto& [key, index]: order) sorted.push_back(std::move(parts[index]));
    parts = std::move(sorted);
}

// Куски Минковского ОДНОЙ кривой точного контура с кругом радиуса d.
void appendCapsules(Polylines& out, const XCurve& xc, double d) {
    const CurveGeometry curve = geometryOf(xc);

    // Диск в начале кривой -- скругление стыка. Конец каждой кривой
    // замкнутого контура совпадает с началом следующей, так что по одному
    // диску на кривую покрывает все стыки ровно по разу.
    out.push_back(Offset::disc(curve.from, d));

    // x-монотонная дуга не длиннее полуокружности, поэтому |bulge| <= 1 --
    // ровно то, что bulge-вид и умеет выразить.
    const double bulge = curve.linear ? 0.0 : std::tan(curve.sweep / 4.0);
    for(Polyline& capsule: Offset::capsulesFor(Vertex(curve.from, bulge), curve.to, d))
        out.push_back(std::move(capsule));
}

// Соседние вершины ЗАМКНУТОГО контура, легшие в одну точку (в том числе
// первая с последней -- на стыке замыкания), схлопываются в одну: нулевой
// сегмент между ними не несёт ни длины, ни площади, зато его прогиб --
// ПРИЗРАЧНОЕ ребро вперёд-и-обратно, из-за которого простая по сути фигура
// (окружность из двух дуг, каждая дуга -- половина) не проходит проверку
// простоты CGAL (`is_valid_unknown_polygon`), потому что видна четырьмя
// вершинами вместо канонических двух, и путь через Offset::fullCircleOf её
// не узнаёт.
//
// Источник -- сам Gerber: область (G36...G37) закрывается явным D02 в
// стартовую точку ПЕРЕД первой дугой, и тот же D02 уже был отмечен при
// входе в область -- отсюда дубль. У прогиба схлопнутой вершины прока нет
// (сегмент, который он описывал, исчез вместе с ней), остаётся прогиб
// СЛЕДУЮЩЕЙ -- он описывает уже настоящий, ненулевой сегмент.
Polyline weldClosedDuplicates(const Polyline& poly) {
    if(!poly.closed || poly.size() < 3) return poly;

    Polyline out;
    out.closed = poly.closed;
    out.width  = poly.width;
    out.reserve(poly.size());
    for(const Vertex& v: poly) {
        if(!out.empty() && static_cast<QPointF>(out.back()) == static_cast<QPointF>(v)) {
            out.back().bulge = v.bulge; // прогиб исчезнувшего ребра теряется, следующего -- остаётся
            continue;
        }
        out.push_back(v);
    }
    // Замыкающий стык: последняя вершина может лечь на первую точно так же.
    // Прогиб первой уже описывает ЕЁ СОБСТВЕННЫЙ (настоящий) исходящий
    // сегмент и трогать его незачем -- отбрасывается только сама
    // последняя вершина: ребро от неё к первой всё равно нулевое.
    if(out.size() > 1 && static_cast<QPointF>(out.back()) == static_cast<QPointF>(out.front()))
        out.pop_back();
    return out;
}

} // namespace

namespace {

// Одна попытка сборки: третья точка каждой дуги берётся на параметре `t`
// вместо жёсткой середины. Возвращает nullopt, если сборка в принципе
// невозможна (вырожденный контур) -- отдельно от того, вышел ли результат
// ПРАВИЛЬНЫМ: это проверяет уже toGPoly, у которого есть с чем сравнить
// (bulge-вид), а не эта функция.
std::optional<GPoly> buildGPoly(const Polyline& poly, double t) {
    const Traits traits;
    auto makeX = traits.make_x_monotone_2_object();

    std::vector<XCurve> xcurves;
    auto addCurve = [&](const Curve& curve) {
        std::vector<std::variant<Traits::Point_2, XCurve>> pieces;
        makeX(curve, std::back_inserter(pieces));
        for(const auto& piece: pieces)
            if(const XCurve* xc = std::get_if<XCurve>(&piece)) xcurves.push_back(*xc);
    };

    // Полная окружность -- КАНОНИЧЕСКИ, одной точной Circle_2 из центра и
    // квадрата радиуса. Так диски скруглений вокруг общей вершины двух
    // рёбер (см. OffsetCapsules.h) дают буквально одну и ту же окружность,
    // и объединение сливает их без микрозигзага двух «почти одинаковых»
    // окружностей, построенных из разных троек точек.
    if(const auto circle = Offset::fullCircleOf(poly)) {
        const K::FT cx(circle->first.x()), cy(circle->first.y());
        const K::FT r2 = CGAL::square(K::FT(circle->second));
        addCurve(Curve(K::Circle_2(K::Point_2(cx, cy), r2, CGAL::COUNTERCLOCKWISE)));
        return GPoly(xcurves.begin(), xcurves.end());
    }

    const std::size_t n = poly.size();
    for(std::size_t i = 0; i < n; ++i) {
        const Vertex& from = poly[i];
        const Vertex& to   = poly[(i + 1) % n];
        const K::Point_2 src(from.x(), from.y());
        const K::Point_2 tgt(to.x(), to.y());
        if(src == tgt) continue; // вырожденное ребро
        // Почти нулевой прогиб -- прямая: сагитта такой «дуги» тонет ниже
        // double-эпсилона, три опорные точки выходят коллинеарными, и
        // конструктор дуги по трём точкам на этом ломается. Микрохорда --
        // так же: середину дуги пришлось бы делить на почти ноль.
        if(std::abs(from.bulge) < weldTolerance
            || std::hypot(to.x() - from.x(), to.y() - from.y()) < weldTolerance) {
            addCurve(Curve(K::Segment_2(src, tgt)));
        } else {
            const QPointF m = arcPointAt(from, to, from.bulge, t);
            addCurve(Curve(src, K::Point_2(m.x(), m.y()), tgt));
        }
    }
    if(xcurves.empty()) return std::nullopt;

    GPoly pgn(xcurves.begin(), xcurves.end());
    if(pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();
    return pgn;
}

} // namespace

std::optional<GPoly> toGPoly(const Polyline& rawPoly) {
    const Polyline poly = weldClosedDuplicates(rawPoly);
    if(!poly.closed || poly.size() < 2) return std::nullopt;

    // Двумя вершинами замкнутый контур задают только дуги: у окружности
    // это два полукруга, у линзы -- две дуги навстречу. Те же две вершины
    // без прогибов -- отрезок, пройденный туда и обратно; площади у него
    // нет, а точный свип на такой «границе» падает изнутри CGAL, не
    // доходя до нашей же проверки валидности.
    if(poly.size() == 2 && poly[0].bulge == 0.0 && poly[1].bulge == 0.0) return std::nullopt;

    // Сливер разбиения, доживший до double. Точное разбиение упирается в
    // касания (окружность, задевающая грань выреза, -- обычное дело у
    // апертур-макросов), и на выдаче наружу такое касание округляется в
    // микроконтур шириной в наноны. Площади у него нет, зато точная кривая,
    // построенная по трём почти совпавшим точкам, вырождается -- и CGAL
    // падает на ней ИЗНУТРИ orientation(), не доходя до проверки валидности
    // ниже.
    //
    // Отбор именно по ПЛОЩАДИ, а не по нулевому прогибу или размаху: у
    // сливера и то и другое ненулевое, просто ничтожное, а порог в квадрат
    // допуска сварки (1e-18 мм², то есть аттометр в квадрате) ни одну
    // настоящую деталь не заденет.
    constexpr double degenerateArea = weldTolerance * weldTolerance;
    if(std::abs(poly.signedArea()) < degenerateArea) return std::nullopt;

    const double bulgeArea = std::abs(poly.area());

    // Редкий, но настоящий сбой построения дуги по трём точкам: третья
    // точка (arcPointAt) сама по себе верна, но конструктор CGAL, получив
    // её вместе с концами, изредка собирает БОЛЬШУЮ дугу вместо малой --
    // почему именно, не выяснено, воспроизводится независимо от переноса,
    // на самой ПЕРВОЙ сборке контура. Результат остаётся ПРОСТЫМ
    // многоугольником (is_valid его не ловит), но с площадью, разительно
    // отличной от той, что даёт сам bulge-вид собственной, не зависящей от
    // CGAL формулой (Polyline::area()).
    //
    // Средство -- взять третью точку НЕ на середине дуги: другая точка --
    // другая тройка координат, и практика показывает, что CGAL с ней
    // справляется. Перебор из пяти точек на gerber1.gbr вытащил все контуры
    // до единого; если и он не помог, контур бракуется тем же путём, что и
    // невалидный -- лучше потерять деталь, чем залить чужую дырку медью.
    for(const double t: {0.5, 0.35, 0.65, 0.2, 0.8}) {
        std::optional<GPoly> pgn = buildGPoly(poly, t);
        if(!pgn) return std::nullopt; // вырожден -- другая точка дуги тут не поможет

        // Контур, прошедший round-trip через double (например, поданный
        // обратно результат прежней операции), может нести микросамокасание
        // -- точный свип такого ввода не переживает и падает внутри CGAL.
        // Валидируем сами и отбраковываем заранее: вызвавший решит, что с
        // контуром делать. Именно от этого и спасает хранение геометрии в
        // точном домене -- обёртки Polygon/Polygons (DxfPolygon.h)
        // существуют ради него.
        const Traits traits;
        if(!CGAL::is_valid_unknown_polygon(*pgn, traits)) continue;

        const double exactArea = std::abs(Cgal::signedArea(*pgn));
        const double areaScale = std::max(exactArea, bulgeArea);
        if(areaScale > 0.0 && std::abs(exactArea - bulgeArea) > 1e-6 * areaScale) continue;

        return pgn;
    }
    return std::nullopt;
}

Polyline toPolyline(const GPoly& pgn) {
    Polyline out;
    out.closed = true;
    for(auto it = pgn.curves_begin(); it != pgn.curves_end(); ++it) {
        const CurveGeometry curve = geometryOf(*it);
        const QPointF src         = curve.from;
        const double bulge        = curve.linear ? 0.0 : std::tan(curve.sweep / 4.0);
        // Микрокривая точного разбиения после округления в double
        // вырождается. Схлопываем МИКРОКРИВУЮ: если предыдущая вершина
        // легла вплотную, текущая забирает её позицию и прогиб -- выбросить
        // текущую вершину нельзя, с ней пропал бы прогиб целой настоящей
        // дуги, начинающейся сразу за микрофрагментом.
        if(!out.empty()
            && std::hypot(src.x() - QPointF(out.back()).x(), src.y() - QPointF(out.back()).y())
                < weldTolerance) {
            out.back().bulge = bulge;
            continue;
        }
        out.emplace_back(src, bulge);
    }
    // Микрокривая могла оказаться и замыкающей: последняя вершина вплотную
    // к первой -- это её же дубль.
    if(out.size() > 1
        && std::hypot(QPointF(out.back()).x() - QPointF(out.front()).x(),
               QPointF(out.back()).y() - QPointF(out.front()).y())
            < weldTolerance)
        out.pop_back();
    return out;
}

void parallelFor(std::size_t count, const std::function<void(std::size_t)>& body) {
    const unsigned workers = std::min<std::size_t>(workerCount(), count / minParallelBatch);
    if(workers < 2) {
        for(std::size_t i = 0; i < count; ++i) body(i);
        return;
    }

    // Индексы разбираются на лету, а не режутся на равные доли: контуры
    // сильно разной величины, и поделённые поровну доли потоков разошлись бы
    // по времени в разы.
    std::atomic<std::size_t> next{0};
    std::vector<std::exception_ptr> failures(workers);
    // Область отмены -- thread_local, и рабочий поток её не унаследует сам:
    // токен приходится занести в него вручную, иначе длинная операция
    // окажется непрерываемой ровно там, где она дольше всего и считает.
    const std::stop_token token = cancelToken();
    {
        std::vector<std::jthread> pool;
        pool.reserve(workers);
        for(unsigned w = 0; w < workers; ++w)
            pool.emplace_back([&, w] {
                CancelScope scope{token};
                for(std::size_t i = next++; i < count; i = next++) try {
                        checkCancelled();
                        body(i);
                    } catch(...) {
                        failures[w] = std::current_exception();
                        return;
                    }
            });
    }
    for(const std::exception_ptr& failure: failures)
        if(failure) std::rethrow_exception(failure);
}

// Куски бывают двух видов -- голые контуры и целые тела с дырками, -- а
// разбор один и тот же: у General_polygon_set_2 join() принимает и то и
// другое. Отсюда шаблон с двумя тонкими обёртками ниже.
template <typename Part>
void joinAllImpl(PolySet& region, std::vector<Part> parts) {
    if(parts.empty()) return;

    const std::size_t n      = parts.size();
    const unsigned workers   = workerCount();
    const std::size_t chunks = std::min(chunksPerWorker * workers, n / minChunkSize);
    if(chunks < 2) {
        region.join(parts.begin(), parts.end());
        return;
    }

    sortSpatially(parts);

    // Нулевой кусок объединяется прямо в region: перемещения у
    // General_polygon_set_2 нет, и отдать результат наружу иначе как лишней
    // полной копией готового разбиения не вышло бы.
    std::vector<PolySet> tail(chunks - 1);
    auto chunk = [&](std::size_t i) -> PolySet& { return i ? tail[i - 1] : region; };

    // Исключение из потока наружу не выпустишь -- CGAL бросает на невалидном
    // вводе, и такой поток унёс бы с собой всю программу. Ловим у себя и
    // перебрасываем уже в вызывающем.
    std::vector<std::exception_ptr> failures(chunks);

    // Область отмены -- thread_local; в рабочие потоки её надо занести явно.
    const std::stop_token token = cancelToken();

    std::atomic<std::size_t> next{0};
    auto buildChunks = [&] {
        CancelScope scope{token};
        for(std::size_t i = next++; i < chunks; i = next++) {
            const std::size_t from = n * i / chunks, to = n * (i + 1) / chunks;
            try {
                checkCancelled();
                chunk(i).join(parts.begin() + from, parts.begin() + to);
            } catch(...) {
                failures[i] = std::current_exception();
            }
        }
    };
    {
        std::vector<std::jthread> pool;
        pool.reserve(workers);
        for(unsigned w = 0; w < workers; ++w) pool.emplace_back(buildChunks);
    }

    // Дерево слияний: на каждом уровне пары независимы и идут параллельно,
    // так что уровней log2(chunks) с убывающим параллелизмом. Пары -- всегда
    // соседние, иначе пространственная связность, ради которой куски и
    // отсортированы, рассыпалась бы на первом же уровне.
    for(std::size_t step = 1; step < chunks; step *= 2) {
        std::vector<std::jthread> level;
        for(std::size_t i = 0; i + step < chunks; i += 2 * step)
            level.emplace_back([&, i, step] {
                CancelScope scope{token};
                if(failures[i] || failures[i + step]) return;
                try {
                    checkCancelled();
                    chunk(i).join(chunk(i + step));
                } catch(...) {
                    failures[i] = std::current_exception();
                }
            });
    }

    for(const std::exception_ptr& failure: failures)
        if(failure) std::rethrow_exception(failure);
}

void joinAll(PolySet& region, std::vector<GPoly> parts) {
    joinAllImpl(region, std::move(parts));
}

void joinAll(PolySet& region, std::vector<GPolyWH> parts) {
    joinAllImpl(region, std::move(parts));
}

namespace {

// Точка кривой (source()/target()) хранит координаты как Sqrt_extension --
// у пересечения дуги с дугой они, вообще говоря, иррациональны (корень из
// рационального). Сдвиг на рациональные (dx, dy) складывается с этим числом
// как есть, не трогая сам корень: a + b*sqrt(r) + dx = (a+dx) + b*sqrt(r).
Traits::Point_2 shiftPoint(const Traits::Point_2& p, const K::FT& dx, const K::FT& dy) {
    return Traits::Point_2(p.x() + dx, p.y() + dy);
}

// Одна x-монотонная кривая точного контура, сдвинутая на (dx, dy). Прямая
// пересобирается по СВОИМ коэффициентам (a*x+b*y+c=0 переходит в
// a*x+b*y+(c-a*dx-b*dy)=0 -- школьная подстановка x=x'-dx, y=y'-dy), дуга --
// по центру опорной окружности; радиус, ориентация и то, какая это ветвь
// окружности, переносятся без изменений -- сдвиг их не трогает вовсе.
XCurve translateCurve(const XCurve& xc, const K::FT& dx, const K::FT& dy) {
    const Traits::Point_2 src = shiftPoint(xc.source(), dx, dy);
    const Traits::Point_2 tgt = shiftPoint(xc.target(), dx, dy);

    if(xc.is_linear()) {
        const K::Line_2& line = xc.supporting_line();
        const K::Line_2 shifted(line.a(), line.b(), line.c() - line.a() * dx - line.b() * dy);
        return XCurve(shifted, src, tgt);
    }

    const K::Circle_2& circ = xc.supporting_circle();
    const K::Point_2 center(circ.center().x() + dx, circ.center().y() + dy);
    const K::Circle_2 shifted(center, circ.squared_radius(), circ.orientation());
    return XCurve(shifted, src, tgt, xc.orientation());
}

GPoly translatePoly(const GPoly& pgn, const K::FT& dx, const K::FT& dy) {
    std::vector<XCurve> shifted;
    shifted.reserve(pgn.size());
    for(auto it = pgn.curves_begin(); it != pgn.curves_end(); ++it)
        shifted.push_back(translateCurve(*it, dx, dy));
    return GPoly(shifted.begin(), shifted.end());
}

} // namespace

GPolyWH translate(const GPolyWH& pgn, double dx, double dy) {
    const K::FT fdx(dx), fdy(dy);

    std::vector<GPoly> holes;
    holes.reserve(pgn.number_of_holes());
    for(auto it = pgn.holes_begin(); it != pgn.holes_end(); ++it)
        holes.push_back(translatePoly(*it, fdx, fdy));

    // Неограниченный кусок (дополнение региона) внешней границы не имеет --
    // сдвигать нечего, дырки при этом переносятся как обычно: они и задают
    // собой всё, чем такой кусок отличается от целой плоскости.
    // is_unbounded() -- это и есть пустая внешняя граница, поэтому её
    // достаточно оставить пустой (конструктор по умолчанию).
    if(pgn.is_unbounded()) {
        GPolyWH result;
        for(GPoly& hole: holes) result.add_hole(std::move(hole));
        return result;
    }

    return GPolyWH(translatePoly(pgn.outer_boundary(), fdx, fdy), holes.begin(), holes.end());
}

void appendToPath(QPainterPath& path, const GPoly& pgn) {
    bool first = true;
    for(auto it = pgn.curves_begin(); it != pgn.curves_end(); ++it) {
        const CurveGeometry curve = geometryOf(*it);
        if(first) {
            path.moveTo(curve.from);
            first = false;
        }

        if(curve.linear) {
            path.lineTo(curve.to);
            continue;
        }

        // Углы у arcTo отсчитываются в другую сторону -- отсюда минусы (та
        // же договорённость, что и в Polyline::toPath()).
        constexpr double toDeg = 180.0 / std::numbers::pi;
        const double start     = std::atan2(curve.from.y() - curve.center.y(), curve.from.x() - curve.center.x());
        const QRectF box(curve.center.x() - curve.radius, curve.center.y() - curve.radius,
            2.0 * curve.radius, 2.0 * curve.radius);
        path.arcTo(box, -start * toDeg, -curve.sweep * toDeg);
    }
    if(!first) path.closeSubpath();
}

QRectF boundingRect(const GPoly& pgn) {
    // Габарит СВОЙ, а не CGAL-овский. У того (Circle_segment_2::bbox) дуга
    // всегда растянута до края опорной окружности: у верхней дуги верх
    // берётся по макушке круга, у нижней низ -- по донышку, попадают они в
    // саму дугу или нет. Четверть окружности отдаёт габарит целого круга --
    // и штрих вдоль дуги «вырастает» на её радиус, хотя на экране его там
    // нет.
    //
    // Здесь x-монотонность кривой доведена до конца: по x крайние точки --
    // это концы, а по y к ним добавляется макушка (или донышко) окружности,
    // и только если она попала в размах кривой по x.
    bool empty = true;
    double xmin{}, xmax{}, ymin{}, ymax{};
    auto grow = [&](QPointF point) {
        if(empty) {
            xmin = xmax = point.x(), ymin = ymax = point.y(), empty = false;
            return;
        }
        xmin = std::min(xmin, point.x()), xmax = std::max(xmax, point.x());
        ymin = std::min(ymin, point.y()), ymax = std::max(ymax, point.y());
    };

    for(auto it = pgn.curves_begin(); it != pgn.curves_end(); ++it) {
        const CurveGeometry curve = geometryOf(*it);
        grow(curve.from);
        grow(curve.to);
        if(curve.linear) continue;
        const double left  = std::min(curve.from.x(), curve.to.x());
        const double right = std::max(curve.from.x(), curve.to.x());
        if(curve.center.x() <= left || curve.center.x() >= right) continue;
        // Какая из ветвей -- тем же признаком, что и в crossesRayUp.
        const bool upper = (curve.sweep > 0.0) == (curve.to.x() < curve.from.x());
        grow({curve.center.x(), curve.center.y() + (upper ? curve.radius : -curve.radius)});
    }

    if(empty) return {};
    return QRectF{QPointF{xmin, ymin}, QPointF{xmax, ymax}};
}

double signedArea(const GPoly& pgn) {
    // Формула шнурков по концам кривых плюс «довесок» дуги над своей
    // хордой -- сегмент круга площадью R^2 (размах - sin размаха) / 2. У
    // дуги по часовой стрелке размах отрицателен, и довесок сам собой
    // вычитается.
    double total = 0.0;
    for(auto it = pgn.curves_begin(); it != pgn.curves_end(); ++it) {
        const CurveGeometry curve = geometryOf(*it);
        total += curve.from.x() * curve.to.y() - curve.to.x() * curve.from.y();
        if(!curve.linear)
            total += curve.radius * curve.radius * (curve.sweep - std::sin(curve.sweep));
    }
    return total / 2.0;
}

double perimeter(const GPoly& pgn) {
    double total = 0.0;
    for(auto it = pgn.curves_begin(); it != pgn.curves_end(); ++it) {
        const CurveGeometry curve = geometryOf(*it);
        total += curve.linear
            ? std::hypot(curve.to.x() - curve.from.x(), curve.to.y() - curve.from.y())
            : curve.radius * std::abs(curve.sweep);
    }
    return total;
}

bool insideContour(const GPoly& pgn, QPointF point) {
    bool inside = false;
    for(auto it = pgn.curves_begin(); it != pgn.curves_end(); ++it)
        if(crossesRayUp(geometryOf(*it), point)) inside = !inside;
    return inside;
}

double area(const GPolyWH& pwh) {
    if(pwh.is_unbounded()) return 0.0;
    double total = std::abs(signedArea(pwh.outer_boundary()));
    for(auto it = pwh.holes_begin(); it != pwh.holes_end(); ++it) total -= std::abs(signedArea(*it));
    return total;
}

double perimeter(const GPolyWH& pwh) {
    double total = pwh.is_unbounded() ? 0.0 : perimeter(pwh.outer_boundary());
    for(auto it = pwh.holes_begin(); it != pwh.holes_end(); ++it) total += perimeter(*it);
    return total;
}

bool contains(const GPolyWH& pwh, QPointF point) {
    // Неограниченный полигон -- это «вся плоскость минус дырки»: внешней
    // границы у него нет вовсе, так что снаружи оказаться негде.
    if(!pwh.is_unbounded() && !insideContour(pwh.outer_boundary(), point)) return false;
    for(auto it = pwh.holes_begin(); it != pwh.holes_end(); ++it)
        if(insideContour(*it, point)) return false;
    return true;
}

std::vector<GPoly> boundaryCapsules(const PolySet& region, double d) {
    std::vector<GPolyWH> parts;
    region.polygons_with_holes(std::back_inserter(parts));

    Polylines capsules;
    auto addContour = [&](const GPoly& contour) {
        for(auto it = contour.curves_begin(); it != contour.curves_end(); ++it)
            appendCapsules(capsules, *it, d);
    };
    for(const GPolyWH& part: parts) {
        if(!part.is_unbounded()) addContour(part.outer_boundary());
        for(auto it = part.holes_begin(); it != part.holes_end(); ++it) addContour(*it);
    }

    // Перевод капсул в точный домен -- в несколько потоков, ровно как в
    // конструкторе региона из контуров: у каждой капсулы свой свип, и друг
    // о друге они ничего не знают.
    std::vector<std::optional<GPoly>> exact(capsules.size());
    parallelFor(capsules.size(), [&](std::size_t i) { exact[i] = toGPoly(capsules[i]); });

    std::vector<GPoly> out;
    out.reserve(capsules.size());
    for(std::optional<GPoly>& pgn: exact)
        if(pgn) out.push_back(std::move(*pgn));
    return out;
}

} // namespace Geo::Cgal
