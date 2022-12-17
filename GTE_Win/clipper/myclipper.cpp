// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "myclipper.h"
#include <QElapsedTimer>
#include <QLineF>
#include <qmath.h>

Path toPath(const QPolygonF& p) {
    Path path;
    path.reserve(p.size());
    for (const QPointF& pt : p)
        path.push_back(toIntPoint(pt));
    return path;
}

Paths toPaths(const QVector<QPolygonF>& p) {
    Paths paths;
    paths.reserve(p.size());
    for (const QPolygonF& pl : p)
        paths.push_back(toPath(pl));
    return paths;
}

QPolygonF toQPolygon(const Path& p) {
    QPolygonF polygon;
    polygon.reserve(p.size());
    for (const IntPoint& pt : p)
        polygon.push_back(toQPointF(pt));
    return polygon;
}

QVector<QPolygonF> toQPolygons(const Paths& p) {
    QVector<QPolygonF> polygons;
    polygons.reserve(p.size());
    for (const Path& pl : p)
        polygons.push_back(toQPolygon(pl));
    return polygons;
}

double Angle(const IntPoint& pt1, const IntPoint& pt2) {
    const double dx = pt2.X - pt1.X;
    const double dy = pt2.Y - pt1.Y;
    const double theta = atan2(-dy, dx) * 360.0 / two_pi;
    const double theta_normalized = theta < 0 ? theta + 360 : theta;
    if (qFuzzyCompare(theta_normalized, double(360)))
        return 0.0;
    else
        return theta_normalized;
}

double Length(const IntPoint& pt1, const IntPoint& pt2) {
    double x = pt2.X - pt1.X;
    double y = pt2.Y - pt1.Y;
    return sqrt(static_cast<double>(x) * static_cast<double>(x) + static_cast<double>(y) * static_cast<double>(y));
}

// static inline bool qt_is_finite(double d)
//{
//     uchar* ch = (uchar*)&d;
// #ifdef QT_ARMFPA
//     return (ch[3] & 0x7f) != 0x7f || (ch[2] & 0xf0) != 0xf0;
// #else
//     if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
//         return (ch[0] & 0x7f) != 0x7f || (ch[1] & 0xf0) != 0xf0;
//     } else {
//         return (ch[7] & 0x7f) != 0x7f || (ch[6] & 0xf0) != 0xf0;
//     }
// #endif
// }

Path CirclePath(double diametr, const IntPoint& center) {
    if (diametr == 0.0)
        return Path();

    const double radius = diametr * 0.5;
    const int intSteps = 18;
    Path poligon(intSteps);
    for (int i = 0; i < intSteps; ++i) {
        poligon[i] = IntPoint(
            static_cast<cInt>(cos(i * 2 * pi / intSteps) * radius) + center.X,
            static_cast<cInt>(sin(i * 2 * pi / intSteps) * radius) + center.Y);
    }
    return poligon;
}

Path RectanglePath(double width, double height, const IntPoint& center) {

    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Path poligon {
        IntPoint(static_cast<cInt>(-halfWidth + center.X), static_cast<cInt>(+halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(-halfWidth + center.X), static_cast<cInt>(-halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(+halfWidth + center.X), static_cast<cInt>(-halfHeight + center.Y)),
        IntPoint(static_cast<cInt>(+halfWidth + center.X), static_cast<cInt>(+halfHeight + center.Y)),
    };
    if (Area(poligon) < 0.0)
        ReversePath(poligon);

    return poligon;
}

void RotatePath(Path& poligon, double angle, const IntPoint& center) {
    const bool fl = Area(poligon) < 0;
    for (IntPoint& pt : poligon) {
        const double dAangle = qDegreesToRadians(angle - Angle(center, pt));
        const double length = Length(center, pt);
        pt = IntPoint(static_cast<cInt>(cos(dAangle) * length), static_cast<cInt>(sin(dAangle) * length));
        pt.X += center.X;
        pt.Y += center.Y;
    }
    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

void TranslatePath(Path& path, const IntPoint& pos) {
    if (pos.X == 0 && pos.Y == 0)
        return;
    for (Path::size_type i = 0, size = path.size(); i < size; ++i) {
        path[i].X += pos.X;
        path[i].Y += pos.Y;
    }
}

double Perimeter(const Path& path) {
    double p = 0.0;
    for (int i = 0; i < path.size() - 1; ++i) {
        p += Length(path[i], path[i + 1]);
    }
    p += Length(path.first(), path.last());
    return p;
}
