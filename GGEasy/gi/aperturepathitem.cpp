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
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

AperturePathItem::AperturePathItem(const Path& path, /*Gerber::File*/AbstractFile* file)
    : GraphicsItem(file)
    , m_path(path)
{
    m_polygon = path;

    Paths tmpPaths;
    ClipperOffset offset;
    offset.AddPath(path, jtSquare, etOpenButt);
    offset.Execute(tmpPaths, 0.01 * uScale);
    for (const Path& tmpPath : tmpPaths)
        m_selectionShape.addPolygon(tmpPath);
    boundingRect_m = m_selectionShape.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

QRectF AperturePathItem::boundingRect() const { return shape().boundingRect(); }

QRectF AperturePathItem::boundingRect2() const { return boundingRect_m; }

void AperturePathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_colorPtr)
        m_color = *m_colorPtr;

    QColor color(m_pen.color());
    QPen pen(m_pen);

    if (option->state & QStyle::State_Selected) {
        color.setAlpha(255);
        pen.setColor(color);
        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
    }
    if (option->state & QStyle::State_MouseOver) {
        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ 3.0, 3.0 });
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolyline(m_path);
}

int AperturePathItem::type() const { return static_cast<int>(GiType::AperturePath); }

Paths AperturePathItem::paths() const { return { m_path }; }

QPainterPath AperturePathItem::shape() const
{
    if (!qFuzzyCompare(m_scale, App::graphicsView()->scaleFactor())) {
        m_scale = App::graphicsView()->scaleFactor();
        m_selectionShape = QPainterPath();
        ClipperOffset offset;
        Paths tmpPpath;
        offset.AddPath(m_path, jtSquare, etOpenButt);
        offset.Execute(tmpPpath, 5 * uScale * m_scale);
        for (const Path& path : tmpPpath)
            m_selectionShape.addPolygon(path);
    }
    return m_selectionShape;
}

void AperturePathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        setSelected(true);
        const double glueLen = GCodePropertiesForm::glue * uScale;
        Point64 dest(m_path.last());
        Point64 init(m_path.first());
        ItemGroup* ig = typedFile<Gerber::File>()->itemGroup(Gerber::File::ApPaths);
        for (int i = 0; i < ig->size(); ++i) {
            auto item = ig->at(i);
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
    if (change == ItemVisibleChange && !value.toBool()) {
        m_selectionShape = QPainterPath();
        m_scale = std::numeric_limits<double>::max();
    }
    return value;
}
