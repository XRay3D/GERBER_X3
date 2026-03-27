#pragma once

#include "myclipper.h"
#include <any>

using namespace std::placeholders;
namespace r = std ::ranges;
namespace v = std ::views;

namespace geo {

using Segment = std::span<Point>;

/* ----------  Вспомогательные функции  ---------- */
constexpr double norm(const QPointF& p) noexcept { return std::hypot(p.x(), p.y()); }
constexpr double dot(const QPointF& a, const QPointF& b) noexcept { return QPointF::dotProduct(a, b); }

constexpr double Length(const QPointF& p1, const QPointF p2) {
    return norm(p1 - p2);
}

struct PointF : QPointF {
    static inline constexpr double tolerance = 0.001;
    using QPointF::QPointF;
    PointF(const QPointF& pt)
        : QPointF{pt} { }

    PointF& operator=(const QPointF& pt) {
        return static_cast<QPointF&>(*this) = pt, *this;
    }

    void Rotate(double cosa, double sina) { // rotate vector by angle
        double temp = -y() * sina + x() * cosa;
        ry() = x() * sina + cosa * y();
        rx() = temp;
    }
    void Rotate(double angle) {
        if(qFuzzyIsNull(angle)) return;
        Rotate(cos(angle), sin(angle));
    }

    double length() const { return hypot(x(), y()); }

    auto normalize() -> PointF& {
        double len = length();
        if(!qFuzzyIsNull(len)) *this = (*this) / len;
        return *this;
    }

    double dist(const QPointF& p) const {
        QPointF d = p - *this;
        return sqrt(d.x() * d.x() + d.y() * d.y());
    }

    constexpr double operator%(const QPointF& p) const {
        return dist(p);
    }

    constexpr static inline qreal crossProduct(const QPointF& l, const QPointF& r) {
        return l.x() * r.y() - l.y() * r.x();
    }

    friend constexpr double operator*(const QPointF& l, const QPointF& r) { // dot product
        return dotProduct(l, r);
    }

    friend constexpr double operator^(const QPointF& l, const QPointF& r) { // cross product m0.m1.sin a = v0 ^ v1
        return crossProduct(l, r);
    }

    constexpr PointF crossV(const QPointF& v) const {
        return {
            y() * v.x() - x() * v.y(),
            x() * v.y() - y() * v.x(),
        };
    }
};

struct Vertex {
    geo::PointF pt{}, center{};
    enum Type : int {
        Line = 0,
        Ccw = 1,
        Cw = -1,
    } type{};
    const void* userData{};
    constexpr double radius() const {
        if(type) return geo::Length(center, pt);
        return std::nan("");
    }
    constexpr Vertex& setRadius(double r) {
        if(type) {
            QLineF l{center, pt};
            l.setLength(r);
            pt = l.p2();
        }
        return *this;
    }
    constexpr operator bool() const { return type != Line; }

    constexpr operator QPointF() const { return pt; }

    constexpr inline qreal x() const noexcept { return pt.x(); }

    constexpr inline qreal y() const noexcept { return pt.y(); }

