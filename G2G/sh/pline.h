#pragma once

#include "shape.h"
#include <QIcon>

namespace Shapes {
class PolyLine final : public Shape {
public:
    explicit PolyLine(QPointF pt1, QPointF pt2);
    explicit PolyLine() { }
    ~PolyLine();

    // QGraphicsItem interface
    int type() const override { return GiShapeL; }
    void redraw() override;
    // Shape interface
    QString name() const override { return QObject::tr("Line"); }
    QIcon icon() const override { return QIcon::fromTheme("draw-line"); };
    QPointF calcPos(Handler* sh) const override;

    void setPt(const QPointF& pt);
    void addPt(const QPointF& pt);
    bool closed();

protected:
    // Shape interface
    void write(QDataStream& stream) const override { }
    void read(QDataStream& stream) override { }
};
}
