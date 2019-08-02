#include "handler.h"

namespace Shape {
Handler::Handler(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
}

QRectF Handler::boundingRect() const
{
    return QRectF();
}

void Handler::paint(QPainter* /*painter*/, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
}
}

void Shape::Handler::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
}

void Shape::Handler::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
}

void Shape::Handler::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
}
