// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shape.h"
#include "sh.h"
#include <app.h>
#include <scene.h>

namespace ShapePr {
Shape::Shape()
    : GraphicsItem(nullptr)
{
}

Shape::Shape(QDataStream& stream)
{
    m_paths.resize(1);
    App::scene()->addItem(this);
    stream >> m_id;
    int size;
    stream >> size;
    sh.reserve(size);
    while (size--) {
        QPointF pos;
        bool center;
        stream >> pos;
        stream >> center;
        SH* item = new SH(this, center);
        item->setPos(pos, false);
        sh.append(item);
        App::scene()->addItem(item);
        item->update();
    }
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());
}

Shape::~Shape() { qDeleteAll(sh); }

QRectF Shape::boundingRect() const { return m_shape.boundingRect() /* + QMarginsF(1, 1, 1, 1)*/; }

QPainterPath Shape::shape() const { return m_shape; }

Paths Shape::paths() const { return m_paths; }

void Shape::write(QDataStream& stream)
{
    stream << m_id;
    stream << sh.size();
    for (SH* item : sh) {
        stream << item->pos();
        stream << item->center;
    }
}

QString Shape::name() const { return QString("Circle|Rectangle|Line").split('|').value(type() - GiShapeC, "ERR"); }

void Shape::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    //    GraphicsItem::mouseDoubleClickEvent(event);
    //    delete this;
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
