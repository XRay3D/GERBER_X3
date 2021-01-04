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
#include "shape.h"

namespace Shapes {

class Rectangle final : public Shape {
public:
    explicit Rectangle(QPointF pt1, QPointF pt2);
    explicit Rectangle() { }
    ~Rectangle();

    // QGraphicsItem interface
    int type() const override { return static_cast<int>(GiType::ShapeR); }
    // GraphicsItem interface
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt);
    enum {
        Center,
        Point1,
        Point2,
        Point3,
        Point4,
        Center1,
        Center2,
        Center3,
        Center4,
    };

protected:
    // Shape interface
    void updateOtherHandlers(Handler* sh) override;
};
}
