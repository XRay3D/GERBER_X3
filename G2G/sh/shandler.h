#pragma once

#include "rectangle.h"
#include "settings.h"
#include "shape.h"
#include <QGraphicsItem>

namespace Shapes {
class Handler : public QGraphicsItem {
    friend class Shape;

public:
    Handler(Shapes::Shape* shape, bool center = false);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setPos(QPointF pos);

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
