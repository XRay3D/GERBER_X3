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

struct Curve : std::vector<geo::Vertex> {
    using std::vector<geo::Vertex>::vector;

    QRectF boundingRect() const;
    Curve& reverse();
    Curve reversed() const;
    double area() const;
    bool isClosed() const;
    bool isPositive() const;
    double perimetr() const;
};

using Curves  = std::vector<Curve>;
using Curvess = std::vector<Curves>;

inline QDataStream& operator>>(QDataStream& stream, Curve& container) {
    uint32_t n;
    stream >> n;
    container.resize(n);
    for(auto& var: container) {
        stream >> var;
        if(stream.status() != QDataStream::Ok)
            return container.clear(), stream;
    }
    return stream;
}

inline QDataStream& operator<<(QDataStream& stream, const Curve& container) {
    stream << uint32_t(container.size());
    for(const auto& var: container) stream << var;
    return stream;
}



QIcon drawIcon(const Curves& curves, QColor color = Qt::black);

constexpr double Area(const Curve& curve) { return curve.area(); }
constexpr double Area(const Curves& curves) {
    return r::fold_left(curves | v::transform(&Curve::area), 0.0, std::plus{});
}
constexpr QRectF BoundingRect(const Curves& curves) {
    if(curves.empty()) return {};
    return r::fold_left(
        curves | v::transform(&Curve::boundingRect),
        curves.front().boundingRect(),
        std::bit_or{});
}

Curve CircleCurve(double diametr, const QPointF& center = {});
Curve RectangleCurve(double width, double height, const QPointF& center = {});

Curve& TransformCurve(Curve& curve, const QTransform& tr);
Curves& TransformCurves(Curves& curve, const QTransform& tr);
Curves& ReverseCurves(Curves& curves);
Curve& TranslateCurve(Curve& curve, const QPointF& pos = {});
void RotateCurve(Curve& curve, double angle, const QPointF& center = {});
//------------------------------------------------------------------------------

QPainterPath toPPath(Curve curve, std::optional<QTransform> tr = {}, int arcLine = {});
QPainterPath toPPath(const Curves& curves);

Curve toCurve(std::span<const QPointF> path);
Curve toCurve(std::span<const Point64> path);
Curves toCurves(std::span<const Path64> paths /*, bool closed = true*/);
Curvess toCurvess(std::span<const Paths64> pathss /*, bool closed = true*/);

Path64 toPath(const Curve& curve);
Paths64 toPaths(const Curves& curves);

Curves toCurves(const QPainterPath& pPath);
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
