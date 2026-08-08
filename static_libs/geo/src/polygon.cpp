#include "geo/polygon.h"
#include "cgal.h"
#include "geo/cancel.h"

#include <QDataStream>
#include <QPainterPath>

namespace Geo {
#if 1

using namespace Cgal;

bool isExactContour(const Polyline& contour) {
    return contour.closed && contour.size() >= 2 && toGPoly(contour).has_value();
}

// ---------------------------------------------------------------------------
// Polygon
// ---------------------------------------------------------------------------

struct Polygon::Impl {
    GPolyWH exact;

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
    }
};

Polygon::Polygon(): impl_{std::make_unique<Impl>()} { }
Polygon::~Polygon() = default;
Polygon::Polygon(const Polygon& other): impl_{std::make_unique<Impl>(*other.impl_)} { }
Polygon::Polygon(Polygon&&) noexcept = default;
Polygon& Polygon::operator=(const Polygon& other) {
    if(this != &other) impl_ = std::make_unique<Impl>(*other.impl_);
    return *this;
}
Polygon& Polygon::operator=(Polygon&&) noexcept = default;

Polygon::Polygon(const Polyline& outer, const Polylines& holes)
    : impl_{std::make_unique<Impl>()} {
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
    return impl_->exact.outer_boundary().is_empty() && !impl_->exact.is_unbounded();
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
    if(!outer().empty()) all.push_back(outer());
    all.insert(all.end(), holes().begin(), holes().end());
    return all;
}

// Мерки берутся из точного представления напрямую (Cgal::area и соседи):
// bulge-вид для них не нужен вовсе, а если его ещё не материализовали, то
// и незачем -- он и сам по себе работа, и вершины в нём уже округлены.
double Polygon::area() const { return Cgal::area(impl_->exact); }

double Polygon::perimeter() const { return Cgal::perimeter(impl_->exact); }

QRectF Polygon::boundingRect() const {
    // Прямо из точного представления: CGAL считает габарит по самим
    // кривым, так что выпуклость дуги учтена без разбора углов. Дырки
    // лежат внутри внешней границы и расширить его не могут.
    if(impl_->exact.is_unbounded()) return {};
    const CGAL::Bbox_2 box = impl_->exact.outer_boundary().bbox();
    return QRectF(QPointF(box.xmin(), box.ymin()), QPointF(box.xmax(), box.ymax()));
}

bool Polygon::contains(QPointF point) const {
    // Габарит отсекает заведомо чужие точки одним сравнением; остальное --
    // счёт пересечений по точным кривым, без QPainterPath.
    if(empty()) return false;
    if(!impl_->exact.is_unbounded() && !boundingRect().contains(point)) return false;
    return Cgal::contains(impl_->exact, point);
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
            auto impl   = std::make_unique<Polygon::Impl>();
            impl->exact = std::move(part);
            polygons.push_back(Polygon{std::move(impl)});
        }
        view = std::move(polygons);
    }
};

Polygons::Polygons(): impl_{std::make_unique<Impl>()} { }
Polygons::~Polygons() = default;
Polygons::Polygons(const Polygons& other): impl_{std::make_unique<Impl>(*other.impl_)} { }
Polygons::Polygons(Polygons&&) noexcept = default;
Polygons& Polygons::operator=(const Polygons& other) {
    if(this != &other) impl_ = std::make_unique<Impl>(*other.impl_);
    return *this;
}
Polygons& Polygons::operator=(Polygons&&) noexcept = default;

Polygons::Polygons(std::unique_ptr<Impl> impl): impl_{std::move(impl)} { }

Polygons::Polygons(const Polygon& polygon): impl_{std::make_unique<Impl>()} {
    if(!polygon.empty()) impl_->exact.join(polygon.impl().exact);
}

Polygons::Polygons(std::initializer_list<Polygon> polygons): impl_{std::make_unique<Impl>()} {
    for(const Polygon& polygon: polygons)
        if(!polygon.empty()) impl_->exact.join(polygon.impl().exact);
}

