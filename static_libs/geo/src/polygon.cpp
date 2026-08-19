#include "geo/polygon.h"
#include "cgal.h"
#include "geo/cancel.h"
#include "geo/geo_json.h"

#include <QPainterPath>

namespace Geo {

using namespace Cgal;

bool isExactContour(const Polyline& contour) {
    return contour.closed && contour.size() >= 2 && toGPoly(contour).has_value();
}

namespace {

// Контур, СХЛОПНУВШИЙСЯ при материализации. В точном представлении он был
// настоящим, но состоял из микрокривых: toPolyline сваривает их по
// exitWeldTolerance, и от целого контура остаётся точка либо волосок между
// двумя вершинами.
//
// Родина таких -- офсет: там, где граница подходит к себе вплотную, сжатие
// сводит целый кусок региона в ниточку. Наружу его отдавать нельзя. Обратно
// в точную геометрию он не пройдёт (isExactContour выше), в дереве
// вложенности ничего не ограничивает, а в траектории фрезы это врезка,
// подъём и ни одного миллиметра хода.
bool isCollapsed(const Polyline& contour) {
    if(contour.size() < 2) return true;
    // Две вершины -- контур только с прогибом: окружность. Без него это ход
    // туда и обратно по одному отрезку.
    if(contour.size() == 2 && !contour.front().isArc() && !contour.back().isArc()) return true;
    return contour.perimeter() <= exitWeldTolerance;
}

} // namespace

// ---------------------------------------------------------------------------
// Polygon
// ---------------------------------------------------------------------------

struct Polygon::Impl {
    GPolyWH exact;

    // ПУСТОЙ полигон от НЕОГРАНИЧЕННОГО по одному exact не отличить: у
    // Polygon_with_holes_2 и то и другое -- пустая внешняя граница, а CGAL
    // читает её однозначно как «вся плоскость». Что имелось в виду, знает
    // только тот, кто полигон построил, -- отсюда флаг, и по умолчанию он
    // снят: и полигон по умолчанию, и полигон из негодного контура означают
    // НИЧЕГО, а не всё. Разница не косметическая -- такой полигон,
    // объединённый с регионом, накрыл бы собой весь слой.
    bool unbounded{};

    // Полигон помечен пустотой: хранимое тело то же, но означает вычитание.
    // Ленивое, поэтому в точном представлении ничего не меняется -- только
    // выдача наружу.
    bool inverted{};

    // Dxf-вид -- материализуется по первому запросу и живёт рядом с точным
    // представлением, а не вместо него.
    mutable std::optional<Polyline> outerView;
    mutable std::optional<Polylines> holesView;

    void materialize() const {
        if(outerView) return;
        outerView = exact.is_unbounded() ? Polyline{} : toPolyline(exact.outer_boundary());
        Polylines holes;
        for(auto it = exact.holes_begin(); it != exact.holes_end(); ++it)
            holes.push_back(toPolyline(*it));
        holesView = std::move(holes);

        // Пустота выражается обратным обходом: внешняя граница по часовой,
        // дырки против. Другого способа сказать «это вычитается» у плоского
        // списка полилиний нет, и Polygons читает его ровно так.
        if(inverted) {
            outerView->reverse();
            for(Polyline& hole: *holesView) hole.reverse();
        }
    }

