#include "handler.h"

namespace ShapePr {
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

void ShapePr::Handler::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
}

void ShapePr::Handler::mouseMoveEvent(QGraphicsSceneMouseEvent* /*event*/)
{
}

void ShapePr::Handler::mouseReleaseEvent(QGraphicsSceneMouseEvent* /*event*/)
{
}
