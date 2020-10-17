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
#include "shape.h"

namespace Shapes {
class PolyLine final : public Shape {
public:
    explicit PolyLine(QPointF pt1, QPointF pt2);
    explicit PolyLine() { }
    ~PolyLine() = default;

    // QGraphicsItem interface
    int type() const override { return static_cast<int>(GiType::ShapeL); }
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt);
    void addPt(const QPointF& pt);
    bool closed();

protected:
    // Shape interface
    void updateOtherHandlers(Handler* handler) override;

private:
    QPointF centroid();
    QPointF centroidFast(); //??????
};
}
