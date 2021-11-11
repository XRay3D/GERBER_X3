// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
* Attributions:                                                                *
* The code in this library is an extension of Bala Vatti's clipping algorithm: *
* "A generic solution to polygon clipping"                                     *
* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
* http://portal.acm.org/citation.cfm?id=129906                                 *
*                                                                              *
*******************************************************************************/

#include "interfaces/file.h"
#include "itemgroup.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <graphicsitem.h>

GraphicsItem::GraphicsItem(FileInterface* file)
    : animation(this, "bodyColor")
    , visibleA(this, "opacity")
    , m_file(file)
    , m_pen(QPen(Qt::white, 0.0))
    , m_colorPtr(file ? &file->color() : nullptr)
    , m_color(Qt::white)
    , m_bodyColor(m_colorPtr ? *m_colorPtr : m_color)
    , m_pathColor(Qt::transparent)

{
    animation.setDuration(100);
    animation.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    connect(this, &GraphicsItem::colorChanged, [this] { update(); });
    visibleA.setDuration(100);
    visibleA.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    connect(&visibleA, &QAbstractAnimation::finished, [this] { QGraphicsObject::setVisible(visibleA.currentValue().toDouble() > 0.9); });
    QGraphicsItem::setVisible(false);
}

void GraphicsItem::setColor(const QColor& brush)
{
    m_bodyColor = m_color = brush;
    colorChanged();
}

void GraphicsItem::setColorPtr(QColor* brushColor)
{
    if (brushColor)
        m_bodyColor = *(m_colorPtr = brushColor);
    m_pathColor = m_colorPtr ? *m_colorPtr : m_color;
    colorChanged();
}

void GraphicsItem::setPen(const QPen& pen)
{
    m_pen = pen;
    colorChanged();
}

void GraphicsItem::setPenColorPtr(const QColor* penColor)
{
    if (penColor)
        m_pnColorPrt = penColor;
    colorChanged();
}

void GraphicsItem::setVisible(bool visible)
{
    //    if (visible == isVisible() && (visible && opacity() < 1.0))
    //        return;
    visibleA.setStartValue(visible ? 0.0 : 1.0);
    visibleA.setEndValue(visible ? 1.0 : 0.0);
    visibleA.start();
    if (visible) {
        setOpacity(0.0);
        QGraphicsObject::setVisible(visible);
    }
}

const FileInterface* GraphicsItem::file() const { return m_file; }

int GraphicsItem::id() const { return m_giId; }

void GraphicsItem::setId(int id) { m_giId = id; }

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
        const bool fl = value.toInt();
        fl ? colorState |= Selected : colorState &= ~Selected;
        changeColor();
    } else if (change == ItemSceneChange) {
    }
    return QGraphicsItem::itemChange(change, value);
}

double GraphicsItem::scaleFactor() const
{
    if (scene() && scene()->views().size())
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        return 1.0 / scene()->views().first()->matrix().m11();
#else
        return 1.0 / scene()->views().first()->transform().m11();
#endif
    return 1.0;
};
