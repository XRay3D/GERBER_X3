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
#include "graphicsview.h"
#include "shhandler.h"
#include <QIcon>
#include <ranges>

namespace Shapes {

Rectangle::Rectangle(QPointF pt1, QPointF pt2) {
    paths_.resize(1);
    paths_.front().resize(5);
    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Center));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this));

    handlers[Point1]->setPos(pt1);
    handlers[Point3]->setPos(pt2);
    currentHandler = handlers[Point1].get();
    redraw();

    App::graphicsView()->scene()->addItem(this);
}

void Rectangle::redraw() {

    auto updCenter = [this] {
        handlers[Center]->setPos(QLineF(
            handlers[Point1]->pos(),
            handlers[Point3]->pos())
                                     .center());
    };
    auto updCorner = [this](int src, int p1, int p2) {
        handlers[p1]->setPos(handlers[src]->x(), currentHandler->y());
        handlers[p2]->setPos(currentHandler->x(), handlers[src]->y());
    };

    switch (handlers.indexOf(currentHandler)) {
    case Center: {
        QRectF rect(handlers[Point1]->pos(), handlers[Point3]->pos());
        rect.moveCenter(handlers[Center]->pos());
        auto center {handlers[Center]->pos()};
        handlers[Point1]->setPos(rect.topLeft());
        handlers[Point2]->setPos(rect.topRight());
        handlers[Point3]->setPos(rect.bottomRight());
        handlers[Point4]->setPos(rect.bottomLeft());
    } break;
    case Point1:
        updCorner(Point3, Point2, Point4), updCenter();
        break;
    case Point2:
        updCorner(Point4, Point1, Point3), updCenter();
        break;
    case Point3:
        updCorner(Point1, Point2, Point4), updCenter();
        break;
    case Point4:
        updCorner(Point2, Point1, Point3), updCenter();
        break;
    }

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

void Rectangle::setPt(const QPointF& pt) {
    handlers[Point3]->setPos(pt);
    updateOtherHandlers(handlers[Point3].get());
}

////////////////////////////////////////////////////////////
/// \brief PluginImpl::PluginImpl
///

int PluginImpl::type() const { return GiType::ShRectangle; }

QIcon PluginImpl::icon() const { return QIcon::fromTheme("draw-rectangle"); }

Shape* PluginImpl::createShape(const QPointF& point) const { return new Rectangle(point, point + QPointF {10, 10}); }

} // namespace Shapes

#include "moc_shrectangle.cpp"
