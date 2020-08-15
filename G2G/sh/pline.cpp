#include "pline.h"
#include "constructor.h"
#include "sh.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <graphicsview.h>
#include <math.h>
#include <scene.h>
#include <settings.h>

namespace ShapePr {
Pline::Pline(QPointF pt1, QPointF pt2)
{
    m_paths.resize(1);
    sh = { new SH(this /*, true*/), new SH(this) };
    sh.first()->setPos(pt1);
    sh.last()->setPos(pt2);

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());

    App::scene()->addItem(this);
    App::scene()->addItem(sh.first());
    App::scene()->addItem(sh.last());
}

void Pline::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
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
        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
    }
    if (option->state & QStyle::State_MouseOver) {
        pen.setColor(Qt::red);
        //        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
        //        pen.setStyle(Qt::CustomDashLine);
        //        pen.setCapStyle(Qt::FlatCap);
        //        pen.setDashPattern({ 3.0, 3.0 });
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_shape);
}

void Pline::redraw()
{
    Path& path = m_paths.first();
    path.resize(sh.size());
    for (int i = 0, e = sh.size(); i < e; ++i) {
        path[i] = toIntPoint(sh[i]->pos());
    }
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(path));
    m_rect = m_shape.boundingRect();
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

void Pline::setPt(const QPointF& pt)
{
    if (sh.last()->pos() == pt)
        return;
    sh.last()->setPos(pt);
    redraw();
}

void Pline::addPt(const QPointF& pt)
{
    sh.append(new SH(this));
    sh.last()->setPos(pt);
    App::scene()->addItem(sh.last());
    redraw();
}

//double Line::radius() const
//{
//    return m_radius;
//}

//void Line::setRadius(double radius)
//{
//    if (!qFuzzyCompare(m_radius, radius))
//        return;
//    m_radius = radius;
//    redraw();
//}
}
