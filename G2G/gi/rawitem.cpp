#include "rawitem.h"

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
    if (m_penColor)
        m_pen.setColor(*m_penColor);
    if (m_brushColor)
        m_brush.setColor(*m_brushColor);

    QColor color(m_pen.color());
    QPen pen(m_pen);

    if (option->state & QStyle::State_Selected) {
        color.setAlpha(255);
        pen.setColor(color);
        pen.setWidthF(2.0 * GraphicsView::scaleFactor());
    }
    if (option->state & QStyle::State_MouseOver) {
        pen.setWidthF(2.0 * GraphicsView::scaleFactor());
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
    if (!qFuzzyCompare(m_scale, GraphicsView::scaleFactor())) {
        m_scale = GraphicsView::scaleFactor();
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
    event->ignore();
    if (event->modifiers() & Qt::ShiftModifier) {
        const double glueLen = GCodePropertiesForm::glue * uScale;
        IntPoint dest(m_path.last());
        IntPoint init(m_path.last());
        QList<int> skip;
        ItemGroup* ig = typedFile<Gerber::File>()->rawItemGroup();

        for (int i = 0; i < ig->size(); ++i) {
            if (skip.contains(i))
                continue;
            const Path path(ig->at(i)->paths().first());
            const IntPoint& first = path.first();
            const IntPoint& last = path.last();
            if (Length(dest, first) < glueLen) {
                dest = last;
                skip.append(i);
                ig->at(i)->setSelected(true);
                if (Length(init, dest) < glueLen)
                    break;
                i = -1;
            } else if (Length(dest, last) < glueLen) {
                dest = first;
                skip.append(i);
                ig->at(i)->setSelected(true);
                if (Length(init, dest) < glueLen)
                    break;
                i = -1;
            }
        }
        return;
    }
    GraphicsItem::mouseReleaseEvent(event);
    if (event->modifiers() & Qt::ShiftModifier)
        setSelected(true);
}

QVariant AperturePathItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemVisibleChange && !value.toBool()) {
        m_selectionShape = QPainterPath();
        m_scale = std::numeric_limits<double>::max();
    }
    return value;
}
