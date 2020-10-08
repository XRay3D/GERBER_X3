// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shrectangle.h"
#include "scene.h"
#include "shhandler.h"
#include <QIcon>

#include "leakdetector.h"

namespace Shapes {

Rectangle::Rectangle(QPointF pt1, QPointF pt2)
{
    m_paths.resize(1);
    handlers = { new Handler(this, Handler::Center), new Handler(this), new Handler(this), new Handler(this), new Handler(this) };
    handlers[Point1]->setPos(pt1);
    handlers[Point3]->setPos(pt2);

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    //    setZValue(std::numeric_limits<double>::max());
    App::scene()->addItem(this);
}

Rectangle::~Rectangle() { }

void Rectangle::redraw()
{
    handlers[Center]->setPos(QLineF(handlers[Point1]->pos(), handlers[Point3]->pos()).center());
    IntPoint p1((handlers[Point1]->pos()));
    IntPoint p2((handlers[Point3]->pos()));
    m_paths.first() = {
        IntPoint { p1.X, p1.Y },
        IntPoint { p2.X, p1.Y },
        IntPoint { p2.X, p2.Y },
        IntPoint { p1.X, p2.Y },
        IntPoint { p1.X, p1.Y },
    };
    if (Area(m_paths.first()) < 0)
        ReversePath(m_paths.first());
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(m_paths.first()));
    m_scale = std::numeric_limits<double>::max();
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QString Rectangle::name() const { return QObject::tr("Rectangle"); }

QIcon Rectangle::icon() const { return QIcon::fromTheme("draw-rectangle"); }

QPointF Rectangle::calcPos(Handler* sh)
{
    switch (handlers.indexOf(sh)) {
    case Center:
        return sh->pos();
    case Point1:
        handlers[Point2]->QGraphicsItem::setPos(handlers[Point3]->pos().x(), sh->pos().y());
        handlers[Point4]->QGraphicsItem::setPos(sh->pos().x(), handlers[Point3]->pos().y());
        break;
    case Point2:
        handlers[Point1]->QGraphicsItem::setPos(handlers[Point4]->pos().x(), sh->pos().y());
        handlers[Point3]->QGraphicsItem::setPos(sh->pos().x(), handlers[Point4]->pos().y());
        break;
    case Point3:
        handlers[Point2]->QGraphicsItem::setPos(handlers[Point1]->pos().x(), sh->pos().y());
        handlers[Point4]->QGraphicsItem::setPos(sh->pos().x(), handlers[Point1]->pos().y());
        break;
    case Point4:
        handlers[Point1]->QGraphicsItem::setPos(handlers[Point2]->pos().x(), sh->pos().y());
        handlers[Point3]->QGraphicsItem::setPos(sh->pos().x(), handlers[Point2]->pos().y());
        break;
    }
    return sh->pos();
}

void Rectangle::setPt(const QPointF& pt)
{
    handlers[Point3]->setPos(pt);
    calcPos(handlers[Point3]);
    redraw();
}
}
