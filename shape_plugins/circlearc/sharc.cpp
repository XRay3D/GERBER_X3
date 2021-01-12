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
#include "sharc.h"
#include "scene.h"
#include "shhandler.h"
#include <QIcon>
#include <QtMath>

#include "leakdetector.h"

namespace Shapes {

Arc::Arc(QPointF center, QPointF pt, QPointF pt2)
    : m_radius(QLineF(center, pt).length())
{
    m_paths.resize(1);
    handlers = { new Handler(this, Handler::Center), new Handler(this), new Handler(this) };
    handlers[Center]->setPos(center);
    handlers[Point1]->setPos(pt);
    handlers[Point2]->setPos(pt2);

    redraw();

    App::scene()->addItem(this);
}

Arc::~Arc() { }

void Arc::redraw()
{
    const QLineF l1(handlers[Center]->pos(), handlers[Point1]->pos());
    const QLineF l2(handlers[Center]->pos(), handlers[Point2]->pos());

    m_radius = l1.length();

    const int intSteps = App::settings().clpCircleSegments(m_radius);
    const cInt radius = static_cast<cInt>(m_radius * uScale);
    const Point64 center((handlers[Center]->pos()));
    const double stepAngle = M_2PI / intSteps;

    double angle1 = M_2PI - qDegreesToRadians(l1.angle());
    double angle2 = M_2PI - qDegreesToRadians(l2.angle());

    if (qFuzzyCompare(angle1, M_2PI))
        angle1 = 0.0;
    double angle = angle2 - angle1;
    if (angle < 0.0)
        angle = M_2PI + angle;

    Path& path = m_paths.first();
    path.clear();
    path.reserve(intSteps);

    for (int i = 0; i < intSteps; i++) {
        const double theta = stepAngle * i;
        if (theta > angle) {
            path.append(Point64(
                static_cast<cInt>(radius * cos(angle2)) + center.X,
                static_cast<cInt>(radius * sin(angle2)) + center.Y));
            break;
        }
        path.append(Point64(
            static_cast<cInt>(radius * cos(angle1 + theta)) + center.X,
            static_cast<cInt>(radius * sin(angle1 + theta)) + center.Y));
    }

    m_shape = QPainterPath();
    m_shape.addPolygon(path);
    m_rect = m_shape.boundingRect();

    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QString Arc::name() const { return QObject::tr("Arc"); }

QIcon Arc::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

void Arc::updateOtherHandlers(Handler* handler)
{
    QLineF l(handlers[Center]->pos(), handler->pos());
    m_radius = l.length();

    QLineF l1(handlers[Center]->pos(),
        handlers[Center]->pos() == handlers[Point1]->pos() //если залипло на центр
            ? handlers[Center]->pos() + QPointF(1.0, 0.0)
            : handlers[Point1]->pos());
    QLineF l2(handlers[Center]->pos(),
        handlers[Center]->pos() == handlers[Point2]->pos() //если залипло на центр
            ? handlers[Center]->pos() + QPointF(1.0, 0.0)
            : handlers[Point2]->pos());

    switch (handlers.indexOf(handler)) {
    case Center:
        break;
    case Point1:
        l2.setLength(m_radius);
        handlers[Point2]->QGraphicsItem::setPos(l2.p2());
        break;
    case Point2:
        l1.setLength(m_radius);
        handlers[Point1]->QGraphicsItem::setPos(l1.p2());
        break;
    }
}

void Arc::setPt(const QPointF& pt)
{
    {
        handlers[Point1]->setPos(pt);
        QLineF l(handlers[Center]->pos(), handlers[Point1]->pos());
        m_radius = l.length();
    }
    {
        QLineF l(handlers[Center]->pos(), handlers[Point2]->pos());
        l.setLength(m_radius);
        handlers[Point2]->QGraphicsItem::setPos(l.p2());
    }
    redraw();
}

void Arc::setPt2(const QPointF& pt)
{
    QLineF l(handlers[Center]->pos(), pt);
    l.setLength(m_radius);

    handlers[Point2]->QGraphicsItem::setPos(l.p2());
    redraw();
}

double Arc::radius() const { return m_radius; }

void Arc::setRadius(double radius)
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

int Plugin::type() const { return static_cast<int>(GiType::ShCirArc); }

QJsonObject Plugin::info() const
{
    return QJsonObject {
        { "Name", "Circle Arc" },
        { "Version", "1.0" },
        { "VendorAuthor", "X-Ray aka Bakiev Damir" },
        { "Info", "Circle Arc" }
    };
}

QIcon Plugin::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

Shape* Plugin::createShape() { return shape = new Arc(); }

Shape* Plugin::createShape(const QPointF& point) { return shape = new Arc(point, point, point); }

bool Plugin::addShapePoint(const QPointF&) { return ctr++ ? ctr = 0, false : true; }

void Plugin::updateShape(const QPointF& point)
{
    if (shape) {
        if (!ctr)
            shape->setPt(point);
        else
            shape->setPt2(point);
    }
}

void Plugin::finalizeShape()
{
    shape = nullptr;
    emit actionUncheck();
}

}