Polygons::Polygons(const Polylines& contours): impl_{std::make_unique<Impl>()} {
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
    parallelFor(contours.size(), [&](std::size_t i) {
        const Polyline& contour = contours[i];
        if(!contour.closed || contour.size() < 2) return;
        isVoid[i] = contour.signedArea() < 0.0;
        exact[i]  = toGPoly(contour);
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
    for(const Polygon& polygon: *this) {
        if(!polygon.outer().empty()) all.push_back(polygon.outer());
        all.insert(all.end(), polygon.holes().begin(), polygon.holes().end());
    }
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

// ---------------------------------------------------------------------------
// Сериализация
// ---------------------------------------------------------------------------

QDataStream& operator<<(QDataStream& stream, const Polygon& polygon) {
    return stream << polygon.outer() << polygon.holes();
}

QDataStream& operator>>(QDataStream& stream, Polygon& polygon) {
    Polyline outer;
    Polylines holes;
    stream >> outer >> holes;
    polygon = Polygon{outer, holes};
    return stream;
}

QDataStream& operator<<(QDataStream& stream, const Polygons& polygons) {
    const std::vector<Polygon>& all = polygons.all();
    stream << quint32(all.size());
    for(const Polygon& polygon: all) stream << polygon;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Polygons& polygons) {
    quint32 count{};
    stream >> count;
    Polygons result;
    while(count--) {
        Polygon polygon;
        stream >> polygon;
        if(stream.status() != QDataStream::Ok) return polygons = Polygons{}, stream;
        // Сразу в точный домен, минуя разбор по ориентациям: структура
        // полигона известна точно, объединять их между собой -- вся работа.
        if(!polygon.empty()) result.impl().exact.join(polygon.impl().exact);
    }
    polygons = std::move(result);
    return stream;
}

#else
using namespace Geo::Cgal;

bool isExactContour(const Polyline& contour) {
    return contour.closed && contour.size() >= 2 && toGPoly(contour).has_value();
}

// ---------------------------------------------------------------------------
// Polygon
// ---------------------------------------------------------------------------

struct Polygon::Impl {
    GPolyWH exact;
};

// Impl здесь уже полон, поэтому все пять специальных функций-членов
// std::indirect выписывает сам -- включая ГЛУБОКУЮ копию.
Polygon::Polygon()                              = default;
Polygon::~Polygon()                             = default;
Polygon::Polygon(const Polygon&)                = default;
Polygon::Polygon(Polygon&&) noexcept            = default;
Polygon& Polygon::operator=(const Polygon&)     = default;
Polygon& Polygon::operator=(Polygon&&) noexcept = default;

Polygon::Polygon(Impl&& impl): impl_{std::in_place, std::move(impl)} { }

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
        for(GPolyWH& part: parts)
            polygons.push_back(Polygon{Polygon::Impl{std::move(part)}});
        view = std::move(polygons);
    }
};

Polygons::Polygons()                               = default;
Polygons::~Polygons()                              = default;
Polygons::Polygons(const Polygons&)                = default;
Polygons::Polygons(Polygons&&) noexcept            = default;
Polygons& Polygons::operator=(const Polygons&)     = default;
Polygons& Polygons::operator=(Polygons&&) noexcept = default;

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
    parallelFor(contours.size(), [&](std::size_t i) {
        const Polyline& contour = contours[i];
        if(!contour.closed || contour.size() < 2) return;
        isVoid[i] = contour.signedArea() < 0.0;
        exact[i]  = toGPoly(contour);
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

const std::vector<Polygon>& Polygons::all() const {
    impl_->materialize();
    return *impl_->view;
}

Polygons Polygons::boundaryBand(double radius) const {
    Polygons band;
    if(radius > 0.0) joinAll(band.impl_->exact, boundaryCapsules(impl_->exact, radius));
    return band;
}

QPainterPath Polygons::toPath() const {
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    for(const Polygon& polygon: *this) path.addPath(polygon.toPath());
    return path;
}

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
#endif

Polygon::Polygon(std::unique_ptr<Impl> impl): impl_{std::move(impl)} { }

} // namespace Geo
