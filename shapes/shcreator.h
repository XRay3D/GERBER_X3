/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QPointF>

class QAction;

namespace Shapes {

class Constructor {
    friend class Handler;
    inline static int type;
    inline static int counter;
    inline static QPointF point;
    inline static class Shape* item;
    inline static bool m_snap;
    inline static QAction* action;

public:
    static void addShapePoint(const QPointF& value);
    static void updateShape(const QPointF& value);
    static void finalizeShape();
    static void setType(const int value, QAction* act);
};

} // namespace ShapePr
