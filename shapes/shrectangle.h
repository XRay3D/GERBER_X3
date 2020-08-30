#pragma once

#include "shape.h"

namespace Shapes {

class Rectangle final : public Shape {
public:
    explicit Rectangle(QPointF pt1, QPointF pt2);
    explicit Rectangle() { }
    ~Rectangle();

    // QGraphicsItem interface
    int type() const override { return GiShapeR; }
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
    };

protected:
    // Shape interface
    void write(QDataStream& /*stream*/) const override { }
    void read(QDataStream& /*stream*/) override { }
    QPointF calcPos(Handler* sh) override;
};
}
