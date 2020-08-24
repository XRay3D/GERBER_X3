// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "handler.h"

namespace Shapes {
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

void Shapes::Handler::mousePressEvent(QGraphicsSceneMouseEvent* /*event*/)
{
}

void Shapes::Handler::mouseMoveEvent(QGraphicsSceneMouseEvent* /*event*/)
{
}

void Shapes::Handler::mouseReleaseEvent(QGraphicsSceneMouseEvent* /*event*/)
{
}
