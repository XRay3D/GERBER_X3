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
#include "datapathitem.h"

#include "graphicsview.h"
#include "interfaces/file.h"
#include "project.h"
#include "scene.h"
#include "settings.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#ifdef __GNUC__
QTimer GiDataPath::timer;
#endif

GiDataPath::GiDataPath(const Path& path, FileInterface* file)
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
        //timer.start(50);
        connect(&timer, &QTimer ::timeout, [] { ++GiDataPath::d; });
    }
}

QRectF GiDataPath::boundingRect() const
{
    if (App::scene())
        return m_boundingRect;
    if (m_selectionShape.boundingRect().isEmpty())
        updateSelection();
    return m_selectionShape.boundingRect();
}

void GiDataPath::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_colorPtr)
        m_color = *m_colorPtr;

    QColor color(m_pen.color());
    QPen pen(m_pen);
    constexpr double dl = 3;
    //    if (!App::scene()->drawPdf())
    //        pen.setWidthF(2.0 * scaleFactor());
    if (option->state & QStyle::State_Selected) {
        color.setAlpha(255);
        pen.setColor(color);
        pen.setWidthF(2.0 * scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashOffset(d);
        pen.setDashPattern({ dl, dl - 1 });
    }
    if (option->state & QStyle::State_MouseOver) {
        pen.setWidthF(2.0 * scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ dl, dl - 1 });
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(m_path);
}

int GiDataPath::type() const { return static_cast<int>(GiType::DataPath); }

Paths GiDataPath::paths() const { return { m_path }; }

QPainterPath GiDataPath::shape() const { return m_selectionShape; }

void GiDataPath::updateSelection() const
{
    const double scale = scaleFactor();
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

void GiDataPath::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier && itemGroup) {
        setSelected(true);
        const double glueLen = App::project()->glue() * uScale;
        Point64 dest(m_path.back());
        Point64 init(m_path.front());
        for (int i = 0; i < itemGroup->size(); ++i) {
            auto item = itemGroup->at(i);
            if (item->isSelected())
                continue;
            const Point64& first = item->paths().front().front();
            const Point64& last = item->paths().front().back();
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

QVariant GiDataPath::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemVisibleChange && value.toBool()) {
        updateSelection();
    } else if (change == ItemSelectedChange && App::settings().animSelection()) {
        if (value.toBool()) {
            updateSelection();
            connect(&timer, &QTimer::timeout, this, &GiDataPath::redraw);
        } else {
            disconnect(&timer, &QTimer::timeout, this, &GiDataPath::redraw);
            update();
        }
    }
    return value;
}

void GiDataPath::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsItem::hoverEnterEvent(event);
    updateSelection();
}
