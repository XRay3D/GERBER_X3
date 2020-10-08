// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shape.h"
#include "graphicsview.h"
#include "scene.h"
#include "shhandler.h"
#include "shnode.h"
//#include "filetree/treeview.h"
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

#include "leakdetector.h"

namespace Shapes {

Shape::Shape()
    : GraphicsItem(nullptr)
{
    changeColor();
}

Shape::~Shape() { qDeleteAll(handlers); }

void Shape::setNode(Node* node) { m_node = node; }

void Shape::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*)
{
    m_pen.setColor(m_pathColor);
    painter->setPen(m_pen);

    m_bodyColor = m_pathColor;
    m_bodyColor.setAlpha(50);
    painter->setBrush(m_bodyColor);
    painter->drawPath(m_shape);
}

QRectF Shape::boundingRect() const
{
    return m_shape.boundingRect();
}

QPainterPath Shape::shape() const
{
    return m_shape;
}

Paths Shape::paths() const { return m_paths; }

QVariant Shape::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        bool fl = value.toBool();
        for (Handler* item : handlers)
            item->setVisible(fl);
    }
    return GraphicsItem::itemChange(change, value);
}

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
        if (item->type() >= GiShapeC) {
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
};

// write to project
QDataStream& operator<<(QDataStream& stream, const Shape& shape)
{
    stream << shape.type();
    stream << shape.m_id;
    stream << shape.handlers.size();
    for (Handler* item : shape.handlers) {
        stream << item->pos();
        stream << item->m_hType;
    }
    shape.write(stream);
    return stream;
}
// read from project
QDataStream& operator>>(QDataStream& stream, Shape& shape)
{
    shape.m_paths.resize(1);
    App::scene()->addItem(&shape);
    stream >> shape.m_id;
    shape.setToolTip(QString::number(shape.m_id));
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
    shape.read(stream);

    shape.setFlags(GraphicsItem::ItemIsSelectable | GraphicsItem::ItemIsFocusable);
    shape.setAcceptHoverEvents(true);
    shape.setZValue(std::numeric_limits<double>::max());
    return stream;
}
}

void Shapes::Shape::changeColor()
{
    {
        auto animation = new QPropertyAnimation(dynamic_cast<GraphicsItem*>(this), "pathColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(m_pathColor);
        m_pathColor = m_colorPtr ? *m_colorPtr : m_color;
        switch (colorState) {
        case Default:
            m_pathColor = Qt::gray;
            break;
        case Hovered:
            m_pathColor = Qt::white;
            break;
        case Selected:
            m_pathColor = Qt::darkGreen;
            break;
        case Hovered | Selected:
            m_pathColor = Qt::green;
            break;
        }
        animation->setEndValue(m_pathColor);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
