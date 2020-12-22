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
#include "drillitem.h"
#include "excellon.h"
#include "gcode.h"
#include "graphicsview.h"
#include "scene.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace ClipperLib;

DrillItem::DrillItem(Excellon::Hole* hole, Excellon::File* file)
    : GraphicsItem(file)
    , m_diameter(hole->state.currentToolDiameter())
    , m_hole(hole)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    create();
    changeColor();
}

DrillItem::DrillItem(double diameter, GCode::File* file)
    : GraphicsItem(file)
    , m_diameter(diameter)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    create();
    changeColor();
}

DrillItem::~DrillItem() { }

QRectF DrillItem::boundingRect() const { return m_rect; }

QPainterPath DrillItem::shape() const { return m_shape; }

void DrillItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{

    if (App::scene()->drawPdf()) {
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawPath(m_shape);
        return;
    }

    painter->setBrush(m_bodyColor);
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(m_shape.toFillPolygon());

    m_pen.setColor(m_pathColor);
    painter->strokePath(m_shape, m_pen);
}

int DrillItem::type() const { return static_cast<int>(GiType::Drill); }

bool DrillItem::isSlot()
{
    if (m_hole)
        return !m_hole->state.path.isEmpty();
    return false;
}

double DrillItem::diameter() const { return m_diameter; }

void DrillItem::setDiameter(double diameter)
{
    if (m_diameter == diameter)
        return;
    m_diameter = diameter;

    create();
    update();
}

Paths DrillItem::paths() const
{
    Path path;
    if (m_hole) {
        if (m_hole->state.path.isEmpty())
            path = CirclePath(m_diameter * uScale, (m_hole->state.offsetedPos()));
        else
            return { m_hole->state.path.translated(m_hole->state.format->offsetPos) };
    } else
        path = CirclePath(m_diameter * uScale, (pos()));
    ReversePath(path);
    return { path };
}

void DrillItem::changeColor()
{
    animation.setStartValue(m_bodyColor);

    switch (colorState) {
    case Default:
        m_bodyColor = QColor(100, 100, 100);
        break;
    case Hovered:
        m_bodyColor = QColor(150, 0x0, 150);
        break;
    case Selected:
        m_bodyColor = QColor(255, 0x0, 255);
        break;
    case Hovered | Selected:
        m_bodyColor = QColor(127, 0x0, 255);
        break;
    }

    m_pathColor = m_bodyColor;
    switch (colorState) {
    case Default:
        break;
    case Hovered:
        break;
    case Selected:
        m_pathColor = Qt::white;
        break;
    case Hovered | Selected:
        m_pathColor = Qt::white;
        break;
    }

    animation.setEndValue(m_bodyColor);
    animation.start();
}

void DrillItem::updateHole()
{
    if (!m_hole)
        return;
    setToolTip(QObject::tr("Tool %1, Ø%2mm").arg(m_hole->state.tCode).arg(m_diameter));
    m_diameter = m_hole->state.currentToolDiameter();
    create();
    update();
    qDebug() << pos();
    auto p(pos());
    setPos(1, 1);
    setPos(p);
}

void DrillItem::create()
{
    m_shape = QPainterPath();
    if (!m_hole) {
        //m_shape.addEllipse(QPointF(), m_diameter / 2, m_diameter / 2);
        auto p = CirclePath(m_diameter * uScale);
        p.append(p.first());
        m_shape.addPolygon(p);
        m_rect = m_shape.boundingRect();
    } else if (m_hole->state.path.isEmpty()) {
        setToolTip(QObject::tr("Tool %1, Ø%2mm").arg(m_hole->state.tCode).arg(m_hole->state.currentToolDiameter()));
        //m_shape.addEllipse(m_hole->state.offsetedPos(), m_diameter / 2, m_diameter / 2);
        auto p = CirclePath(m_diameter * uScale, (m_hole->state.offsetedPos()));
        p.append(p.first());
        m_shape.addPolygon(p);
        m_rect = m_shape.boundingRect();
    } else {
        setToolTip(QObject::tr("Tool %1, Ø%2mm").arg(m_hole->state.tCode).arg(m_hole->state.currentToolDiameter()));
        Paths tmpPpath;
        ClipperOffset offset;
        offset.AddPath(m_hole->state.path.translated(m_hole->state.format->offsetPos), jtRound, etOpenRound);
        offset.Execute(tmpPpath, m_diameter * 0.5 * uScale);
        for (Path& path : tmpPpath) {
            path.append(path.first());
            m_shape.addPolygon(path);
        }
        m_rect = m_shape.boundingRect();
        fillPolygon = m_shape.toFillPolygon();
    }
}
