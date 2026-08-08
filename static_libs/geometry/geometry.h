/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include <any>
#include <optional>

using namespace std::placeholders;
namespace r = std ::ranges;
namespace v = std ::views;

namespace geo {

using Segment = std::span<Point64>;

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

// Вершина кривой: точка плюс ПРОГИБ (bulge) сегмента, НАЧИНАЮЩЕГОСЯ в ней.
//
// Прогиб -- тангенс четверти угла дуги: b = tan(theta/4), где theta --
// знаковый угол дуги (против часовой стрелки положителен). b == 0 -- прямой
// сегмент, b == 1 -- полуокружность, |b| > 1 -- дуга больше полукруга.
// Прогиб последней вершины не описывает ничего (сегмента за ней нет).
//
// Центр и радиус здесь НЕ хранятся: они однозначно восстанавливаются из пары
// соседних точек и прогиба (arcOf), и потому не могут разъехаться с ними при
// правке координат -- ровно ради этого DXF хранит дуги именно так. Обратная
// сторона: одна вершина сама по себе о дуге ничего не знает, дуга -- всегда
// свойство ПАРЫ вершин.
struct Vertex {
    enum Dir : int {
        Cw = -1,
        Line = 0,
        Ccw = 1,
    };

    geo::PointF pt{};
    double bulge{};
    const void* userData{};

    constexpr Vertex() noexcept = default;
    constexpr Vertex(const QPointF& pt, double bulge = {}) noexcept
        : pt{pt}, bulge{bulge} { }

    constexpr Dir dir() const noexcept {
        return bulge > 0.0 ? Ccw : bulge < 0.0 ? Cw
                                               : Line;
    }
    constexpr bool isArc() const noexcept { return bulge != 0.0; }

    constexpr operator bool() const { return isArc(); }

    constexpr operator QPointF() const { return pt; }

    constexpr inline qreal x() const noexcept { return pt.x(); }

    constexpr inline qreal y() const noexcept { return pt.y(); }

    friend QDebug operator<<(QDebug dbg, const Vertex& v) {
        constexpr std::array types{
            "CW"sv,
            "LINE"sv,
            "CCW"sv,
        };
        return dbg.noquote() << "\nVertex(" << v.pt << types[v.dir() + 1] << v.bulge << ')';
    }
};

inline QDataStream& operator>>(QDataStream& ds, Vertex& val) {
    return ds >> val.pt >> val.bulge;
}

inline QDataStream& operator<<(QDataStream& ds, const Vertex val) {
    return ds << val.pt << val.bulge;
}

//------------------------------------------------------------------------------

// Дуга, восстановленная из пары точек и прогиба между ними, -- то самое, что
// Vertex умышленно не хранит. Существует ровно на время расчёта: рисования,
// длины, площади, вывода G2/G3.
struct Arc {
    QPointF center;
    double radius{};     // всегда > 0
    double startAngle{}; // atan2 в начальной точке, радианы
    double theta{};      // знаковый угол дуги, радианы: > 0 -- против часовой

