#ifndef CIRCLE_H
#define CIRCLE_H

#include "shape.h"

namespace ShapePr {
class Circle : public Shape {
public:
    explicit Circle(QPointF center, QPointF pt);
    ~Circle() override = default;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return GiType::Shape; }

    // GraphicsItem interface
    void redraw() override;

    void setPt(const QPointF& pt);
    double radius() const;
    void setRadius(double radius);
    enum {
        Center,
        Point1,
    };

private:
    double m_radius;
};
}

#endif // CIRCLE_H
