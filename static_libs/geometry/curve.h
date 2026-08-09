/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "geometry.h"
#include <QPainterPath>

using namespace std::literals;
using namespace std::placeholders;

// Ломаная с дугами: прогиб вершины i описывает сегмент i -> i+1.
//
// Замкнутость -- ФЛАГ, а не повтор первой точки в конце: у замкнутой кривой
// есть ещё и сегмент back -> front, и прогиб последней вершины описывает
// именно его. Повторять первую точку не нужно и вредно -- получился бы лишний
// вырожденный сегмент, а сама «замкнута ли» пришлось бы каждый раз угадывать
// сравнением координат.
//
// Единственный правильный способ обойти кривую -- segments(): он и сегмент
// замыкания не потеряет, и о том, где лежит прогиб, помнить не заставит.
struct Curve : std::vector<geo::Vertex> {
    using std::vector<geo::Vertex>::vector;

    bool closed{};

    QRectF boundingRect() const;
    Curve& reverse();
    Curve reversed() const;
    double area() const;
    bool isClosed() const;
    bool isPositive() const;
    double perimetr() const;

    // Замкнуть кривую. Повторную первую точку в конце (как её принято писать
    // в DXF/Gerber/клиппере) при этом убираем: замыкающий сегмент теперь есть
    // сам по себе, а её дубликат стал бы вырожденным сегментом нулевой длины.
    Curve& close() {
        if(size() > 1 && front().pt == back().pt) pop_back();
        closed = true;
        return *this;
    }

    // Число сегментов: у замкнутой кривой на один больше -- замыкающий.
    size_t segmentCount() const noexcept {
        return empty() ? 0 : size() - !closed;
    }

    using SegmentRef = std::pair<const geo::Vertex&, const geo::Vertex&>;

    // Сегменты парами (начальная вершина, конечная), включая замыкающий.
    auto segments() const {
        return v::iota(size_t{}, segmentCount())
            | v::transform([this](size_t i) -> SegmentRef {
                  return {at(i), at((i + 1) % size())};
              });
    }

    // Геометрия сегмента, начинающегося в вершине i; nullopt -- отрезок.
    std::optional<geo::Arc> arcAt(size_t i) const {
        if(i >= segmentCount()) return {};
        return geo::arcOf(at(i).pt, at((i + 1) % size()).pt, at(i).bulge);
    }
};

using Geo::Polygon = std::vector<Curve>;
using Geo::Polygons = std::vector<Geo::Polygon>;

inline QDataStream& operator>>(QDataStream& stream, Curve& container) {
    uint32_t n;
    stream >> n >> container.closed;
    container.resize(n);
    for(auto& var: container) {
        stream >> var;
        if(stream.status() != QDataStream::Ok)
            return container.clear(), stream;
    }
    return stream;
}

inline QDataStream& operator<<(QDataStream& stream, const Curve& container) {
    stream << uint32_t(container.size()) << container.closed;
    for(const auto& var: container) stream << var;
    return stream;
}

QIcon drawIcon(const Geo::Polygon& curves, QColor color = Qt::black);

constexpr double Area(const Curve& curve) { return curve.area(); }
constexpr double Area(const Geo::Polygon& curves) {
    return r::fold_left(curves | v::transform(&Curve::area), 0.0, std::plus{});
}
constexpr QRectF BoundingRect(const Geo::Polygon& curves) {
    if(curves.empty()) return {};
    return r::fold_left(
        curves | v::transform(&Curve::boundingRect),
        curves.front().boundingRect(),
        std::bit_or{});
}

Curve CircleCurve(double diametr, const QPointF& center = {});
Curve RectangleCurve(double width, double height, const QPointF& center = {});

// Дуга «центр + радиус + углы» в вершины с прогибами. Сегменты не длиннее
// полуокружности: прогиб растёт как tan(theta/4) и у почти полного оборота
// улетает в бесконечность, поэтому крупные дуги приходится резать. Полный
// оборот (|sweep| >= 2*pi) даёт замкнутую окружность из трёх вершин.
Curve ArcCurve(const QPointF& center, double radius, double startAngle, double sweep);

Curve& TransformCurve(Curve& curve, const QTransform& tr);
Geo::Polygon& TransformCurves(Geo::Polygon& curve, const QTransform& tr);
Geo::Polygon& ReverseCurves(Geo::Polygon& curves);
Curve& TranslateCurve(Curve& curve, const QPointF& pos = {});
void RotateCurve(Curve& curve, double angle, const QPointF& center = {});
//------------------------------------------------------------------------------

