#pragma once

#include "shape.h"
#include <QFont>
#include <QIcon>

class ShTextDialog;

namespace Shapes {
class Text final : public Shape {
    friend ShTextDialog;
    friend Shape;

    QString text;
    QString font;
    double angle = 0;
    double height = 10;
    int centerAlign = 0;

public:
    explicit Text(QPointF pt1);
    explicit Text() { }
    ~Text();

    // QGraphicsItem interface
    int type() const override { return GiShapeT; }
    void redraw() override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    // Shape interface
    QString name() const override { return QObject::tr("Text"); }
    QIcon icon() const override { return QIcon::fromTheme("draw-rectangle"); };
    QPointF calcPos(Handler* sh) const override;

protected:
    // Shape interface
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;
};

}
