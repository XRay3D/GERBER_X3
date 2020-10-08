// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "graphicsitem.h"
#include "abstractfile.h"
#include "itemgroup.h"
#include <QPropertyAnimation>

GraphicsItem::GraphicsItem(AbstractFile* file)
    : m_file(file)
    , m_pen(QPen(Qt::white, 0.0))
    , m_color(Qt::white)
    , m_colorPtr(file ? &file->color() : nullptr)
    , m_bodyColor(m_colorPtr ? *m_colorPtr : m_color)
    , m_pathColor(Qt::transparent)
{
}

void GraphicsItem::setVisible(bool visible)
{
    auto animation = new QPropertyAnimation(this, "opacity");
    animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    animation->setDuration(200);
    animation->setStartValue(visible ? 0.0 : 1.0);
    animation->setEndValue(visible ? 1.0 : 0.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    if (visible)
        QGraphicsObject::setVisible(visible);
    else
        connect(animation, &QPropertyAnimation::finished, [visible, this] { QGraphicsObject::setVisible(visible); });
}

const AbstractFile* GraphicsItem::file() const { return m_file; }

int GraphicsItem::id() const { return m_id; }

void GraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

void GraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant GraphicsItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        if (value.toInt())
            colorState |= Selected;
        else
            colorState &= ~Selected;
        changeColor();
    }
    return QGraphicsItem::itemChange(change, value);
}