    // Тело в КАНОНИЧЕСКОМ виде, без разворота по флагу, -- для тех, кому
    // нужно само тело, а не то, что оно означает (сериализация, копирование).
    Polylines canonicalContours() const {
        Polylines contours;
        if(!exact.is_unbounded()) contours.push_back(toPolyline(exact.outer_boundary()));
        for(auto it = exact.holes_begin(); it != exact.holes_end(); ++it)
            contours.push_back(toPolyline(*it));
        return contours;
    }
};

// Impl здесь уже полон, поэтому все пять специальных функций-членов
// std::indirect выписывает сам -- включая ГЛУБОКУЮ копию, которую
// unique_ptr пришлось бы расписывать руками.
Polygon::Polygon() = default;
Polygon::~Polygon() = default;
Polygon::Polygon(const Polygon&) = default;
Polygon::Polygon(Polygon&&) noexcept = default;
Polygon& Polygon::operator=(const Polygon&) = default;
Polygon& Polygon::operator=(Polygon&&) noexcept = default;

Polygon::Polygon(Impl&& impl): impl_{std::in_place, std::move(impl)} { }

Polygon::Polygon(const Polyline& outer, const Polylines& holes) {
    auto boundary = toGPoly(outer);
    if(!boundary) return;

    std::vector<GPoly> exactHoles;
    for(const Polyline& hole: holes)
        if(auto pgn = toGPoly(hole)) {
            // toGPoly() приводит всё к обходу против часовой стрелки, а
            // дырку CGAL ждёт навстречу телу.
            pgn->reverse_orientation();
            exactHoles.push_back(std::move(*pgn));
        }
    impl_->exact = GPolyWH(*boundary, exactHoles.begin(), exactHoles.end());
}

bool Polygon::empty() const {
    return !impl_->unbounded && impl_->exact.outer_boundary().is_empty();
}

// std::size_t Polygon::size() const {
//     return impl_->exact.number_of_holes()+impl_->exact.;
// }

const Polyline& Polygon::outer() const {
    impl_->materialize();
    return *impl_->outerView;
}

const Polylines& Polygon::holes() const {
    impl_->materialize();
    return *impl_->holesView;
}

Polylines Polygon::contours() const {
    Polylines all;
    all.reserve(1 + holes().size());
    if(!isCollapsed(outer())) all.push_back(outer());
    for(const Polyline& hole: holes())
        if(!isCollapsed(hole)) all.push_back(hole);
    return all;
}

// Мерки берутся из точного представления напрямую (Cgal::area и соседи):
// bulge-вид для них не нужен вовсе, а если его ещё не материализовали, то
// и незачем -- он и сам по себе работа, и вершины в нём уже округлены.
double Polygon::area() const { return Cgal::area(impl_->exact); }

double Polygon::perimeter() const { return Cgal::perimeter(impl_->exact); }

QRectF Polygon::boundingRect() const {
    // Прямо из точного представления, но СВОИМ обходом кривых, а не
    // GPoly::bbox(): тот у всякой дуги растягивает габарит до края опорной
    // окружности, и штрих вдоль четверти дуги вырастал бы на её радиус.
    // Дырки лежат внутри внешней границы и расширить его не могут.
    if(impl_->exact.is_unbounded()) return {};
    return Cgal::boundingRect(impl_->exact.outer_boundary());
}

bool Polygon::isUnbounded() const { return impl_->unbounded; }

bool Polygon::isInverted() const { return impl_->inverted; }

Polygon& Polygon::invert() {
    impl_->inverted = !impl_->inverted;
    // Bulge-вид материализован в ту сторону, что была до переключения, --
    // пересоберётся при следующем обращении.
    impl_->outerView.reset();
    impl_->holesView.reset();
    return *this;
}

bool Polygon::contains(QPointF point) const {
    // Габарит отсекает заведомо чужие точки одним сравнением; остальное --
    // счёт пересечений по точным кривым, без QPainterPath.
    //
    // Пустое тело даёт false в обе стороны: вычитать нечего, и «наоборот»
    // тут отвечать не на что -- иначе пустая пустота накрыла бы всю
    // плоскость, разойдясь и с empty(), и с тем, что даст Polygons.
    if(empty()) return false;
    const bool inside = [&] {
        if(!impl_->exact.is_unbounded() && !boundingRect().contains(point)) return false;
        return Cgal::contains(impl_->exact, point);
    }();
    return impl_->inverted ? !inside : inside;
}

QPainterPath Polygon::toPath() const {
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    // Прямо из точных кривых, минуя bulge-вид: он для отрисовки не нужен, а
    // лишний перевод только копил бы погрешность. Дырки идут навстречу телу
    // (канон CGAL), так что WindingFill вычитает их сам -- в отличие от
    // EvenOdd, который «вычел» бы и законный остров внутри дырки.
    if(impl_->exact.is_unbounded()) return path;
    Cgal::appendToPath(path, impl_->exact.outer_boundary());
    for(auto it = impl_->exact.holes_begin(); it != impl_->exact.holes_end(); ++it)
        Cgal::appendToPath(path, *it);

    // Пустота рисуется встречным направлением: добавленная к чужому пути с
    // WindingFill, она из него вычтется -- ровно то, что и означает.
    if(impl_->inverted) {
        path = path.toReversed();
        path.setFillRule(Qt::WindingFill);
    }
    return path;
}

// ---------------------------------------------------------------------------
// Polygons
// ---------------------------------------------------------------------------

struct Polygons::Impl {
    PolySet exact;
    mutable std::optional<std::vector<Polygon>> view;

