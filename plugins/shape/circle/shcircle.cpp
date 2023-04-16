// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shcircle.h"
#include "graphicsview.h"
#include "math.h"
#include "shhandler.h"
#include <QIcon>

namespace Shapes {

Circle::Circle(QPointF center, QPointF pt)
    : radius_(QLineF(center, pt).length()) {
    paths_.resize(1);

    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Center));
    handlers.emplace_back(std::make_unique<Handle>(this));

    handlers[Center]->setPos(center);
    handlers[Point1]->setPos(pt);

    redraw();

    App::graphicsView().addItem(this);
}

void Circle::redraw() {
    radius_ = (QLineF(handlers[Center]->pos(), handlers[Point1]->pos()).length());
    const int intSteps = App::settings().clpCircleSegments(radius_);
    const Point::Type radius = static_cast<Point::Type>(radius_ * uScale);
    const Point center((handlers[Center]->pos()));
    const double delta_angle = (2.0 * pi) / intSteps;
    Path& path = paths_.front();
    path.clear();
    for(int i = 0; i <= intSteps; i++) {
        const double theta = delta_angle * i;
        path.emplace_back(Point(
            static_cast<Point::Type>(radius * cos(theta)) + center.x,
            static_cast<Point::Type>(radius * sin(theta)) + center.y));
    }
    shape_ = QPainterPath();
    shape_.addPolygon(path);
    setPos({1, 1}); // костыли    //update();
    setPos({0, 0});
}

QString Circle::name() const { return QObject::tr("Circle"); }

QIcon Circle::icon() const { return QIcon::fromTheme("draw-ellipse"); }

void Circle::setPt(const QPointF& pt) {
    handlers[Point1]->setPos(pt);
    redraw();
}

double Circle::radius() const { return radius_; }

void Circle::setRadius(double radius) {
    if(!qFuzzyCompare(radius_, radius))
        return;
    radius_ = radius;
    redraw();
}

////////////////////////////////////////////////////////////
/// \brief Plugin::Plugin
///

int PluginImpl::type() const { return GiType::ShCircle; }

QIcon PluginImpl::icon() const { return QIcon::fromTheme("draw-ellipse"); }

AbstractShape* PluginImpl::createShape(const QPointF& point) const { return new Circle(point, point + QPointF{5, 0}); }

} // namespace Shapes

#include "moc_shcircle.cpp"
