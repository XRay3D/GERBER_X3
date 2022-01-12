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
//#include "line.h"
//#include "qdatastream.h"
//#include "qdebug.h"
//#include "qmath.h"

//#ifndef two_pi
//#define two_pi 6.28318530717958647692528676655900576
//#endif

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

//double IntLine::length() const
//{
//    double x = pt2.X - pt1.X;
//    double y = pt2.Y - pt1.Y;
//    return qSqrt(x * x + y * y);
//}

//double IntLine::angle() const
//{
//    const double dx = pt2.X - pt1.X;
//    const double dy = pt2.Y - pt1.Y;

//    const double theta = qAtan2(-dy, dx) * 360.0 / two_pi;

//    const double theta_normalized = theta < 0 ? theta + 360 : theta;

//    if (qFuzzyCompare(theta_normalized, double(360)))
//        return double(0);
//    else
//        return theta_normalized;
//}

//void IntLine::setAngle(double angle)
//{
//    const double angleR = angle * two_pi / 360.0;
//    const double l = length();

//    const double dx = qCos(angleR) * l;
//    const double dy = -qSin(angleR) * l;

//    pt2.X = pt1.X + dx;
//    pt2.Y = pt1.Y + dy;
//}

//IntLine IntLine::fromPolar(double length, double angle)
//{
//    const double angleR = angle * two_pi / 360.0;
//    return IntLine(0, 0, qCos(angleR) * length, -qSin(angleR) * length);
//}

//IntLine IntLine::unitVector() const
//{
//    double x = pt2.X - pt1.X;
//    double y = pt2.Y - pt1.Y;

//    double len = qSqrt(x * x + y * y);
//    IntLine f(p1(), IntPoint(pt1.X + x / len, pt1.Y + y / len));

//#ifndef QT_NO_DEBUG
//    if (qAbs(f.length() - 1) >= 0.001)
//        qWarning("QLine::unitVector: New line does not have unit length");
//#endif

//    return f;
//}

//IntLine::IntersectType IntLine::intersect(const IntLine& l, IntPoint* intersectionPoint) const
//{
//    // ipmlementation is based on Graphics Gems III's "Faster Line Segment Intersection"
//    const IntPoint a = pt2 - pt1;
//    const IntPoint b = l.pt1 - l.pt2;
//    const IntPoint c = pt1 - l.pt1;

//    const double denominator = a.Y * b.X - a.X * b.Y;
//    if (denominator == 0 || !qt_is_finite(denominator))
//        return NoIntersection;

//    const double reciprocal = 1 / denominator;
//    const double na = (b.Y * c.X - b.X * c.Y) * reciprocal;
//    if (intersectionPoint)
//        *intersectionPoint = pt1 + a * na;

//    if (na < 0 || na > 1)
//        return UnboundedIntersection;

//    const double nb = (a.X * c.Y - a.Y * c.X) * reciprocal;
//    if (nb < 0 || nb > 1)
//        return UnboundedIntersection;

//    return BoundedIntersection;
//}

//double IntLine::angleTo(const IntLine& l) const
//{
//    if (isNull() || l.isNull())
//        return 0;

//    const double a1 = angle();
//    const double a2 = l.angle();

//    const double delta = a2 - a1;
//    const double delta_normalized = delta < 0 ? delta + 360 : delta;

//    if (qFuzzyCompare(delta, double(360)))
//        return 0;
//    else
//        return delta_normalized;
//}

//double IntLine::angle(const IntLine& l) const
//{
//    if (isNull() || l.isNull())
//        return 0;
//    double cos_line = (dx() * l.dx() + dy() * l.dy()) / (length() * l.length());
//    double rad = 0;
//    // only accept cos_line in the range [-1,1], if it is outside, use 0 (we return 0 rather than PI for those cases)
//    if (cos_line >= -1.0 && cos_line <= 1.0)
//        rad = qAcos(cos_line);
//    return rad * 360 / two_pi;
//}