    void invalidate() { view.reset(); }

    void materialize() const {
        if(view) return;
        std::vector<GPolyWH> parts;
        exact.polygons_with_holes(std::back_inserter(parts));
        std::vector<Polygon> polygons;
        polygons.reserve(parts.size());
        for(GPolyWH& part: parts) {
            Polygon::Impl impl;
            // Единственное место, где неограниченность БЕРЁТСЯ ИЗ CGAL: только
            // разбиение региона и может отдать кусок без внешней границы.
            impl.unbounded = part.is_unbounded();
            impl.exact = std::move(part);
            polygons.push_back(Polygon{std::move(impl)});
        }
        view = std::move(polygons);
    }
};

Polygons::Polygons() = default;
Polygons::~Polygons() = default;
Polygons::Polygons(const Polygons&) = default;
Polygons::Polygons(Polygons&&) noexcept = default;
Polygons& Polygons::operator=(const Polygons&) = default;
Polygons& Polygons::operator=(Polygons&&) noexcept = default;

Polygons::Polygons(Impl&& impl): impl_{std::in_place, std::move(impl)} { }

// Полигон, помеченный пустотой, ВЫЧИТАЕТСЯ, а не объединяется -- ровно как
// контур обратного обхода в плоском списке. Из одной лишь пустоты регион
// выходит пустым: вычитать её не из чего, и это не оплошность, а то же
// самое, что даст Polygons{polygon.contours()}.
Polygons::Polygons(const Polygon& polygon) {
    if(!polygon.empty() && !polygon.isInverted()) impl_->exact.join(polygon.impl().exact);
}

Polygons::Polygons(std::initializer_list<Polygon> polygons) {
    // Сперва все тела, и лишь потом пустоты: вычитать надо из уже собранного,
    // иначе пустота, поданная раньше своего тела, не встретит его вовсе.
    for(const Polygon& polygon: polygons)
        if(!polygon.empty() && !polygon.isInverted()) impl_->exact.join(polygon.impl().exact);
    for(const Polygon& polygon: polygons)
        if(!polygon.empty() && polygon.isInverted()) impl_->exact.difference(polygon.impl().exact);
}

Polygons::Polygons(std::span<const Polygon> polygons) {
    // Тела -- одним объединением на всю пачку (многопоточным), пустоты --
    // следом и тоже разом: вычитать надо из уже собранного, иначе пустота,
    // поданная раньше своего тела, не встретит его вовсе.
    //
    // Сливер разбиения отсеивается ПОРОГОМ ПЛОЩАДИ -- тем же, что и у
    // отдельного контура в toGPoly. Цепочка булевых операций (как в
    // ApMacro::draw, где термал собирается раздельными union/difference)
    // изредка оставляет в разбиении именно такой кусок: с исчезающей
    // площадью, но самокасающейся границей -- valid как часть СВОЕГО
    // разбиения (там его никто не проверяет заново), но невалидный как
    // САМОСТОЯТЕЛЬНЫЙ Polygon_with_holes_2. General_polygon_set_2::join()
    // валидирует каждый переданный кусок сам и на такой границе не просто
    // предупреждает -- корёжит всю принимающую арену, а не только сам
    // сливер: получившийся регион заливается там, где должна была остаться
    // дырка. Дешевле выбросить его заранее, чем разбираться потом.
    constexpr double degenerateArea = weldTolerance * weldTolerance;

    std::vector<GPolyWH> solids, voids;
    solids.reserve(polygons.size());
    for(const Polygon& polygon: polygons)
        if(!polygon.empty() && std::abs(polygon.area()) >= degenerateArea)
            (polygon.isInverted() ? voids : solids).push_back(polygon.impl().exact);

    joinAll(impl_->exact, std::move(solids));
    if(!voids.empty()) {
        PolySet holes;
        joinAll(holes, std::move(voids));
        impl_->exact.difference(holes);
    }
}

Polygons::Polygons(const Polylines& contours) {
    // В плоском списке вложенность выражена одной лишь ориентацией: контур
    // с отрицательной площадью -- пустота внутри чьего-то тела. Регион =
    // объединение тел минус объединение пустот.
    //
    // Перевод в точный домен идёт в несколько потоков: у каждого контура
    // свой собственный свип, и контуры друг о друге ничего не знают.
    // Раскладка по телам и пустотам -- уже последовательная, чтобы порядок
    // не зависел от того, какой поток успел раньше.
    std::vector<std::optional<GPoly>> exact(contours.size());
    std::vector<char> isVoid(contours.size(), 0);
    Cgal::parallelFor(contours.size(), [&](std::size_t i) {
        const Polyline& contour = contours[i];
        if(!contour.closed || contour.size() < 2) return;
        isVoid[i] = contour.signedArea() < 0.0;
        exact[i] = toGPoly(contour);
    });

    std::vector<GPoly> solids, voids;
    for(std::size_t i = 0; i < contours.size(); ++i)
        if(exact[i]) (isVoid[i] ? voids : solids).push_back(std::move(*exact[i]));
    joinAll(impl_->exact, std::move(solids));
    if(!voids.empty()) {
        PolySet holes;
        joinAll(holes, std::move(voids));
        impl_->exact.difference(holes);
    }
}

bool Polygons::empty() const { return impl_->exact.is_empty(); }

std::size_t Polygons::size() const { return impl_->exact.number_of_polygons_with_holes(); }

const std::vector<Polygon>& Polygons::all() const {
    impl_->materialize();
    return *impl_->view;
}

Polylines Polygons::contours() const {
    Polylines all;
    for(const Polygon& polygon: *this)
        all.append_range(polygon.contours());
    return all;
}

double Polygons::area() const {
    return std::transform_reduce(begin(), end(), 0.0, std::plus{},
        [](const Polygon& polygon) { return polygon.area(); });
}

QRectF Polygons::boundingRect() const {
    QRectF bounds;
    for(const Polygon& polygon: *this) {
        const QRectF own = polygon.boundingRect();
        if(own.isNull()) continue;
        bounds = bounds.isNull() ? own : bounds.united(own);
    }
    return bounds;
}

bool Polygons::contains(QPointF point) const {
    // Локализация точки прямо в разбиении: региону для этого не нужно
    // раскладываться на полигоны, а полигонам -- на контуры.
    const Traits::Point_2 query(K::FT(point.x()), K::FT(point.y()));
    return impl_->exact.oriented_side(query) == CGAL::ON_POSITIVE_SIDE;
}

QPainterPath Polygons::toPath() const {
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    for(const Polygon& polygon: *this) path.addPath(polygon.toPath());
    return path;
}

Polygons Polygons::boundaryBand(double radius) const {
    Polygons band;
    if(radius > 0.0) joinAll(band.impl_->exact, boundaryCapsules(impl_->exact, radius));
    return band;
}

// Отмена проверяется ПЕРЕД операцией: сам свип CGAL прервать нечем, он
// атомарен для вызывающего. Зато в цепочке (раздуть -> объединить -> вычесть)
// проверок оказывается столько же, сколько звеньев, и отмена срабатывает на
// ближайшей границе, а не по завершении всей цепочки.
Polygons& Polygons::operator|=(const Polygons& other) {
    checkCancelled();
    impl_->exact.join(other.impl_->exact);
    impl_->invalidate();
    return *this;
}

Polygons& Polygons::operator&=(const Polygons& other) {
    checkCancelled();
    impl_->exact.intersection(other.impl_->exact);
    impl_->invalidate();
    return *this;
}

Polygons& Polygons::operator-=(const Polygons& other) {
    checkCancelled();
    impl_->exact.difference(other.impl_->exact);
    impl_->invalidate();
    return *this;
}

Polygons& Polygons::operator^=(const Polygons& other) {
    checkCancelled();
    impl_->exact.symmetric_difference(other.impl_->exact);
    impl_->invalidate();
    return *this;
}

Polygons& Polygons::invert() {
    checkCancelled();
    impl_->exact.complement();
    impl_->invalidate();
    return *this;
}

// Определение здесь, а не рядом с прочими членами Polygon: ему нужен полный
// Polygons::Impl, а тот объявлен ниже по файлу.
Polygons Polygon::complement() const {
    // Через регион: у Polygon_with_holes_2 дополнения нет и быть не может --
    // кусков в нём столько же, сколько дырок, плюс один неограниченный.
    // Пометка пустотой сюда не вмешивается: она о том, как тело СОБИРАЮТ, а
    // дополняется само тело.
    Polygons region;
    if(!empty()) region.impl().exact.join(impl_->exact);
    return region.invert();
}

bool Polygons::isUnbounded() const {
    // По разбиению, а не по арифметике над габаритом: неограниченный кусок
    // приходит в нём отдельным полигоном без внешней границы, а разбиение
    // всё равно кэшировано.
    const std::vector<Polygon>& parts = all();
    return std::ranges::any_of(parts, [](const Polygon& part) { return part.isUnbounded(); });
}

// ---------------------------------------------------------------------------
// Точный перенос
// ---------------------------------------------------------------------------

Polygon translated(const Polygon& polygon, QPointF offset) {
    Polygon::Impl impl;
    impl.exact = Cgal::translate(polygon.impl().exact, offset.x(), offset.y());
    impl.inverted = polygon.isInverted();
    impl.unbounded = polygon.isUnbounded();
    return Polygon{std::move(impl)};
}

Polygons translated(const Polygons& polygons, QPointF offset) {
    Polygons result;
    for(const Polygon& polygon: polygons)
        result.impl().exact.join(translated(polygon, offset).impl().exact);
    return result;
}

// ---------------------------------------------------------------------------
// Сериализация
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// JSON (см. кодировку в geo_json.h)
// ---------------------------------------------------------------------------

} // namespace Geo

