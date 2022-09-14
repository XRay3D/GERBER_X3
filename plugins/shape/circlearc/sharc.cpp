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
#include "sharc.h"
#include "graphicsview.h"
#include "shhandler.h"
#include "utils.h"
#include <QIcon>
#include <QtMath>

namespace Shapes {

Arc::Arc(QPointF center, QPointF pt1, QPointF pt2)
    : radius_(QLineF(center, pt1).length()) {
    paths_.resize(1);

    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Center));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this));

    handlers[Center]->setPos(center);
    handlers[Point1]->setPos(pt1);
    handlers[Point2]->setPos(pt2);

    currentHandler = handlers[Point1].get();

    redraw();

    App::graphicsView()->scene()->addItem(this);
}

Arc::~Arc() { }

void Arc::redraw() {

    qDebug() << __FUNCTION__ << handlers.indexOf(currentHandler);

    if (currentHandler) {
        QLineF l(handlers[Center]->pos(), currentHandler->pos());
        radius_ = l.length();

        QLineF l1(handlers[Center]->pos(),
            handlers[Center]->pos() == handlers[Point1]->pos() //если залипло на центр
                ? handlers[Center]->pos() + QPointF(1.0, 0.0)
                : handlers[Point1]->pos());
        QLineF l2(handlers[Center]->pos(),
            handlers[Center]->pos() == handlers[Point2]->pos() //если залипло на центр
                ? handlers[Center]->pos() + QPointF(1.0, 0.0)
                : handlers[Point2]->pos());

        switch (handlers.indexOf(currentHandler)) {
        case Center:
            break;
        case Point1:
            l2.setLength(radius_);
            handlers[Point2]->QGraphicsItem::setPos(l2.p2());
            break;
        case Point2:
            l1.setLength(radius_);
            handlers[Point1]->QGraphicsItem::setPos(l1.p2());
            break;
        }
    }

    const QLineF l1(handlers[Center]->pos(), handlers[Point1]->pos());
    const QLineF l2(handlers[Center]->pos(), handlers[Point2]->pos());

    radius_ = l1.length();

    double angle1 = two_pi - qDegreesToRadians(l1.angle());
    double angle2 = two_pi - qDegreesToRadians(l2.angle());

    if (qFuzzyCompare(angle1, two_pi))
        angle1 = 0.0;
    double angle = angle2 - angle1;
    if (angle < 0.0)
        angle = two_pi + angle;

    int intSteps = App::settings().clpCircleSegments(radius_);
    const double stepAngle = two_pi / intSteps;
    const cInt radius = static_cast<cInt>(radius_ * uScale);
    const IntPoint center((handlers[Center]->pos()));
    intSteps *= angle / two_pi;

    Path& path = paths_.front();
    path.clear();
    path.reserve(intSteps);

    for (int i {}; i <= intSteps; i++) {
        const double angle = angle1 + stepAngle * i;
        path.emplace_back(
            static_cast<cInt>(radius * cos(angle)) + center.X,
            static_cast<cInt>(radius * sin(angle)) + center.Y);
    }

    path.back() = IntPoint {
        static_cast<cInt>(radius * cos(angle2)) + center.X,
        static_cast<cInt>(radius * sin(angle2)) + center.Y
    };

    shape_ = QPainterPath();
    shape_.addPolygon(path);

    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QString Arc::name() const { return QObject::tr("Arc"); }

QIcon Arc::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

bool Arc::addPt(const QPointF& pt) {
    switch (ptCtr++) {
    case 0:
        return (currentHandler = handlers[Point1].get())->setPos(pt), true;
    case 1:
        return (currentHandler = handlers[Point2].get())->setPos(pt), true;
    default:
        return false;
    }
}

void Arc::setPt(const QPointF& pt) {
    currentHandler->setPos(pt);
    redraw();
}

double Arc::radius() const { return radius_; }

void Arc::setRadius(double radius) {
    if (!qFuzzyCompare(radius_, radius))
        return;
    radius_ = radius;
    redraw();
}

////////////////////////////////////////////////////////////
/// \brief PluginImpl::PluginImpl
///

int PluginImpl::type() const { return GiType::ShCirArc; }

QIcon PluginImpl::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

Shape* PluginImpl::createShape(const QPointF& point) const { return new Arc(point, point, point); }

} // namespace Shapes

#include "moc_sharc.cpp"