QPainterPath toPPath(Curve curve, std::optional<QTransform> tr = {}, int arcLine = {});
QPainterPath toPPath(const Geo::Polygon& curves);

Curve toCurve(std::span<const QPointF> path);
Curve toCurve(std::span<const Point64> path);
Geo::Polygon toCurves(std::span<const Path64> paths /*, bool closed = true*/);
Geo::Polygons toCurvess(std::span<const Paths64> pathss /*, bool closed = true*/);

Path64 toPath(const Curve& curve);
Paths64 toPaths(const Geo::Polygon& curves);

Geo::Polygon toCurves(const QPainterPath& pPath);
inline Paths64 toPaths(const QPainterPath& pPath) {
    return toPaths(toCurves(pPath));
    // Paths64 paths;
    // Point64 rpc;
    // for(const Curve& curve: curves) {
    //     Path64 path{curve.front()};
    //     for(auto&& [fr, to]: v::pairwise(curve)) {
    //         if(to.type) {
    //             if(rpc != Point64{})
    //                 SetCForce(path.back(),
    //                     rpc == ~to.center ? ~to.center
    //                                       : path.back()); // центр для смещения при смене направления дуги

    //             path.append_range(AddVertex(fr, to));
    //             rpc = ~to.center;
    //         } else
    //             path.emplace_back(~to.pt);
    //     }
    //     r::for_each(path, SetCSelf);
    //     if(path.size()) paths.emplace_back(std::move(path));
    // }
}

//------------------------------------------------------------------------------
// double angleBetweenSegments(const QPointF& A, const QPointF& O, const QPointF& B);
QPointF polar(QPointF p, double angle /*radians*/, double distance);
double angle(QPointF p1, QPointF p2);
double signedBulgeRadius(QPointF start, QPointF end, double bulge);
std::tuple<QPointF, double, double, double> bulgeToArc(QPointF start, QPointF end, double bulge);

struct CurveTree {
};

struct BoolOp_ {
    enum class ClipType {
        NoClip,
        Intersection,
        Union,
        Difference,
        Xor
    };

    enum class FillRule {
        EvenOdd,
        NonZero,
        Positive,
        Negative
    };

    Geo::Polygon operator()(ClipType ct, FillRule fr, const Geo::Polygon& subjects, const Geo::Polygon& clips);

    void operator()(ClipType ct, FillRule fr, const Geo::Polygon& subjects, const Geo::Polygon& clips, CurveTree& solution);

    Geo::Polygon Intersect(const Geo::Polygon& subjects, const Geo::Polygon& clips, FillRule fr);

    Geo::Polygon Union(const Geo::Polygon& subjects, const Geo::Polygon& clips, FillRule fr);

    Geo::Polygon Union(const Geo::Polygon& subjects, FillRule fr);

    Geo::Polygon Difference(const Geo::Polygon& subjects, const Geo::Polygon& clips, FillRule fr);

    Geo::Polygon Xor(const Geo::Polygon& subjects, const Geo::Polygon& clips, FillRule fr);

} inline BoolOp;

using ClipType = BoolOp_::ClipType;
using FillRule = BoolOp_::FillRule;

struct Inflate64 {

    enum class JoinType {
        Square,
        Bevel,
        Round,
        Miter
    };
    // Square : Joins are 'squared' at exactly the offset distance (more complex code)
    // Bevel  : Similar to Square, but the offset distance varies with angle (simple code & faster)

    enum class EndType {
        Polygon,
        Joined,
        Butt,
        Square,
        Round
    };
    // Butt   : offsets both sides of a path, with square blunt ends
    // Square : offsets both sides of a path, with square extended ends
    // Round  : offsets both sides of a path, with round extended ends
    // Joined : offsets both sides of a path, with joined ends
    // Polygon: offsets only one side of a closed path

    Geo::Polygon operator()(const Geo::Polygon& paths, double delta,
        JoinType jt, EndType et, double miterLimit = 2.0,
        double arcTolerance = 0.0);

    Geo::Polygon PathsZ(const Geo::Polygon& paths, double delta,
        JoinType jt, EndType et, double miterLimit = 2.0,
        double arcTolerance = 0.0);

    Geo::Polygon RoundPolygon(const Geo::Polygon& paths, double delta,
        double miterLimit = 2.0, double arcTolerance = {});

    Geo::Polygon MiterPolygon(const Geo::Polygon& paths, double delta,
        double miterLimit = 2.0, double arcTolerance = {});

} inline Inflate;

using JoinType = Inflate64::JoinType;
using EndType = Inflate64::EndType;