using namespace Serial;
using Geo::Polygon, Geo::Polygons, Geo::Polyline, Geo::Polylines;

void Serial::Adapter<Geo::Polygon>::write(simdjson::builder::string_builder& sb, const Geo::Polygon& polygon) {
    // Пишется КАНОНИЧЕСКОЕ тело плюс флаг, а не то, что отдают
    // outer()/holes(): у помеченного пустотой они уже развёрнуты, и запись их
    // вместе с флагом развернула бы контуры вторично.
    const Polylines contours = polygon.impl().canonicalContours();
    const Polyline outer = contours.empty() ? Polyline{} : contours.front();
    const Polylines holes{contours.empty() ? contours.begin() : contours.begin() + 1, contours.end()};
    sb.start_object();
    sb.append_raw("\"o\":");
    Adapter<Geo::Polyline>::write(sb, outer);
    if(!holes.empty()) {
        sb.append_raw(",\"h\":");
        Adapter<Geo::Polylines>::write(sb, holes);
    }
    if(polygon.isInverted()) sb.append_raw(",\"i\":true");
    sb.end_object();
}

simdjson::error_code Serial::Adapter<Geo::Polygon>::read(simdjson::ondemand::value& val, Geo::Polygon& polygon) {
    simdjson::ondemand::object obj;
    if(auto err = val.get_object().get(obj); err) return err;
    Polyline outer;
    Polylines holes;
    bool inverted{};
    for(auto field: obj) {
        std::string_view key;
        if(auto err = field.unescaped_key().get(key); err) return err;
        simdjson::ondemand::value v;
        if(auto err = field.value().get(v); err) return err;
        if(key == "o") {
            if(auto err = Adapter<Geo::Polyline>::read(v, outer); err) return err;
        } else if(key == "h") {
            if(auto err = Adapter<Geo::Polylines>::read(v, holes); err) return err;
        } else if(key == "i") {
            if(auto err = v.get_bool().get(inverted); err) return err;
        }
    }
    polygon = Polygon{outer, holes};
    if(inverted) polygon.invert();
    return simdjson::SUCCESS;
}

