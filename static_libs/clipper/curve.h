/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "myclipper.h"
#include "span.h"
#include <QPainterPath>

using namespace std::literals;
using namespace std::placeholders;

struct Curve : std::vector<Vertex> {
    using std::vector<Vertex>::vector;

    QRectF boundingRect() const;
    Curve& reverse();
    double area() const;
    bool isClosed() const;
    double perimetr() const;
};

using Curves = std::vector<Curve>;

QPainterPath toPPath(Curve curve, std::optional<QTransform> tr = {}, bool arcOnly = {});
QPainterPath toPPath(const Curves& curves);

Curve toCurve(std::span<Point> path, bool open = {});
Curves toCurves(std::span<Path> paths, bool open = {});

Curve CircleCurve(double diametr, const PointF& center = {});
Curve RectangleCurve(double width, double height, const PointF& center = {});

Curve& TransformCurve(Curve& curve, const QTransform& tr);
Curves& ReverseCurves(Curves& curves);
Curve& TranslateCurve(Curve& curve, const PointF& pos = {});
void RotateCurve(Curve& curve, double angle, const PointF& center = {});

Path toPath(const Curve& curve);
Paths toPaths(const Curves& curves);

Paths toPaths(const QPainterPath& pPath);