    friend QDebug operator<<(QDebug dbg, const Vertex& v) { // cross product m0.m1.sin a = v0 ^ v1
        constexpr std::array types{
            "CW"sv,
            "LINE"sv,
            "CCW"sv,
        };
        return dbg.noquote() << "\nVertex(" << v.pt << v.center << types[v.type + 1] << ')';
    }
};

inline QDataStream& operator>>(QDataStream& ds, Vertex& val) {
    ds >> val.pt >> val.type;
    if(val.type) ds >> val.center;
    return ds;
}

inline QDataStream& operator<<(QDataStream& ds, const Vertex val) {
    ds << val.pt << val.type;
    if(val.type) ds << val.center;
    return ds;
}

#if 1
struct Epsilon {
    double epsilon{0.001}; // mm
    double val{};
    friend constexpr Epsilon operator*(double l, Epsilon r) { return r.val = l, r; }
    friend constexpr Epsilon operator*(Epsilon l, double r) { return l.val = r, l; }
    friend constexpr bool operator==(Epsilon l, double r) {
        return std::abs(l.val - r) <= l.epsilon;
    }
    friend constexpr bool operator==(double l, Epsilon r) {
        return std::abs(l - r.val) <= r.epsilon;
    }
};

constexpr Epsilon operator""_e(long double epsilon) { return {static_cast<double>(epsilon)}; }

static_assert(10. * 0.001_e == 10);
static_assert(10. * 0.001_e != 11);

static_assert(10. == 10 * 0.001_e);
static_assert(10. != 11 * 0.001_e);
#endif

//------------------------------------------------------------------------------

//  Вычисление площади.
constexpr auto Area(std::span<const QPointF> points) -> double {
    if(points.front() == points.back()) points = points.subspan(1);

    if(points.size() < 3) return std::nan("");

    double sum{};

    // Перебираем пары
    for(auto&& [it, next]: points | v::pairwise)
        sum += it.x() * next.y() - next.x() * it.y();

    // последний и первый
    sum += points.back().x() * points.front().y()
        - points.front().x() * points.back().y();

    return sum / 2;
}

// Функция вычисления угла между отрезками OA и OB в градусах
// Возвращает угол в диапазоне [0, 180], либо NAN при вырожденных отрезках
struct {
    static constexpr double angleBetweenSegments1(const QPointF& A, const QPointF& O, const QPointF& B) {
        // Векторы OA и OB
        PointF vecOA = A - O;
        PointF vecOB = B - O;

        // Скалярное произведение
        double dotProduct = PointF::dotProduct(vecOA, vecOB);

        // Векторное произведение (для определения направления)
        double crossProduct = PointF::crossProduct(vecOA, vecOB);
        // double crossProduct = vecOA.x() * vecOB.y() - vecOA.y() * vecOB.x();

        // Длины векторов
        double lenOA = vecOA.length();
        double lenOB = vecOB.length();

        // Проверка на нулевые векторы
        if(lenOA == 0.0 || lenOB == 0.0) return 0.0;

        // Угол с учетом направления используя atan2
        double angleRad = std::atan2(crossProduct, dotProduct);

        // Преобразуем в градусы
        double angleDeg = angleRad * 180.0 / pi;

        // Если угол отрицательный, берем положительный эквивалент (внутренний угол)
        // if(angleDeg < 0.0) angleDeg += 180.0;
        // if(angleDeg < 0.0) angleDeg = 360.0 + angleDeg;
        // return 360.0 - (angleDeg < 0.0 ? angleDeg - 180.0 : angleDeg + 180.0);

        return angleDeg;
    }

    static constexpr double angleBetweenSegments2(const QPointF& A, const QPointF& O, const QPointF& B) {
        PointF vecOA = A - O;
        PointF vecOB = B - O;

        double lenOA = vecOA.manhattanLength() > 0 ? vecOA.length() : 0;
        double lenOB = vecOB.manhattanLength() > 0 ? vecOB.length() : 0;

        if(lenOA == 0.0 || lenOB == 0.0) return std::nan("");

        double dotProduct = QPointF::dotProduct(vecOA, vecOB);
        double cosAngle = dotProduct / (lenOA * lenOB);
        cosAngle = std::max(-1.0, std::min(1.0, cosAngle));

        return std::acos(cosAngle) * 180.0 / pi;
    }

    static constexpr double angleBetweenSegments3(const QPointF& A, const QPointF& O, const QPointF& B) {
        // Векторы OA и OB
        PointF vecOA = A - O;
        PointF vecOB = B - O;

        // Скалярное произведение
        double dotProduct = QPointF::dotProduct(vecOA, vecOB);

        // Длины векторов
        double lenOA = vecOA.length();
        double lenOB = vecOB.length();

        // Проверка на нулевые векторы
        if(lenOA == 0.0 || lenOB == 0.0) return 0.0;

        // cos(угла) = (a · b) / (|a| * |b|)
        double cosAngle = dotProduct / (lenOA * lenOB);

        // Ограничиваем значение [-1, 1] для избежания ошибок в arccos из-за погрешности
        cosAngle = std::max(-1.0, std::min(1.0, cosAngle));

        // Угол в радианах
        double angleRad = std::acos(cosAngle);

        // Преобразуем в градусы
        double angleDeg = angleRad * 180.0 / pi;

        return angleDeg;
    }

    template <typename... Pts>
    constexpr double operator()(const Pts... pts) const
        requires(sizeof...(Pts) == 3) // && ((std::is_same_v<Point, Pts> || std::convertible_to<Pts, QPointF>) && ...)
    {
        return angleBetweenSegments1(Cast{pts}...);
    }

    constexpr double operator()(const QPointF& A, const QPointF& O, const QPointF& B) const {
        return angleBetweenSegments1(A, O, B);
    }

