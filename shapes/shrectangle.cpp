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
#include "shrectangle.h"
#include "scene.h"
#include "shhandler.h"
#include <QIcon>

#include "leakdetector.h"

namespace Shapes {

Rectangle::Rectangle(QPointF pt1, QPointF pt2)
{
    m_paths.resize(1);
    handlers = {
        new Handler(this, Handler::Center),
        new Handler(this),
        new Handler(this),
        new Handler(this),
        new Handler(this),
        //        new Handler(this, Handler::Adder),
        //        new Handler(this, Handler::Adder),
        //        new Handler(this, Handler::Adder),
        //        new Handler(this, Handler::Adder)
    };

    handlers[Point1]->setPos(pt1);
    handlers[Point3]->setPos(pt2);

    redraw();

    App::scene()->addItem(this);
}

Rectangle::~Rectangle() { }

void Rectangle::redraw()
{
    handlers[Center]->QGraphicsItem::setPos(QLineF(handlers[Point1]->pos(), handlers[Point3]->pos()).center());
    Point64 p1(handlers[Point1]->pos());
    Point64 p2(handlers[Point3]->pos());

    m_paths.first() = {
        Point64 { p1.X, p1.Y },
        Point64 { p2.X, p1.Y },
        Point64 { p2.X, p2.Y },
        Point64 { p1.X, p2.Y },
        Point64 { p1.X, p1.Y },
    };
    if (Area(m_paths.first()) < 0)
        ReversePath(m_paths.first());
    m_shape = QPainterPath();
    m_shape.addPolygon(m_paths.first());
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QString Rectangle::name() const { return QObject::tr("Rectangle"); }

QIcon Rectangle::icon() const { return QIcon::fromTheme("draw-rectangle"); }

void Rectangle::updateOtherHandlers(Handler* handler)
{
    switch (handlers.indexOf(handler)) {
    case Center:
        return;
    case Point1:
        handlers[Point2]->QGraphicsItem::setPos(handlers[Point3]->pos().x(), handler->pos().y());
        handlers[Point4]->QGraphicsItem::setPos(handler->pos().x(), handlers[Point3]->pos().y());
        break;
    case Point2:
        handlers[Point1]->QGraphicsItem::setPos(handlers[Point4]->pos().x(), handler->pos().y());
        handlers[Point3]->QGraphicsItem::setPos(handler->pos().x(), handlers[Point4]->pos().y());
        break;
    case Point3:
        handlers[Point2]->QGraphicsItem::setPos(handlers[Point1]->pos().x(), handler->pos().y());
        handlers[Point4]->QGraphicsItem::setPos(handler->pos().x(), handlers[Point1]->pos().y());
        break;
    case Point4:
        handlers[Point1]->QGraphicsItem::setPos(handlers[Point2]->pos().x(), handler->pos().y());
        handlers[Point3]->QGraphicsItem::setPos(handler->pos().x(), handlers[Point2]->pos().y());
        break;
    }
}

void Rectangle::setPt(const QPointF& pt)
{
    handlers[Point3]->setPos(pt);
    updateOtherHandlers(handlers[Point3]);
    redraw();
}
}
