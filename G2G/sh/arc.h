#pragma once

#include "shape.h"
#include <QIcon>

namespace Shapes {
class Arc final : public Shape {
public:
    explicit Arc(QPointF center, QPointF pt, QPointF pt2);
    Arc(QDataStream& stream);
    ~Arc();

    // QGraphicsItem interface
    int type() const override { return GiShapeA; }
    void redraw() override;
    // Shape interface
    QString name() const override { return QObject::tr("Arc"); }
    QIcon icon() const override { return QIcon::fromTheme("draw-ellipse-arc"); };
    QPointF calcPos(Handler* sh) const override;

    void setPt(const QPointF& pt);
    void setPt2(const QPointF& pt);
    double radius() const;
    void setRadius(double radius);

    enum {
        Center,
        Point1,
        Point2,
    };

private:
    mutable double m_radius;
};
}
