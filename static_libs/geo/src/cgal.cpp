#include "cgal.h"
#include "geo/cancel.h"
#include "offsetcapsules.h"

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

// Середина bulge-дуги (в double) -- третья точка для построения дуги через
// три точки: окружность через три рациональные точки сама рациональна, и
// кривая ТОЧНО проходит через свои концы (CGAL требует, чтобы концы лежали
// на опорной окружности ровно). Цена -- отклонение самой окружности от
// идеальной в пределах double-эпсилона; дальше всё точно.
QPointF arcMidpoint(QPointF from, QPointF to, double bulge) {
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
    const double am      = a0 + theta / 2.0;
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
void sortSpatially(std::vector<GPoly>& parts) {
    std::vector<CGAL::Bbox_2> boxes;
    boxes.reserve(parts.size());
    for(const GPoly& part: parts) boxes.push_back(part.bbox());

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

    std::vector<GPoly> sorted;
    sorted.reserve(parts.size());
    for(const auto& [key, index]: order) sorted.push_back(std::move(parts[index]));
    parts = std::move(sorted);
}

// Куски Минковского ОДНОЙ кривой точного контура с кругом радиуса d.
void appendCapsules(std::vector<Polyline>& out, const XCurve& xc, double d) {
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

} // namespace

std::optional<GPoly> toGPoly(const Polyline& poly) {
    if(!poly.closed || poly.size() < 2) return std::nullopt;

    // Двумя вершинами замкнутый контур задают только дуги: у окружности
    // это два полукруга, у линзы -- две дуги навстречу. Те же две вершины
    // без прогибов -- отрезок, пройденный туда и обратно; площади у него
    // нет, а точный свип на такой «границе» падает изнутри CGAL, не
    // доходя до нашей же проверки валидности.
    if(poly.size() == 2 && poly[0].bulge == 0.0 && poly[1].bulge == 0.0) return std::nullopt;

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
            const QPointF m = arcMidpoint(from, to, from.bulge);
            addCurve(Curve(src, K::Point_2(m.x(), m.y()), tgt));
        }
    }
    if(xcurves.empty()) return std::nullopt;

    GPoly pgn(xcurves.begin(), xcurves.end());
    if(pgn.orientation() == CGAL::CLOCKWISE) pgn.reverse_orientation();

    // Контур, прошедший round-trip через double (например, поданный обратно
    // результат прежней операции), может нести микросамокасание -- точный
    // свип такого ввода не переживает и падает внутри CGAL. Валидируем сами
    // и отбраковываем заранее: вызвавший решит, что с контуром делать.
    // Именно от этого и спасает хранение геометрии в точном домене --
    // обёртки Polygon/Polygons (DxfPolygon.h) существуют ради него.
    if(!CGAL::is_valid_unknown_polygon(pgn, traits)) return std::nullopt;
    return pgn;
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

void joinAll(PolySet& region, std::vector<GPoly> parts) {
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

    std::vector<Polyline> capsules;
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
