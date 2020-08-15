#pragma once



#include "rectangle.h"
#include "shape.h"
#include <QGraphicsItem>

namespace ShapePr {
class SH : public QGraphicsItem {
public:
    SH(ShapePr::Shape* shape, bool center = false);

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    ShapePr::Shape* shape;
    const bool center;
    QVector<QPointF> pt;
    inline QRectF rect() const;

    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};
}

