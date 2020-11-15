// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "shape.h"
#include "graphicsview.h"
#include "scene.h"
#include "shhandler.h"
#include "shnode.h"
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

#include "leakdetector.h"

namespace Shapes {

Shape::Shape()
    : GraphicsItem(nullptr)
{
    m_paths.resize(1);
    changeColor();
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
    setVisible(true);
    //    setZValue(std::numeric_limits<double>::max());
}

Shape::~Shape() { qDeleteAll(handlers); }

void Shape::setNode(Node* node) { m_node = node; }

void Shape::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*)
{
    m_pathColor = m_bodyColor;
    m_pathColor.setAlpha(255);
    m_pen.setColor(m_pathColor);

    painter->setPen(m_pen);
    painter->setBrush(m_bodyColor);
    painter->drawPath(m_shape);
}

QRectF Shape::boundingRect() const { return m_shape.boundingRect(); }

QPainterPath Shape::shape() const { return m_shape; }

Paths Shape::paths() const { return m_paths; }

void Shape::mouseMoveEvent(QGraphicsSceneMouseEvent* event) // групповое перемещение
{
    QGraphicsItem::mouseMoveEvent(event);
    const auto dp(GlobalSettings::getSnappedPos(event->pos(), event->modifiers()) - initPos);
    for (auto& [shape, hPos] : hInitPos) {
        for (int i = 0, e = hPos.size(); i < e; ++i)
            shape->handlers[i]->QGraphicsItem::setPos(hPos[i] + dp);
        shape->redraw();
    }
}

void Shape::mousePressEvent(QGraphicsSceneMouseEvent* event) // групповое перемещение
{
    QGraphicsItem::mousePressEvent(event);
    hInitPos.clear();
    const auto p(GlobalSettings::getSnappedPos(event->pos(), event->modifiers()) - event->pos());
    initPos = event->pos() + p;
    for (auto item : scene()->selectedItems()) {
        if (static_cast<GiType>(item->type()) >= GiType::ShapeC) {
            auto* shape = static_cast<Shape*>(item);
            hInitPos[shape].reserve(shape->handlers.size());
            for (auto h : shape->handlers) {
                hInitPos[shape].append(h->pos());
            }
        }
    }
}

void Shape::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    m_node->menu(&menu, App::treeView());
    menu.exec(event->screenPos());
}

QVariant Shape::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        const bool selected = value.toInt();
        for (Handler* item : handlers)
            item->setVisible(selected);
    }
    return GraphicsItem::itemChange(change, value);
}

void Shape::write(QDataStream& /*stream*/) const { }

void Shape::read(QDataStream& /*stream*/) { }

void Shape::updateOtherHandlers(Handler*) { }

// write to project
QDataStream& operator<<(QDataStream& stream, const Shape& shape)
{
    stream << shape.type();
    stream << shape.m_id;
    stream << shape.isVisible();
    {
        stream << shape.handlers.size();
        for (Handler* item : shape.handlers) {
            stream << item->QGraphicsItem::pos();
            stream << item->m_hType;
        }
    }
    shape.write(stream);
    return stream;
}
// read from project
QDataStream& operator>>(QDataStream& stream, Shape& shape)
{
    App::scene()->addItem(&shape);
    bool visible;
    stream >> shape.m_id;
    stream >> visible;
    shape.QGraphicsItem::setVisible(visible);
    shape.setToolTip(QString::number(shape.m_id));
    {
        int size;
        stream >> size;
        shape.handlers.reserve(size);
        while (size--) {
            QPointF pos;
            int type;
            stream >> pos;
            stream >> type;
            Handler* item = new Handler(&shape, static_cast<Handler::HType>(type));
            item->QGraphicsItem::setPos(pos);
            item->setVisible(false);
            shape.handlers.append(item);
        }
    }
    shape.read(stream);
    shape.redraw();
    return stream;
}

void Shape::changeColor()
{
    animation.setStartValue(m_bodyColor);
    switch (colorState) {
    case Default:
        m_bodyColor = QColor(255, 255, 255, 50);
        break;
    case Hovered:
        m_bodyColor = QColor(255, 255, 255, 100);
        break;
    case Selected:
        m_bodyColor = QColor(255, 0x0, 0x0, 100);
        break;
    case Hovered | Selected:
        m_bodyColor = QColor(255, 0x0, 0x0, 150);
        break;
    }
    animation.setEndValue(m_bodyColor);
    animation.start();
}

}
