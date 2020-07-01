// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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
