// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gerberitem.h"

#include "gbrfile.h"
#include "graphicsview.h"
#include "scene.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>

GiGerber::GiGerber(Paths& paths, Gerber::File* file)
    : GraphicsItem(file)
    , m_paths(paths)
{
    for (Path path : m_paths) {
        if (path.size())
            path.append(path.first());
        m_shape.addPolygon(path);
    }
    fillPolygon = m_shape.toFillPolygon();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

GiGerber::~GiGerber() { }

QRectF GiGerber::boundingRect() const { return m_shape.boundingRect(); }

QPainterPath GiGerber::shape() const { return m_shape; }

void GiGerber::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if (App::scene()->drawPdf()) {
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawPath(m_shape);
        return;
    }

    painter->setBrush(m_bodyColor);
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(fillPolygon);

    m_pen.setColor(m_pathColor);
    painter->strokePath(m_shape, m_pen);
}

int GiGerber::type() const { return static_cast<int>(GiType::Gerber); }

void GiGerber::redraw()
{
    m_shape = QPainterPath();
    for (Path path : m_paths) {
        path.append(path.first());
        m_shape.addPolygon(path);
    }
    fillPolygon = m_shape.toFillPolygon();
    setPos({ 1, 1 }); // костыли
    setPos({ 0, 0 });
    //update();
}

Paths GiGerber::paths() const { return m_paths; }

Paths* GiGerber::rPaths() { return &m_paths; }

void GiGerber::changeColor()
{
    //    auto animation = new QPropertyAnimation(this, "bodyColor");
    //    animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    //    animation.setDuration(100);
    animation.setStartValue(m_bodyColor);
    m_bodyColor = m_colorPtr ? *m_colorPtr : m_color;

    switch (colorState) {
    case Default:
        break;
    case Hovered:
        m_bodyColor.setAlpha(255);
        break;
    case Selected:
        m_bodyColor.setAlpha(255);
        break;
    case Hovered | Selected:
        m_bodyColor.setAlpha(255);
        m_bodyColor = m_bodyColor.lighter(150);
        break;
    }

    m_pathColor = m_colorPtr ? *m_colorPtr : m_color;
    switch (colorState) {
    case Default:
        m_pathColor.setAlpha(10);
        break;
    case Hovered:
        m_pathColor.setAlpha(255);
        m_pathColor = m_pathColor.darker(125);
        break;
    case Selected:
        m_pathColor.setAlpha(10);
        break;
    case Hovered | Selected:
        m_pathColor.setAlpha(255);
        m_pathColor = m_pathColor.lighter(150);
        break;
    }

    animation.setEndValue(m_bodyColor);
    animation.start();
}
