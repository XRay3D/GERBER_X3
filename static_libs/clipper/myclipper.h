/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "clipper.hpp"
#include <QDebug>
#include <QIcon>
#include <QPolygonF>

Q_DECLARE_METATYPE(ClipperLib::IntPoint)

using Pathss = mvector /*mvector*/<ClipperLib::Paths>;

double Perimeter(const ClipperLib::Path& path);

ClipperLib::Path CirclePath(double diametr, const ClipperLib::IntPoint& center = {});

ClipperLib::Path RectanglePath(double width, double height, const ClipperLib::IntPoint& center = {});

void RotatePath(ClipperLib::Path& poligon, double angle, const ClipperLib::IntPoint& center = {});

void TranslatePath(ClipperLib::Path& path, const ClipperLib::IntPoint& pos);

void mergeSegments(ClipperLib::Paths& paths, double glue = 0.0);

void mergePaths(ClipperLib::Paths& paths, const double dist = 0.0);

enum { IconSize = 24 };

QIcon drawIcon(const ClipperLib::Paths& paths);

QIcon drawDrillIcon(QColor color = Qt::black);

ClipperLib::Paths& normalize(ClipperLib::Paths& paths);

using namespace ClipperLib;
