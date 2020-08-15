// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shape.h"
#include "sh.h"
namespace ShapePr {
Shape::Shape()
    : GraphicsItem(nullptr)
{
}

Shape::~Shape()
{
    qDeleteAll(sh);
}

QRectF Shape::boundingRect() const { return m_shape.boundingRect() /* + QMarginsF(1, 1, 1, 1)*/; }

QPainterPath Shape::shape() const { return m_shape; }

Paths Shape::paths() const { return m_paths; }

void Shape::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseDoubleClickEvent(event);
    delete this;
}

QVariant Shape::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == GraphicsItemChange::ItemSelectedChange) {
        for (SH* item : sh)
            item->setVisible(value.toInt());
    }
    return QGraphicsItem::itemChange(change, value);
}
}
