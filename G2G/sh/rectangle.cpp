#include "rectangle.h"
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
Rectangle::Rectangle(QPointF pt1, QPointF pt2)
{
    m_paths.resize(1);
    sh = { new SH(this, true), new SH(this), new SH(this) };
    sh[Point1]->setPos(pt1);
    sh[Point2]->setPos(pt2);

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());

    App::scene()->addItem(this);
    App::scene()->addItem(sh[Center]);
    App::scene()->addItem(sh[Point1]);
    App::scene()->addItem(sh[Point2]);
}

void Rectangle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
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

void Rectangle::redraw()
{
    sh[Center]->setPos(QLineF(sh[Point1]->pos(), sh[Point2]->pos()).center());
    IntPoint p1(toIntPoint(sh[Point1]->pos()));
    IntPoint p2(toIntPoint(sh[Point2]->pos()));
    m_paths.first() = {
        IntPoint{ p1.X, p1.Y },
        IntPoint{ p2.X, p1.Y },
        IntPoint{ p2.X, p2.Y },
        IntPoint{ p1.X, p2.Y },
        IntPoint{ p1.X, p1.Y },
    };
    if (Area(m_paths.first()) < 0)
        ReversePath(m_paths.first());
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(m_paths.first()));
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

void Rectangle::setPt(const QPointF& pt)
{
    if (sh[Point2]->pos() == pt)
        return;
    sh[Point2]->setPos(pt);
    redraw();
}
}
