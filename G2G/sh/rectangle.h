#ifndef Rectangle_H
#define Rectangle_H

#include "shape.h"

namespace ShapePr {
class Rectangle : public Shape {
public:
    explicit Rectangle(QPointF pt1, QPointF pt2);
    ~Rectangle() override = default;

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

#endif // Rectangle_H
