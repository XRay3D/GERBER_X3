#pragma once

#include "shape.h"

namespace Shapes {
class PolyLine final : public Shape {
public:
    explicit PolyLine(QPointF pt1, QPointF pt2);
    explicit PolyLine() { }
    ~PolyLine() = default;

    // QGraphicsItem interface
    int type() const override { return GiShapeL; }
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt);
    void addPt(const QPointF& pt);
    bool closed();

protected:
    // Shape interface
    void write(QDataStream& /*stream*/) const override { }
    void read(QDataStream& /*stream*/) override { }
    QPointF calcPos(Handler* handler) override;

private:
    QPointF centroid();
    QPointF centroidFast(); //??????
};
}
