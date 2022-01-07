/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#pragma once

#include "clipper.hpp"
#include <QDebug>
#include <QPolygonF>

//QDebug operator<<(QDebug debug, const IntPoint& p)
//{
//    //QDebugStateSaver saver(debug);
//    debug.nospace() << '(' << p.X << ", " << p.Y << ')';
//    return debug;
//}

using namespace ClipperLib;

using Pathss = mvector /*mvector*/<Paths>;

double Perimeter(const Path& path);

Path CirclePath(double diametr, const IntPoint& center = IntPoint());
Path RectanglePath(double width, double height, const IntPoint& center = IntPoint());
void RotatePath(Path& poligon, double angle, const IntPoint& center = IntPoint());
void TranslatePath(Path& path, const IntPoint& pos);

void mergeSegments(Paths& paths, double glue = 0.0);
void mergePaths(Paths& paths, const double dist = 0.0);
