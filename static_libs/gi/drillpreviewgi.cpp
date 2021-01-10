// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "drillpreviewgi.h"
#include "forms/drillform/drillmodel.h"
#include "graphicsview.h"
#include "tool.h"
#include <QPainter>
#include <QPropertyAnimation>

AbstractDrillPrGI::AbstractDrillPrGI(Row& row)
    : row(row)
    , m_bodyColor(colors[(int)Colors::Default])
    , m_pathColor(colors[(int)Colors::UnUsed])
{
    connect(this, &AbstractDrillPrGI::colorChanged, [this] { update(); });
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setOpacity(0);
    setZValue(std::numeric_limits<double>::max() - 10);
}

void AbstractDrillPrGI::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen({ m_bodyColor, 0.0 });
    painter->setBrush(m_bodyColor);
    painter->drawPath(m_sourcePath);
    // draw tool
    if (row.toolId > -1) {
        painter->setPen(QPen(m_pathColor, 2 * App::graphicsView()->scaleFactor()));
        painter->setBrush(Qt::NoBrush);
        if (m_toolPath.isEmpty())
            painter->drawPath(App::toolHolder().tool(row.toolId).path(pos()));
        else
            painter->drawPath(m_toolPath);
    }
}

QRectF AbstractDrillPrGI::boundingRect() const { return m_sourcePath.boundingRect(); }

int AbstractDrillPrGI::type() const { return static_cast<int>(m_type); }

double AbstractDrillPrGI::sourceDiameter() const { return m_sourceDiameter; }

int AbstractDrillPrGI::toolId() const { return row.toolId; }

void AbstractDrillPrGI::changeColor()
{
    if (row.useForCalc)
        colorState |= Used;
    else
        colorState &= ~Used;
    {
        auto animation = new QPropertyAnimation(this, "bodyColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(m_bodyColor);
        if (colorState & Selected) {
            animation->setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::SelectedHovered] : colors[(int)Colors::Selected]));
        } else {
            if (colorState & Used)
                animation->setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::UsedHovered] : colors[(int)Colors::Used]));
            else
                animation->setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::DefaultHovered] : colors[(int)Colors::Default]));
        }
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    {
        auto animation = new QPropertyAnimation(this, "pathColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(m_pathColor);
        animation->setEndValue(QColor((colorState & Tool)
                ? ((colorState & Used) ? QRgb(colors[(int)Colors::Tool] ^ App::settings().guiColor(GuiColors::Background).rgba()) : colors[(int)Colors::Default])
                : colors[(int)Colors::UnUsed]));
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void AbstractDrillPrGI::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

void AbstractDrillPrGI::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant AbstractDrillPrGI::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        if (value.toInt()) {
            colorState |= Selected;
        } else {
            colorState &= ~Selected;
        }
        changeColor();
    } else if (change == ItemVisibleChange) {
        auto animation = new QPropertyAnimation(this, "opacity");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    return QGraphicsItem::itemChange(change, value);
}