    constexpr Vertex::Dir dir() const noexcept {
        return theta > 0.0 ? Vertex::Ccw : theta < 0.0 ? Vertex::Cw
                                                       : Vertex::Line;
    }
    constexpr double endAngle() const noexcept { return startAngle + theta; }
    double length() const noexcept { return radius * std::abs(theta); }
    // Точка на дуге, 0 -- начало, 1 -- конец.
    QPointF pointAt(double t) const noexcept {
        const double a = startAngle + theta * t;
        return center + QPointF{radius * std::cos(a), radius * std::sin(a)};
    }
    // Знаковая площадь кругового сегмента -- «довеска» между дугой и её
    // хордой. Знак совпадает со знаком theta, так что её достаточно просто
    // прибавить к обычной знаковой площади по вершинам.
    double segmentArea() const noexcept {
        return 0.5 * radius * radius * (theta - std::sin(theta));
    }
};

// Геометрия сегмента p1 -> p2 с прогибом bulge; nullopt -- сегмент прямой
// (bulge == 0) или вырожденный (точки совпали).
inline std::optional<Arc> arcOf(const QPointF& p1, const QPointF& p2, double bulge) {
    constexpr double eps = 1e-12;
    if(std::abs(bulge) < eps) return {};

    const double dx = p2.x() - p1.x(), dy = p2.y() - p1.y();
    const double chord = std::hypot(dx, dy);
    if(chord < eps) return {};

    // Знаковая стрела прогиба -- точное тождество sagitta = (хорда/2) * bulge,
    // из него же знаковый радиус по прямоугольному треугольнику
    // (хорда/2, sagitta, radius - sagitta).
    const double halfChord = chord / 2.0;
    const double sagitta = bulge * halfChord;
    const double radiusSigned = (halfChord * halfChord + sagitta * sagitta) / (2.0 * sagitta);

    // Левая нормаль к хорде: центр лежит на ней, на знаковом расстоянии
    // (radius - sagitta) от середины -- знак сам разворачивает центр на нужную
    // сторону хорды, отдельной ветки для отрицательного прогиба не нужно.
    const QPointF n{-dy / chord, dx / chord};
    const QPointF mid{(p1.x() + p2.x()) / 2.0, (p1.y() + p2.y()) / 2.0};

    Arc arc;
    arc.center = mid + n * (radiusSigned - sagitta);
    arc.radius = std::abs(radiusSigned);
    arc.startAngle = std::atan2(p1.y() - arc.center.y(), p1.x() - arc.center.x());
    arc.theta = 4.0 * std::atan(bulge);
    return arc;
}

// Знаковый угол дуги p1 -> p2 вокруг центра в заданном направлении:
// (0, 2*pi] для Ccw, [-2*pi, 0) для Cw. Обратная задача к arcOf -- нужна там,
// где дуга приходит извне в виде «центр + направление» (DXF ARC, G2/G3,
// восстановление дуг после клиппера).
inline double arcSweep(const QPointF& p1, const QPointF& p2,
    const QPointF& center, Vertex::Dir dir) {
    if(dir == Vertex::Line) return 0.0;
    const double a1 = std::atan2(p1.y() - center.y(), p1.x() - center.x());
    const double a2 = std::atan2(p2.y() - center.y(), p2.x() - center.x());
    double theta = a2 - a1;
    if(dir == Vertex::Ccw)
        while(theta <= 0.0) theta += 2.0 * pi;
    else
        while(theta >= 0.0) theta -= 2.0 * pi;
    return theta;
}

// Прогиб по знаковому углу дуги. Полный оборот (|theta| == 2*pi) прогибом не
// выражается вовсе -- окружность приходится резать хотя бы надвое (CircleCurve).
constexpr double bulgeOf(double theta) { return std::tan(theta / 4.0); }

inline double bulgeOf(const QPointF& p1, const QPointF& p2,
    const QPointF& center, Vertex::Dir dir) {
    return bulgeOf(arcSweep(p1, p2, center, dir));
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
        requires(sizeof...(Pts) == 3) // && ((std::is_same_v<Point64, Pts> || std::convertible_to<Pts, QPointF>) && ...)
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

} constexpr angle_between_segments;

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
    constexpr double operator()(r::range auto values) const noexcept
        requires std::convertible_to<r::range_value_t<decltype(values)>, double>
    {
        // return *r::min_element(values, {}, countSignificantBits);
        return *r::min_element(values, {}, measureRoundness);
    }

} constexpr get_roundest;

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

    constexpr bool operator()(const Point64& p1, const Point64& p2,
        const Point64& p3, const Point64& p4,
        double epsilon = 0.001) const {
        Point64 t1 = p2 - p1;
        Point64 t2 = p4 - p3;
        double sql1 = t1.x * t1.x + t1.y * t1.y;
        double sql2 = t2.x * t2.x + t2.y * t2.y;
        return std::abs(sql1 - sql2) <= epsilon * uScale * uScale;
    }

} constexpr TEST;

struct {
    constexpr auto operator()(std::convertible_to<Point64> auto&&... points) const
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
