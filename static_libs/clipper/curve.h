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
#include <QPainterPath>

using namespace std::literals;

struct PointF : QPointF {
    static inline constexpr double tolerance = 0.001;
    using QPointF::QPointF;
    PointF(QPointF pt)
        : QPointF{pt} { }

    PointF& operator=(QPointF pt) {
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

    double length() const { return sqrt(x() * x() + y() * y()); }

    double normalize() {
        double len = length();
        if(!qFuzzyIsNull(len)) *this = (*this) / len;
        return len;
    }

    double dist(const QPointF& p) const {
        QPointF d = p - *this;
        return sqrt(d.x() * d.x() + d.y() * d.y());
    }
    constexpr static inline qreal crossProduct(const QPointF& l, const QPointF& r) {
        return l.x() * r.x() - l.y() * r.y();
    }

    friend constexpr double operator*(const QPointF& l, const QPointF& r) { // dot product
        return dotProduct(l, r);
    }

    friend constexpr double operator^(const QPointF& l, const QPointF& r) { // cross product m0.m1.sin a = v0 ^ v1
        return crossProduct(l, r);
    }
};

struct Vertex {
    PointF pt{}, center{};
    enum Type : int {
        Line = 0,
        Ccw = 1,
        Cw = -1,
    } type{};

    friend QDebug operator<<(QDebug dbg, const Vertex& v) { // cross product m0.m1.sin a = v0 ^ v1
        constexpr std::array types{
            "CW"sv,
            "LINE"sv,
            "CCW"sv,
        };
        return dbg.noquote() << "Vertex(" << v.pt << v.center << types[v.type + 1] << ')';
    }
};

struct Curve : std::vector<Vertex> {
    using std::vector<Vertex>::vector;

    QRectF boundingRect() const;
    Curve& reverse();
    double area() const;
    bool isClosed() const;
};

using Curves = std::vector<Curve>;

QPainterPath toPPath(Curve curve, std::optional<QTransform> tr = {}, bool arcOnly = {});
QPainterPath toPPath(const Curves& curves);

Curve toCurve(Path& path);
