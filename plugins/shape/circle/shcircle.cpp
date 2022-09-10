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
#include "shcircle.h"
#include "math.h"
#include "scene.h"
#include "shhandler.h"
#include <QIcon>

namespace Shapes {

Circle::Circle(QPointF center, QPointF pt)
    : radius_(QLineF(center, pt).length()) {
    paths_.resize(1);

    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handler>(this, Handler::Center));
    handlers.emplace_back(std::make_unique<Handler>(this));

    handlers[Center]->setPos(center);
    handlers[Point1]->setPos(pt);

    redraw();

    App::scene()->addItem(this);
}

Circle::~Circle() { }

void Circle::redraw() {
    radius_ = (QLineF(handlers[Center]->pos(), handlers[Point1]->pos()).length());
    const int intSteps = App::settings().clpCircleSegments(radius_);
    const cInt radius = static_cast<cInt>(radius_ * uScale);
    const IntPoint center((handlers[Center]->pos()));
    const double delta_angle = (2.0 * pi) / intSteps;
    Path& path = paths_.front();
    path.clear();
    for (int i = 0; i < intSteps; i++) {
        const double theta = delta_angle * i;
        path.emplace_back(IntPoint(
            static_cast<cInt>(radius * cos(theta)) + center.X,
            static_cast<cInt>(radius * sin(theta)) + center.Y));
    }
    path.push_back(path.front());
    shape_ = QPainterPath();
    shape_.addPolygon(path);
    setPos({1, 1}); //костыли    //update();
    setPos({0, 0});
}

QString Circle::name() const { return QObject::tr("Circle"); }

QIcon Circle::icon() const { return QIcon::fromTheme("draw-ellipse"); }

void Circle::setPt(const QPointF& pt) {
    if (handlers[Point1]->pos() == pt)
        return;
    handlers[Point1]->setPos(pt);
    redraw();
}

double Circle::radius() const { return radius_; }

void Circle::setRadius(double radius) {
    if (!qFuzzyCompare(radius_, radius))
        return;
    radius_ = radius;
    redraw();
}

////////////////////////////////////////////////////////////
/// \brief Plugin::Plugin
///
Plugin::Plugin() { }

Plugin::~Plugin() { }

int Plugin::type() const { return GiType::ShCircle; }

QIcon Plugin::icon() const { return QIcon::fromTheme("draw-ellipse"); }

Shape* Plugin::createShape() { return new Circle(); }

Shape* Plugin::createShape(const QPointF& point) {
    return shape = new Circle(point, point);
}

bool Plugin::addShapePoint(const QPointF&) {
    return false;
}

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
