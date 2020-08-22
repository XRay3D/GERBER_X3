#pragma once

#include "shape.h"

namespace ShapePr {
class Circle final : public Shape {
public:
    explicit Circle(QPointF center, QPointF pt);
    Circle(QDataStream& stream);
    ~Circle();

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return GiShapeC; }

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
