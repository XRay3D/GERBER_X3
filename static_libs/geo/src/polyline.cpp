#include "geo/polyline.h"
#include "geo/geo_json.h"

#include <QPainterPath>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numbers>
#include <optional>
#include <ranges>
#include <utility>

namespace Geo {

namespace r = std::ranges;
namespace v = std::views;

namespace {

// Радиус и знаковый угол (theta = 4*atan(bulge), > 0 -- против часовой
// стрелки) дуги от `from` к `to` -- ни то, ни другое не зависит от центра,
// поэтому для длины/площади сегмента центр вычислять не нужно вовсе.
// std::nullopt при bulge == 0 (прямая, не дуга) или вырожденной (нулевая
// хорда) паре точек.
std::optional<std::pair<double, double>> bulgeRadiusAndTheta(const Vertex& from, const Vertex& to) {
    if(from.bulge == 0.0) return std::nullopt;
    const double chord = std::hypot(to.x() - from.x(), to.y() - from.y());
    if(chord <= 0.0) return std::nullopt;
    const double theta = 4.0 * std::atan(from.bulge);
    const double radius = chord / (2.0 * std::abs(std::sin(theta / 2.0)));
    return std::pair{radius, theta};
}

// Длина одного сегмента полилинии (прямого или дугового) от `from` к `to`.
//
// Имя своё, не Geo::segmentLength (geo/util.h): тот считает ровно то же, но
// через Geo::arcOf, а этот файл держится на одном geo/polyline.h и тащить
// сюда util.h незачем. Два ОДИНАКОВЫХ имени в одном пространстве при
// unity-сборке склеиваются в неоднозначность -- отсюда и разные.
double segmentLen(const Vertex& from, const Vertex& to) {
    if(const auto rt = bulgeRadiusAndTheta(from, to))
        return rt->first * std::abs(rt->second);             // длина дуги = радиус * |угол|
    return std::hypot(to.x() - from.x(), to.y() - from.y()); // прямой сегмент -- длина хорды
}

// Площадь "кругового сегмента" -- разница между площадью дугового куска и
// площадью треугольника/хорды, которую формула шнурков уже учла бы, если
// бы этот сегмент был прямым. Знак совпадает со знаком bulge/theta, так
// что её можно просто прибавлять к обычной знаковой площади по вершинам.
double bulgeSegmentArea(const Vertex& from, const Vertex& to) {
    const auto rt = bulgeRadiusAndTheta(from, to);
    if(!rt) return 0.0;
    const auto [radius, theta] = *rt;
    return 0.5 * radius * radius * (theta - std::sin(theta));
}

} // namespace

// Appends a circular arc segment (start angle a1, sweeping to a2, both in
// radians) to `path` via QPainterPath::arcTo, whose angles are
// clockwise-positive in Qt's convention -- hence the negation. Callers keep
// the individual segments under 90 degrees for good accuracy.
void appendArcSegment2(QPainterPath& path, QPointF center, double radius, double a1, double a2)
    pre(radius > 0.0) {
    constexpr double toDeg = 180.0 / std::numbers::pi;
    const QRectF rect(center.x() - radius, center.y() - radius, 2.0 * radius, 2.0 * radius);
    path.arcTo(rect, -a1 * toDeg, -(a2 - a1) * toDeg);
}

// Expands a bulge-defined arc from `from` to `to` into `path`, splitting it
// into sub-90-degree segments for a smooth approximation.
void appendBulgeArc(QPainterPath& path, QPointF from, QPointF to, double bulge)
    pre(bulge != 0.0) {
    constexpr double pi = std::numbers::pi;

    const double dx = to.x() - from.x();
    const double dy = to.y() - from.y();
    const double chord = std::hypot(dx, dy);
    if(chord <= 0.0) {
        path.lineTo(to);
        return;
    }

    const double theta = 4.0 * std::atan(bulge);
    const double sagitta = bulge * chord / 2.0;
    const double radius = chord / (2.0 * std::abs(std::sin(theta / 2.0)));

    const QPointF n(dy / chord, -dx / chord); // chord rotated -90 degrees
    const QPointF mid((from.x() + to.x()) / 2.0, (from.y() + to.y()) / 2.0);
    // `radius` above is unsigned, but the center only lands on the correct
    // side of the chord if it's subtracted with bulge's own sign --
    // otherwise sagitta and radius fail to cancel for a negative bulge,
    // misplacing the center entirely.
    const QPointF center = mid + n * (sagitta - std::copysign(radius, bulge));

    const double a1 = std::atan2(from.y() - center.y(), from.x() - center.x());
    const double a2 = a1 + theta;

    const int segments = std::max(1, static_cast<int>(std::ceil(std::abs(theta) / (pi / 2.0))));
    const double step = theta / segments;
    for(int i = 0; i < segments; ++i)
        appendArcSegment2(path, center, radius, a1 + i * step, a1 + (i + 1) * step);
}

Polyline::Polyline(const QPolygonF& pgn)
    : vector{std::from_range, pgn} {
}

QPainterPath Polyline::toPath() const {
    QPainterPath path;
    if(empty())
        return path;

    path.moveTo(front().x(), front().y());

    auto addSegment = [&path](const Vertex& from, const Vertex& to) {
        if(from.bulge == 0.0)
            path.lineTo(to);
        else
            appendBulgeArc(path, from, to, from.bulge);
    };

    for(auto&& [from, to]: *this | v::pairwise)
        addSegment(from, to);

    if(closed)
        addSegment(back(), front());

    if(closed)
        path.closeSubpath();

    return path;
}

Polyline::operator QPolygonF() const {
    auto tr = QTransform::fromScale(100, 100);
    return tr.inverted().map(toPath().toFillPolygon(tr));
}

void Polyline::reverse() {
    if(size() < 2) return;

    // Прогиб хранится на "исходящей" вершине сегмента, так что при
    // перестановке точек в обратном порядке его нельзя просто оставить на
    // месте: он должен переехать на новую исходящую вершину ЭТОГО ЖЕ
    // сегмента и поменять знак (направление обхода дуги тоже развернулось).
    // Забираем исходные прогибы в отдельный вектор до перестановки точек,
    // чтобы было с чем сверяться после.
    std::vector<double> bulge;
    bulge.reserve(size());
    r::copy(*this | v::transform(&Vertex::bulge), std::back_inserter(bulge));

    r::reverse(*this); // сами точки — просто переставляются в обратном порядке

    if(!closed)
        bulge.pop_back(); // хвостовой прогиб открытой полилинии не описывает никакого сегмента

    r::reverse(bulge);
    r::transform(bulge, bulge.begin(), [](double b) { return -b; });

    if(closed)
        // Циклический сдвиг на одну позицию: у замкнутой полилинии прогиб
        // последнего (перед разворотом) сегмента после разворота
        // оказывается на новой ПЕРВОЙ вершине, а не последней.
        r::rotate(bulge, bulge.begin() + 1);
    else
        bulge.push_back(0.0); // новая последняя вершина — конец, без исходящего сегмента

    for(auto&& [vertex, b]: v::zip(*this, bulge))
        vertex.bulge = b;
}

Polyline Polyline::reversed() const {
    Polyline copy{*this};
    copy.reverse();
    return copy;
}

Polyline& Polyline::close() {
    // Прогиб последнего сегмента лежит на ПРЕДпоследней вершине (он всегда на
    // исходящей), так что замыкающая дуга переживает удаление дубля сама. У
    // самого же дубля прогиб не описывает никакого сегмента -- он уходит.
    if(size() > 1 && static_cast<const QPointF&>(front()) == static_cast<const QPointF&>(back()))
        pop_back();
    closed = true;
    return *this;
}

double Polyline::signedArea() const pre(closed) {
    double area = 0.0;

    auto shoelaceAndArc = [&](const Vertex& from, const Vertex& to) {
        area += from.x() * to.y() - to.x() * from.y(); // обычная формула шнурков (без деления на 2 -- один раз в конце)
        area += 2.0 * bulgeSegmentArea(from, to);      // площадь дугового "довеска" над/под хордой (тот же множитель 1/2 сократится ниже)
    };

    for(auto&& [from, to]: *this | v::pairwise)
        shoelaceAndArc(from, to);
    if(size() > 1) shoelaceAndArc(back(), front());

    return area / 2.0;
}

double Polyline::area() const pre(closed) {
    return std::abs(signedArea());
}

QRectF Polyline::boundingRect() const {
    if(empty()) return {};

    double xMin = front().x(), xMax = xMin;
    double yMin = front().y(), yMax = yMin;
    auto include = [&](QPointF p) {
        xMin = std::min(xMin, p.x());
        xMax = std::max(xMax, p.x());
        yMin = std::min(yMin, p.y());
        yMax = std::max(yMax, p.y());
    };

    auto extendByArc = [&](const Vertex& from, const Vertex& to) {
        include(to);
        const auto rt = bulgeRadiusAndTheta(from, to);
        if(!rt) return;
        const auto [radius, theta] = *rt;

        // Дуга выпирает за хорду, так что одних концов мало: крайние по осям
        // точки окружности (углы 0, 90, 180, 270) попадают в габарит, только
        // когда лежат внутри размаха самой дуги.
        const double chord = std::hypot(to.x() - from.x(), to.y() - from.y());
        const QPointF n((to.y() - from.y()) / chord, -(to.x() - from.x()) / chord);
        const QPointF mid((from.x() + to.x()) / 2.0, (from.y() + to.y()) / 2.0);
        const QPointF center = mid + n * (from.bulge * chord / 2.0 - std::copysign(radius, from.bulge));

        const double a0 = std::atan2(from.y() - center.y(), from.x() - center.x());
        for(int quadrant = 0; quadrant < 4; ++quadrant) {
            const double angle = quadrant * std::numbers::pi / 2.0;
            // Смещение от начала дуги в сторону её обхода, приведённое к
            // [0, 2*pi): внутри размаха, если не превосходит |theta|.
            double delta = std::remainder(angle - a0, 2.0 * std::numbers::pi);
            if(theta >= 0.0) {
                if(delta < 0.0) delta += 2.0 * std::numbers::pi;
                if(delta > theta) continue;
            } else {
                if(delta > 0.0) delta -= 2.0 * std::numbers::pi;
                if(delta < theta) continue;
            }
            include({center.x() + radius * std::cos(angle), center.y() + radius * std::sin(angle)});
        }
    };

    for(auto&& [from, to]: *this | v::pairwise)
        extendByArc(from, to);
    if(closed && size() > 1)
        extendByArc(back(), front());

    return QRectF(QPointF(xMin, yMin), QPointF(xMax, yMax));
}

double Polyline::perimeter() const {
    double total = 0.0;

    for(auto&& [from, to]: *this | v::pairwise)
        total += segmentLen(from, to);
    if(closed)
        total += segmentLen(back(), front());

    return total;
}

QPainterPath Polylines::toPath() const {
    QPainterPath path;
    for(const Polyline& polyline: *this)
        path.addPath(polyline.toPath());
    return path;
}

// ---------------------------------------------------------------------------
// Сериализация
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// JSON (см. кодировку в geo_json.h)
// ---------------------------------------------------------------------------

} // namespace Geo

