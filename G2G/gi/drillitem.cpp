#include "drillitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <exfile.h>
#include <exvars.h>
#include <gcfile.h>
#include <graphicsview.h>

using namespace ClipperLib;

DrillItem::DrillItem(Excellon::Hole* hole, Excellon::File* file)
    : GraphicsItem(file)
    , m_hole(hole)
    , m_diameter(hole->state.currentToolDiameter())
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    create();
}

DrillItem::DrillItem(double diameter, GCode::File* file)
    : GraphicsItem(file)
    , m_diameter(diameter)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    create();
}

QRectF DrillItem::boundingRect() const { return m_rect; }

QPainterPath DrillItem::shape() const { return m_shape; }

void DrillItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    //    if (m_paths.first().isEmpty()) {
    //    if (m_penColor)
    //        m_pen.setColor(*m_penColor);
    //    if (m_brushColor)
    //        m_brush.setColor(*m_brushColor);
    painter->save();
    QBrush brush(m_brush);
    if (brush.style() != Qt::SolidPattern) {
        const double scale = GraphicsView::scaleFactor();
        brush.setMatrix(QMatrix().scale(scale, scale));
    }

    if (option->state & QStyle::State_Selected)
        brush.setColor(Qt::magenta);

    if (option->state & QStyle::State_MouseOver)
        brush.setColor(brush.color().darker(150));

    painter->setBrush(brush);
    painter->setPen(m_pen);
    painter->drawPath(m_shape);
    painter->restore();
}

int DrillItem::type() const { return GiDrill; }

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
    update(/*m_rect*/);
}

Paths DrillItem::paths() const
{
    Path path;
    if (m_hole) {
        if (m_hole->state.path.isEmpty())
            path = CirclePath(m_diameter * uScale, toIntPoint(m_hole->state.offsetedPos()));
        else
            return { toPath(m_hole->state.path.translated(m_hole->state.format->offsetPos)) };
    } else
        path = CirclePath(m_diameter * uScale, toIntPoint(pos()));
    ReversePath(path);
    return { path };
}

void DrillItem::updateHole()
{
    if (!m_hole)
        return;
    setToolTip(QObject::tr("Tool %1, Ø%2mm").arg(m_hole->state.tCode).arg(m_diameter));
    m_diameter = m_hole->state.currentToolDiameter();
    create();
    update(/*m_rect*/);
}

void DrillItem::create()
{
    m_shape= QPainterPath();
    if (!m_hole) {
        m_shape.addEllipse(QPointF(), m_diameter / 2, m_diameter / 2);
        m_rect = m_shape.boundingRect();
    } else if (m_hole->state.path.isEmpty()) {
        setToolTip(QObject::tr("Tool %1, Ø%2mm").arg(m_hole->state.tCode).arg(m_hole->state.currentToolDiameter()));
        m_shape.addEllipse(m_hole->state.offsetedPos(), m_diameter / 2, m_diameter / 2);
        m_rect = m_shape.boundingRect();
    } else {
        setToolTip(QObject::tr("Tool %1, Ø%2mm").arg(m_hole->state.tCode).arg(m_hole->state.currentToolDiameter()));
        Paths tmpPpath;
        ClipperOffset offset;
        offset.AddPath(toPath(m_hole->state.path.translated(m_hole->state.format->offsetPos)), jtRound, etOpenRound);
        offset.Execute(tmpPpath, m_diameter * 0.5 * uScale);
        for (Path& path : tmpPpath) {
            path.append(path.first());
            m_shape.addPolygon(toQPolygon(path));
        }
        m_rect = m_shape.boundingRect();
    }
}
