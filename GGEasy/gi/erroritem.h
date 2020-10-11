#pragma once

#include "graphicsitem.h"

class ErrorItem : public QGraphicsItem {
    QPainterPath m_shape;
    const double m_area;

public:
    ErrorItem(const Paths& paths, double area);
    double area() const;

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;
};
