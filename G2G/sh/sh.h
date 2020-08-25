#pragma once

#include "rectangle.h"
#include "settings.h"
#include "shape.h"
#include <QGraphicsItem>

namespace Shapes {
class SH : public QGraphicsItem {
    friend class Shape;

public:
    SH(Shapes::Shape* shape, bool center = false);

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setPos(QPointF pos, bool fl = true);

private:
    Shape* shape;
    const bool center;
    QVector<QPointF> pt;
    inline QRectF rect() const;
    QPointF lastPos;

    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};
}
