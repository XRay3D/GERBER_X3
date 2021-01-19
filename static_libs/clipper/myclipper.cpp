// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "app.h"
#include "qmath.h"
#include <QElapsedTimer>
#include <QLineF>
#include <myclipper.h>

//static inline bool qt_is_finite(double d)
//{
//    uchar* ch = (uchar*)&d;
//#ifdef QT_ARMFPA
//    return (ch[3] & 0x7f) != 0x7f || (ch[2] & 0xf0) != 0xf0;
//#else
//    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
//        return (ch[0] & 0x7f) != 0x7f || (ch[1] & 0xf0) != 0xf0;
//    } else {
//        return (ch[7] & 0x7f) != 0x7f || (ch[6] & 0xf0) != 0xf0;
//    }
//#endif
//}

Path CirclePath(double diametr, const IntPoint& center)
{
    if (diametr == 0.0)
        return Path();

    const double radius = diametr * 0.5;
    const int intSteps = App::settings().clpCircleSegments(radius * dScale);
    Path poligon(intSteps);
    for (int i = 0; i < intSteps; ++i) {
        poligon[i] = IntPoint(
            static_cast<cInt>(cos(i * 2 * M_PI / intSteps) * radius) + center.X,
            static_cast<cInt>(sin(i * 2 * M_PI / intSteps) * radius) + center.Y);
    }
    return poligon;
}

Path RectanglePath(double width, double height, const IntPoint& center)
{

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

void RotatePath(Path& poligon, double angle, const IntPoint& center)
{
    const bool fl = Area(poligon) < 0;
    for (IntPoint& pt : poligon) {
        const double dAangle = qDegreesToRadians(angle - pt.angleTo(center));
        const double length = pt.distTo(center);
        pt = IntPoint(static_cast<cInt>(cos(dAangle) * length), static_cast<cInt>(sin(dAangle) * length));
        pt.X += center.X;
        pt.Y += center.Y;
    }
    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

void TranslatePath(Path& path, const IntPoint& pos)
{
    if (pos.X == 0 && pos.Y == 0)
        return;
    for (auto& pt : path) {
        pt.X += pos.X;
        pt.Y += pos.Y;
    }
}

double Perimeter(const Path& path)
{
    double p = 0.0;
    for (size_t i = 0, j = path.size() - 1; i < path.size(); ++i) {
        double x = path[j].X - path[i].X;
        double y = path[j].Y - path[i].Y;
        p += x * x + y * y;
        j = i;
    }
    return sqrt(p);
}
