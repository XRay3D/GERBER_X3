#pragma once
#ifndef COMPONENTITEM_H
#define COMPONENTITEM_H

#include "graphicsitem.h"
#include <QObject>
#include <gbrcomponent.h>

class ComponentItem final : public GraphicsItem {
    const Gerber::Component& m_component;
    QVector<QRectF> pins;
    mutable QPainterPath pathRefDes;
    mutable QVector<QPair<QPainterPath, QPointF>> pathPins;
    mutable double m_scale = std::numeric_limits<double>::max();
    mutable QPointF pt;

public:
    ComponentItem(const Gerber::Component& component, AbstractFile* file);

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    // GraphicsItem interface
    Paths paths() const override;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // COMPONENTITEM_H
