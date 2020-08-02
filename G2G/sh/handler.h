#pragma once
//#ifndef HANDLER_H
//#define HANDLER_H

#include <QGraphicsItem>

namespace ShapePr {
class Handler : public QGraphicsItem {
public:
    explicit Handler(QGraphicsItem* parent = nullptr);

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // QGraphicsItem interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};
}

//#endif // HANDLER_H
