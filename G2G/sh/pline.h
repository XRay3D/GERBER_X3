#pragma once

#include "shape.h"

namespace Shapes {
class PolyLine final : public Shape {
public:
    PolyLine(QPointF pt1, QPointF pt2);
    PolyLine(QDataStream& stream);
    ~PolyLine();

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return GiShapeL; }

    // GraphicsItem interface
    void redraw() override;
    QString name() const override { return QObject::tr("Line"); }

    void setPt(const QPointF& pt);
    void addPt(const QPointF& pt);
    bool closed();
};
}
