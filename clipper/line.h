/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
//#pragma once
//
//

//#include "clipper.hpp"
//#include <QPolygonF>
//using namespace ClipperLib;

///*******************************************************************************
// * class IntLine
// *******************************************************************************/

//class IntLine {
//public:
//    enum IntersectType {
//        NoIntersection,
//        BoundedIntersection,
//        UnboundedIntersection
//    };

//    inline IntLine();
//    inline IntLine(const IntPoint& pt1, const IntPoint& pt2);
//    inline IntLine(double x1, double y1, double x2, double y2);
//    inline IntLine(const IntLine& line)
//        : pt1(line.p1())
//        , pt2(line.p2())
//    {
//    }

//    inline IntLine(const Path& path, const int index)
//        : pt1(path.at(index))
//        , pt2(path.at((index + 1) % path.size()))
//    {
//    }

//    static IntLine fromPolar(double length, double angle);

//    bool isNull() const;

//    inline IntPoint p1() const;
//    inline IntPoint p2() const;

//    inline cInt x1() const;
//    inline cInt y1() const;

//    inline cInt x2() const;
//    inline cInt y2() const;

//    inline cInt dx() const;
//    inline cInt dy() const;

//    double length() const;
//    void setLength(double len);

//    double angle() const;
//    void setAngle(double angle);

//    double angleTo(const IntLine& l) const;

//    IntLine unitVector() const;
//    inline IntLine normalVector() const;

//    // ### Qt 6: rename intersects() or intersection() and rename IntersectType IntersectionType
//    IntersectType intersect(const IntLine& l, IntPoint* intersectionPoint) const;

//    double angle(const IntLine& l) const;

//    inline IntPoint pointAt(double t) const;
////    inline void translate(const IntPoint& p);
////    inline void translate(double dx, double dy);

////    inline IntLine translated(const IntPoint& p) const;
////    inline IntLine translated(double dx, double dy) const;

//    inline IntPoint center() const;

//    inline void setP1(const IntPoint& p1);
//    inline void setP2(const IntPoint& p2);
//    inline void setPoints(const IntPoint& p1, const IntPoint& p2);
//    inline void setLine(double x1, double y1, double x2, double y2);

//    inline bool operator==(const IntLine& d) const;
//    inline bool operator!=(const IntLine& d) const { return !(*this == d); }

//private:
//    IntPoint pt1, pt2;
//};
//Q_DECLARE_TYPEINFO(IntLine, Q_MOVABLE_TYPE);

///*******************************************************************************
// * class IntLine inline members
// *******************************************************************************/

//inline IntLine::IntLine()
//{
//}

//inline IntLine::IntLine(const IntPoint& apt1, const IntPoint& apt2)
//    : pt1(apt1)
//    , pt2(apt2)
//{
//}

//inline IntLine::IntLine(double x1pos, double y1pos, double x2pos, double y2pos)
//    : pt1(x1pos, y1pos)
//    , pt2(x2pos, y2pos)
//{
//}

//inline cInt IntLine::x1() const
//{
//    return pt1.X;
//}

//inline cInt IntLine::y1() const
//{
//    return pt1.Y;
//}

//inline cInt IntLine::x2() const
//{
//    return pt2.X;
//}

//inline cInt IntLine::y2() const
//{
//    return pt2.Y;
//}

//inline bool IntLine::isNull() const
//{
//    return pt1.X == pt2.X && pt1.Y == pt2.Y;
//}

//inline IntPoint IntLine::p1() const
//{
//    return pt1;
//}

//inline IntPoint IntLine::p2() const
//{
//    return pt2;
//}

//inline cInt IntLine::dx() const
//{
//    return pt2.X - pt1.X;
//}

//inline cInt IntLine::dy() const
//{
//    return pt2.Y - pt1.Y;
//}

////inline IntLine IntLine::normalVector() const
////{
////    return IntLine(p1(), p1() + IntPoint(dy(), -dx()));
////}

////inline void IntLine::translate(const IntPoint& point)
////{
////    pt1 += point;
////    pt2 += point;
////}

////inline void IntLine::translate(double adx, double ady)
////{
////    this->translate(IntPoint(adx, ady));
////}

////inline IntLine IntLine::translated(const IntPoint& p) const
////{
////    return IntLine(pt1 + p, pt2 + p);
////}

//inline IntLine IntLine::translated(double adx, double ady) const
//{
//    return translated(IntPoint(adx, ady));
//}

//inline IntPoint IntLine::center() const
//{
//    return IntPoint(0.5 * pt1.X + 0.5 * pt2.X, 0.5 * pt1.Y + 0.5 * pt2.Y);
//}

//inline void IntLine::setLength(double len)
//{
//    if (isNull())
//        return;
//    IntLine v = unitVector();
//    pt2 = IntPoint(pt1.X + v.dx() * len, pt1.Y + v.dy() * len);
//}

//inline IntPoint IntLine::pointAt(double t) const
//{
//    return IntPoint(pt1.X + (pt2.X - pt1.X) * t, pt1.Y + (pt2.Y - pt1.Y) * t);
//}

//inline void IntLine::setP1(const IntPoint& aP1)
//{
//    pt1 = aP1;
//}

//inline void IntLine::setP2(const IntPoint& aP2)
//{
//    pt2 = aP2;
//}

//inline void IntLine::setPoints(const IntPoint& aP1, const IntPoint& aP2)
//{
//    pt1 = aP1;
//    pt2 = aP2;
//}

//inline void IntLine::setLine(double aX1, double aY1, double aX2, double aY2)
//{
//    pt1 = IntPoint(aX1, aY1);
//    pt2 = IntPoint(aX2, aY2);
//}

//inline bool IntLine::operator==(const IntLine& d) const
//{
//    return pt1 == d.pt1 && pt2 == d.pt2;
//}

//
