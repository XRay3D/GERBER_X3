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
    double perimetr() const;
};

using Curves = std::vector<Curve>;
using Curvess = std::vector<Curves>;

Curve CircleCurve(double diametr, const QPointF& center = {});
Curve RectangleCurve(double width, double height, const QPointF& center = {});

Curve& TransformCurve(Curve& curve, const QTransform& tr);
Curves& ReverseCurves(Curves& curves);
Curve& TranslateCurve(Curve& curve, const QPointF& pos = {});
void RotateCurve(Curve& curve, double angle, const QPointF& center = {});
//------------------------------------------------------------------------------

QPainterPath toPPath(Curve curve, std::optional<QTransform> tr = {}, int arcLine = {});
QPainterPath toPPath(const Curves& curves);

Curve toCurve(std::span<const QPointF> path);
Curve toCurve(std::span<const Point> path);
Curves toCurves(std::span<const Path> paths /*, bool closed = true*/);
Curvess toCurvess(std::span<const Paths> pathss /*, bool closed = true*/);

Path toPath(const Curve& curve);
Paths toPaths(const Curves& curves);

Curves toCurves(const QPainterPath& pPath);
inline Paths toPaths(const QPainterPath& pPath) {
    return toPaths(toCurves(pPath));
    // Paths paths;
    // Point rpc;
    // for(const Curve& curve: curves) {
    //     Path path{curve.front()};
    //     for(auto&& [fr, to]: v::pairwise(curve)) {
    //         if(to.type) {
    //             if(rpc != Point{})
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
