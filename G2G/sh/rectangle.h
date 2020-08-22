#pragma once

#include "shape.h"

namespace ShapePr {
class Rectangle final : public Shape {
public:
    explicit Rectangle(QPointF pt1, QPointF pt2);
    Rectangle(QDataStream& stream);
    ~Rectangle();

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return GiShapeR; }

    // GraphicsItem interface
    void redraw() override;

    void setPt(const QPointF& pt);
    enum {
        Center,
        Point1,
        Point2,
    };
};
}