using namespace Serial;
using Geo::Vertex, Geo::Polyline, Geo::Polylines;

void Serial::Adapter<Geo::Vertex>::write(simdjson::builder::string_builder& sb, const Geo::Vertex& vertex) {
    sb.start_array();
    sb.append(vertex.x());
    sb.append_comma();
    sb.append(vertex.y());
    if(vertex.bulge != 0.0) {
        sb.append_comma();
        sb.append(vertex.bulge);
    }
    sb.end_array();
}

simdjson::error_code Serial::Adapter<Geo::Vertex>::read(simdjson::ondemand::value& val, Geo::Vertex& vertex) {
    simdjson::ondemand::array arr;
    if(auto err = val.get_array().get(arr); err) return err;
    double xyb[3]{};
    size_t i{};
    for(auto elem: arr) {
        if(i == 3) return simdjson::CAPACITY;
        if(auto err = elem.get_double().get(xyb[i]); err) return err;
        ++i;
    }
    if(i < 2) return simdjson::INCORRECT_TYPE;
    vertex = Vertex{xyb[0], xyb[1], xyb[2]};
    return simdjson::SUCCESS;
}

void Serial::Adapter<Geo::Polyline>::write(simdjson::builder::string_builder& sb, const Geo::Polyline& polyline) {
    sb.start_object();
    if(polyline.closed) sb.append_raw("\"c\":true,");
    if(polyline.width != 0.0) {
        sb.append_raw("\"w\":");
        sb.append(polyline.width);
        sb.append_comma();
    }
    sb.append_raw("\"v\":[");
    bool first = true;
    for(const Vertex& vertex: polyline) {
        if(!first) sb.append_comma();
        first = false;
        Adapter<Geo::Vertex>::write(sb, vertex);
    }
    sb.append_raw("]");
    sb.end_object();
}

