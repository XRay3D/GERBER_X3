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
#pragma once
//#include "Clipper2/clipper2.h"
//#include "Clipper2/clipper_offset.h"
#include "clipper.hpp"
#include <QDebug>
#include <QPolygonF>
#include <QtMath>

#ifndef M_2PI
#define M_2PI (6.28318530717958647692528676655900576)
#endif

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

//QDebug operator<<(QDebug debug, const IntPoint& p)
//{
//    //QDebugStateSaver saver(debug);
//    debug.nospace() << '(' << p.X << ", " << p.Y << ')';
//    return debug;
//}

using namespace ClipperLib;

using Pathss = mvector /*mvector*/<Paths>;

//Path toPath(const QPolygonF& p);
//Paths toPaths(const mvector<QPolygonF>& p);

//QPolygonF toQPolygon(const Path& p);
//mvector<QPolygonF> toQPolygons(const Paths& p);
using Point64 = IntPoint;
double Angle(const Point64& pt1, const Point64& pt2);
double Length(const Point64& pt1, const Point64& pt2);
double Perimeter(const Path& path);

//IntPoint Center(const IntPoint& pt1, const IntPoint& pt2)
//{
//    return IntPoint(int((qint64(pt1.X) + pt2.X) / 2), int((qint64(pt1.Y) + pt2.Y) / 2));
//}

Path CirclePath(double diametr, const Point64& center = Point64());
Path RectanglePath(double width, double height, const Point64& center = Point64());
void RotatePath(Path& poligon, double angle, const Point64& center = Point64());
void TranslatePath(Path& path, const Point64& pos);
