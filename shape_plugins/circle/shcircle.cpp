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
#include "shcircle.h"
#include "math.h"
#include "scene.h"
#include "shhandler.h"
#include <QIcon>

#include "leakdetector.h"

namespace Shapes {

Circle::Circle(QPointF center, QPointF pt)
    : m_radius(QLineF(center, pt).length())
{
    m_paths.resize(1);

    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handler>(this, Handler::Center));
    handlers.emplace_back(std::make_unique<Handler>(this));

    handlers[Center]->setPos(center);
    handlers[Point1]->setPos(pt);

    redraw();

    App::scene()->addItem(this);
}

Circle::~Circle() { }

void Circle::redraw()
{
    m_radius = (QLineF(handlers[Center]->pos(), handlers[Point1]->pos()).length());
    const int intSteps = App::settings().clpCircleSegments(m_radius);
    const cInt radius = static_cast<cInt>(m_radius * uScale);
    const Point64 center((handlers[Center]->pos()));
    const double delta_angle = (2.0 * M_PI) / intSteps;
    Path& path = m_paths.first();
    path.clear();
    for (int i = 0; i < intSteps; i++) {
        const double theta = delta_angle * i;
        path.append(Point64(
            static_cast<cInt>(radius * cos(theta)) + center.X,
            static_cast<cInt>(radius * sin(theta)) + center.Y));
    }
    path.append(path.first());
    m_shape = QPainterPath();
    m_shape.addPolygon(path);
    m_rect = m_shape.boundingRect();
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QString Circle::name() const { return QObject::tr("Circle"); }

QIcon Circle::icon() const { return QIcon::fromTheme("draw-ellipse"); }

void Circle::setPt(const QPointF& pt)
{
    if (handlers[Point1]->pos() == pt)
        return;
    handlers[Point1]->setPos(pt);
    redraw();
}

double Circle::radius() const
{
    return m_radius;
}

void Circle::setRadius(double radius)
{
    if (!qFuzzyCompare(m_radius, radius))
        return;
    m_radius = radius;
    redraw();
}

////////////////////////////////////////////////////////////
/// \brief Plugin::Plugin
///
Plugin::Plugin() { }

Plugin::~Plugin() { }

QObject* Plugin::getObject() { return this; }

int Plugin::type() const { return static_cast<int>(GiType::ShCircle); }

QJsonObject Plugin::info() const
{
    return QJsonObject {
        { "Name", "Circle" },
        { "Version", "1.0" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "Circle" }
    };
}

QIcon Plugin::icon() const { return QIcon::fromTheme("draw-ellipse"); }

Shape* Plugin::createShape() { return new Circle(); }

Shape* Plugin::createShape(const QPointF& point)
{
    return shape = new Circle(point, point);
}

bool Plugin::addShapePoint(const QPointF&)
{
    return false;
}

void Plugin::updateShape(const QPointF& point)
{
    if (shape)
        shape->setPt(point);
}

void Plugin::finalizeShape()
{
    shape = nullptr;
    emit actionUncheck();
}

}
