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

#include "Clipper2Lib/include/clipper2/clipper.h"
// #include "clipper.hpp"
#include <QDebug>
#include <QIcon>
#include <QPolygonF>

Q_DECLARE_METATYPE(Clipper2Lib::PointD)

class cancelException : public std::exception {
public:
    cancelException(const char* description)
        : m_descr(description) {
    }
    ~cancelException() noexcept override = default;
    const char* what() const noexcept override { return m_descr.c_str(); }

private:
    std::string m_descr;
};

namespace Clipper2Lib {
using cInt = double;
}

using Pathss = mvector /*mvector*/<Clipper2Lib::PathsD>;

double Perimeter(const Clipper2Lib::PathD& path);

Clipper2Lib::PathD CirclePath(double diametr, const Clipper2Lib::PointD& center = {});

Clipper2Lib::PathD RectanglePath(double width, double height, const Clipper2Lib::PointD& center = {});

void RotatePath(Clipper2Lib::PathD& poligon, double angle, const Clipper2Lib::PointD& center = {});

void TranslatePath(Clipper2Lib::PathD& path, const Clipper2Lib::PointD& pos);

void mergeSegments(Clipper2Lib::PathsD& paths, double glue = 0.0);

void mergePaths(Clipper2Lib::PathsD& paths, const double dist = 0.0);

enum { IconSize = 24 };

QIcon drawIcon(const Clipper2Lib::PathsD& paths);

QIcon drawDrillIcon(QColor color = Qt::black);

Clipper2Lib::PathsD& normalize(Clipper2Lib::PathsD& paths);

using namespace Clipper2Lib;

template <typename T>
inline void CleanPolygons(Path<T>&, double) { }

template <typename T>
inline Path<T>& ReversePath(Path<T>& path) {
    std::reverse(path.begin(), path.end());
    return path;
}

template <typename T>
inline Paths<T>& ReversePaths(Paths<T>& paths) {
    for (auto&& path : paths)
        ReversePath(path);
    return paths;
}

template <typename T>
inline Rect<T> GetBounds(const Paths<T>& paths) {
    Rect<T> rect;

    if (paths.size() == 0 || paths.front().size() == 0)
        return rect;

    auto pt {paths.front().front()};
    rect.bottom = pt.x;
    rect.top = pt.x;
    rect.left = pt.y;
    rect.right = pt.y;

    for (auto&& path : paths) {
        for (auto&& pt : path) {
            rect.bottom = std::min(rect.bottom, pt.x);
            rect.top = std::max(rect.bottom, pt.x);
            rect.left = std::min(rect.bottom, pt.y);
            rect.right = std::max(rect.bottom, pt.y);
        }
    }
    return rect;
}