void Serial::Adapter<Geo::Polygons>::write(simdjson::builder::string_builder& sb, const Geo::Polygons& polygons) {
    sb.start_array();
    bool first = true;
    for(const Polygon& polygon: polygons.all()) {
        if(!first) sb.append_comma();
        first = false;
        Adapter<Geo::Polygon>::write(sb, polygon);
    }
    sb.end_array();
}

simdjson::error_code Serial::Adapter<Geo::Polygons>::read(simdjson::ondemand::value& val, Geo::Polygons& polygons) {
    simdjson::ondemand::array arr;
    if(auto err = val.get_array().get(arr); err) return err;
    // Не join по одному: последовательное объединение дорожает с каждым
    // полигоном, и открытие проекта с плотной медью растягивается на минуты.
    // Конструктор от span собирает регион деревом слияний в несколько потоков
    // и сам разводит тела и пустоты (инверсные куски).
    std::vector<Polygon> parsed;
    for(auto elem: arr) {
        simdjson::ondemand::value v;
        if(auto err = elem.get(v); err) return err;
        if(auto err = Adapter<Geo::Polygon>::read(v, parsed.emplace_back()); err)
            return polygons = Polygons{}, err;
    }
    polygons = Polygons{std::span<const Polygon>{parsed}};
    return simdjson::SUCCESS;
}