simdjson::error_code Serial::Adapter<Geo::Polyline>::read(simdjson::ondemand::value& val, Geo::Polyline& polyline) {
    simdjson::ondemand::object obj;
    if(auto err = val.get_object().get(obj); err) return err;
    polyline.clear();
    polyline.closed = false;
    polyline.width = 0.0;
    for(auto field: obj) { // один проход, порядок записи известен: c, w, v
        std::string_view key;
        if(auto err = field.unescaped_key().get(key); err) return err;
        if(key == "c") {
            if(auto err = field.value().get_bool().get(polyline.closed); err) return err;
        } else if(key == "w") {
            if(auto err = field.value().get_double().get(polyline.width); err) return err;
        } else if(key == "v") {
            simdjson::ondemand::array arr;
            if(auto err = field.value().get_array().get(arr); err) return err;
            // Не resize: Vertex по умолчанию не создаётся вовсе -- прогиб без
            // точки ничего не значит, и «пустая» вершина типу не нужна.
            for(auto elem: arr) {
                simdjson::ondemand::value v;
                if(auto err = elem.get(v); err) return err;
                Vertex vertex{0.0, 0.0};
                if(auto err = Adapter<Geo::Vertex>::read(v, vertex); err)
                    return polyline.clear(), err;
                polyline.push_back(vertex);
            }
        }
    }
    return simdjson::SUCCESS;
}

void Serial::Adapter<Geo::Polylines>::write(simdjson::builder::string_builder& sb, const Geo::Polylines& polylines) {
    sb.start_array();
    bool first = true;
    for(const Polyline& polyline: polylines) {
        if(!first) sb.append_comma();
        first = false;
        Adapter<Geo::Polyline>::write(sb, polyline);
    }
    sb.end_array();
}

simdjson::error_code Serial::Adapter<Geo::Polylines>::read(simdjson::ondemand::value& val, Geo::Polylines& polylines) {
    simdjson::ondemand::array arr;
    if(auto err = val.get_array().get(arr); err) return err;
    polylines.clear();
    for(auto elem: arr) {
        simdjson::ondemand::value v;
        if(auto err = elem.get(v); err) return err;
        Polyline polyline;
        if(auto err = Adapter<Geo::Polyline>::read(v, polyline); err)
            return polylines.clear(), err;
        polylines.push_back(std::move(polyline));
    }
    return simdjson::SUCCESS;
}
