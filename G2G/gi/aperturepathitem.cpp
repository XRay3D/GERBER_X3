// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "aperturepathitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <gbrfile.h>
#include <graphicsview.h>
#include <myclipper.h>

#include "forms/gcodepropertiesform.h"

AperturePathItem::AperturePathItem(const Path& path, Gerber::File* file)
    : GraphicsItem(file)
    , m_path(path)
{
    m_polygon = toQPolygon(path);

    Paths tmpPpath;
    ClipperOffset offset;
    offset.AddPath(path, jtSquare, etOpenButt);
    offset.Execute(tmpPpath, 0.01 * uScale);
    for (const Path& path : tmpPpath)
        m_selectionShape.addPolygon(toQPolygon(path));
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

QRectF AperturePathItem::boundingRect() const { return shape().boundingRect(); }

void AperturePathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_brColorPtr)
        m_brush.setColor(*m_brColorPtr);

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
    painter->drawPolyline(toQPolygon(m_path));
}

int AperturePathItem::type() const { return GiAperturePath; }

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
            m_selectionShape.addPolygon(toQPolygon(path));
    }
    return m_selectionShape;
}

void AperturePathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        setSelected(true);
        const double glueLen = GCodePropertiesForm::glue * uScale;
        IntPoint dest(m_path.last());
        IntPoint init(m_path.first());
        ItemGroup* ig = typedFile<Gerber::File>()->itemGroup(Gerber::File::ApPaths);
        for (int i = 0; i < ig->size(); ++i) {
            auto item = ig->at(i);
            if (item->isSelected())
                continue;
            const IntPoint& first = item->paths().first().first();
            const IntPoint& last = item->paths().first().last();
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
