// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shrectangle.h"
#include "scene.h"
#include "shhandler.h"
#include <QIcon>

namespace Shapes {

Rectangle::Rectangle(QPointF pt1, QPointF pt2) {
    paths_.resize(1);
    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handler>(this, Handler::Center));
    handlers.emplace_back(std::make_unique<Handler>(this));
    handlers.emplace_back(std::make_unique<Handler>(this));
    handlers.emplace_back(std::make_unique<Handler>(this));
    handlers.emplace_back(std::make_unique<Handler>(this));

    handlers[Point1]->setPos(pt1);
    handlers[Point3]->setPos(pt2);

    redraw();

    App::scene()->addItem(this);
}

Rectangle::~Rectangle() { }

void Rectangle::redraw() {
    handlers[Center]->QGraphicsItem::setPos(QLineF(handlers[Point1]->pos(), handlers[Point3]->pos()).center());
    paths_.front() = {
        handlers[Point1]->pos(),
        handlers[Point2]->pos(),
        handlers[Point3]->pos(),
        handlers[Point4]->pos(),
        handlers[Point1]->pos(),
    };
    if (Area(paths_.front()) < 0)
        ReversePath(paths_.front());
    shape_ = QPainterPath();
    shape_.addPolygon(paths_.front());
    setPos({1, 1}); //костыли    //update();
    setPos({0, 0});
}

QString Rectangle::name() const { return QObject::tr("Rectangle"); }

QIcon Rectangle::icon() const { return QIcon::fromTheme("draw-rectangle"); }

void Rectangle::updateOtherHandlers(Handler* handler) {
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

void Rectangle::setPt(const QPointF& pt) {
    handlers[Point3]->setPos(pt);
    updateOtherHandlers(handlers[Point3].get());
    redraw();
}

////////////////////////////////////////////////////////////
/// \brief Plugin::Plugin
///
Plugin::Plugin() { }

Plugin::~Plugin() { }

int Plugin::type() const { return static_cast<int>(GiType::ShRectangle); }

QIcon Plugin::icon() const { return QIcon::fromTheme("draw-rectangle"); }

Shape* Plugin::createShape() { return shape = new Rectangle(); }

Shape* Plugin::createShape(const QPointF& point) { return shape = new Rectangle(point, point); }

bool Plugin::addShapePoint(const QPointF&) { return false; }

void Plugin::updateShape(const QPointF& point) {
    if (shape)
        shape->setPt(point);
}

void Plugin::finalizeShape() {
    if (shape)
        shape->finalize();
    shape = nullptr;
    emit actionUncheck();
}

} // namespace Shapes