    constexpr double operator()(Segment::iterator it) const {
        return angleBetweenSegments1(~*it++, ~*it++, ~*it++);
    }

    constexpr double operator()(Segment::reverse_iterator it) const {
        return angleBetweenSegments1(~*it++, ~*it++, ~*it++);
    }

    constexpr double operator()(Segment seg) const {
        assert(seg.size() > 2);
        return (*this)(seg.begin());
    }

} angle_between_segments;

//------------------------------------------------------------------------------

struct {
    // Подсчитывает количество значимых битов в мантиссе
    static constexpr int countSignificantBits(double value) noexcept {
        if(value == 0.0) return 0;
        uint64_t bits = std::bit_cast<uint64_t>(value);
        // Извлекаем мантиссу (52 бита для double)
        std::uint64_t mantissa = bits & 0x000FFFFFFFFFFFFF;
        if(mantissa == 0) return 0;
        // Считаем количество значимых битов
        return 64 - std::countl_zero(mantissa);
    }
    // Через текстовое представление
    static constexpr int measureRoundness(double value) noexcept {
        char buffer[32];
        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value,
            std::chars_format::fixed, 10);
        if(ec != std::errc()) return INT_MAX;
        std::string_view str{buffer, ptr};
        // Находим позицию запятой
        if(str.find('.') == std::string_view::npos) return 0;
        // Считаем значимые цифры после запятой
        int significantDigits = 0;
        bool foundNonzero = false;
        for(char ch: v::reverse(str)) {
            if(ch == '.') break;
            if(ch != '0' || foundNonzero) {
                significantDigits++;
                if(ch == '0') foundNonzero = true;
            }
        }
        return significantDigits;
    }

    // Находим наиболее "округлённое" число
    constexpr double operator()(r::range auto values) noexcept
        requires std::convertible_to<r::range_value_t<decltype(values)>, double>
    {
        // return *r::min_element(values, {}, countSignificantBits);
        return *r::min_element(values, {}, measureRoundness);
    }

} get_roundest;

struct {
    constexpr bool operator()(double l1, double l2,
        double epsilon = 0.001) const {
        return std::abs(l1 - l2) <= epsilon;
    }
    constexpr bool operator()(const QPointF& p1, const QPointF& p2, double l2,
        double epsilon /*= 0.001*/) const {
        return std::abs(Length(p1, p2) - l2) <= epsilon;
    }
    constexpr bool operator()(double l1, const QPointF& p1, const QPointF& p2,
        double epsilon = 0.001) const {
        return std::abs(l1 - Length(p1, p2)) <= epsilon;
    }
    constexpr bool operator()(const QPointF& p1, const QPointF& p2,
        const QPointF& p3, const QPointF& p4,
        double epsilon = 0.001) const {
        return std::abs(Length(p1, p2) - Length(p3, p4)) <= epsilon;
    }
    constexpr bool operator()(const QPointF& p1, const QPointF& p2,
        double epsilon = 0.001) const {
        return Length(p1, p2) <= epsilon;
    }

    constexpr bool operator()(const Point& p1, const Point& p2,
        const Point& p3, const Point& p4,
        double epsilon = 0.001) const {
        Point t1 = p2 - p1;
        Point t2 = p4 - p3;
        double sql1 = t1.x * t1.x + t1.y * t1.y;
        double sql2 = t2.x * t2.x + t2.y * t2.y;
        return std::abs(sql1 - sql2) <= epsilon * uScale * uScale;
    }

} constexpr TEST;

struct {
    constexpr auto operator()(std::convertible_to<Point> auto&&... points) const
        requires(sizeof...(points) > 3)
    {
        return Area({std::forward<decltype(points)>(points)...}) > .0 ? Vertex::Ccw : Vertex::Cw;
    }

    constexpr auto operator()(const QPointF& p1, const QPointF& c, const QPointF& p2) const {
        return angle_between_segments(p1, c, p2) > .0 ? Vertex::Ccw : Vertex::Cw;
    }

    constexpr auto operator()(Segment seg) const {
        assert(seg.size() > 1);
        const double crossProduct = PointF::crossProduct(~(seg[0] - !seg[1]), ~(seg[1] - !seg[1]));
        if(crossProduct > 0) return Vertex::Ccw;
        if(crossProduct < 0) return Vertex::Cw;
        return Vertex::Line;
    }
} constexpr DIR;

} // namespace geo
