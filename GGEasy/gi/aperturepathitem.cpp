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
#include "aperturepathitem.h"

#include "forms/gcodepropertiesform.h"
#include "gbrfile.h"
#include "graphicsview.h"
#include "settings.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#ifdef __GNUC__
QTimer AperturePathItem::timer;
#endif

AperturePathItem::AperturePathItem(const Path& path, /*Gerber::File*/ AbstractFile* file)
    : GraphicsItem(file)
    , m_path(path)
{
    m_polygon = path;

    Paths tmpPaths;
    ClipperOffset offset;
    offset.AddPath(path, jtSquare, etOpenButt);
    offset.Execute(tmpPaths, 0.01 * uScale);
    for (const Path& tmpPath : qAsConst(tmpPaths))
        m_selectionShape.addPolygon(tmpPath);
    m_boundingRect = m_selectionShape.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setSelected(false);
    if (!timer.isActive()) {
        timer.start(50);
        connect(&timer, &QTimer ::timeout, [] { ++AperturePathItem::d; });
    }
}

QRectF AperturePathItem::boundingRect() const
{
    if (m_selectionShape.boundingRect().isEmpty())
        updateSelection();
    return m_selectionShape.boundingRect();
}

QRectF AperturePathItem::boundingRect2() const { return m_boundingRect; }

void AperturePathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_colorPtr)
        m_color = *m_colorPtr;

    QColor color(m_pen.color());
    QPen pen(m_pen);
    constexpr double dl = 3;
    if (option->state & QStyle::State_Selected) {
        color.setAlpha(255);
        pen.setColor(color);
        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashOffset(d);
        pen.setDashPattern({ dl, dl });
    }
    if (option->state & QStyle::State_MouseOver) {
        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ dl, dl });
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(m_path);
}

int AperturePathItem::type() const { return static_cast<int>(GiType::AperturePath); }

Paths AperturePathItem::paths() const { return { m_path }; }

QPainterPath AperturePathItem::shape() const { return m_selectionShape; }

void AperturePathItem::updateSelection() const
{
    const double scale = App::graphicsView()->scaleFactor();
    if (m_selectionShape.boundingRect().isEmpty() || !qFuzzyCompare(m_scale, scale)) {
        m_scale = scale;
        m_selectionShape = QPainterPath();
        Paths tmpPpath;
        ClipperOffset offset;
        offset.AddPath(m_path, jtSquare, etOpenButt);
        offset.Execute(tmpPpath, 5 * uScale * m_scale);
        for (const Path& path : qAsConst(tmpPpath))
            m_selectionShape.addPolygon(path);
    }
}

void AperturePathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier && itemGroup) {
        setSelected(true);
        const double glueLen = GCodePropertiesForm::glue * uScale;
        Point64 dest(m_path.last());
        Point64 init(m_path.first());
        for (int i = 0; i < itemGroup->size(); ++i) {
            auto item = itemGroup->at(i);
            if (item->isSelected())
                continue;
            const Point64& first = item->paths().first().first();
            const Point64& last = item->paths().first().last();
            if (Length(dest, first) < glueLen) {
                dest = last;
                item->setSelected(true);
                if (Length(init, dest) < glueLen)
                    break;
                i = -1;
            } else if (Length(dest, last) < glueLen) {
                dest = first;
                item->setSelected(true);
                if (Length(init, dest) < glueLen)
                    break;
                i = -1;
            }
        }
        event->accept();
        return;
    }
    GraphicsItem::mouseReleaseEvent(event);
}

QVariant AperturePathItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemVisibleChange && value.toBool()) {
        updateSelection();
    } else if (change == ItemSelectedChange && GlobalSettings::animSelection()) {
        if (value.toBool()) {
            updateSelection();
            connect(&timer, &QTimer::timeout, this, &AperturePathItem::redraw);
        } else {
            disconnect(&timer, &QTimer::timeout, this, &AperturePathItem::redraw);
            update();
        }
    }
    return value;
}

void AperturePathItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsItem::hoverEnterEvent(event);
    updateSelection();
}
