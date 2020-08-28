// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "arc.h"
#include "shandler.h"
#include <QtMath>
#include <scene.h>

namespace Shapes {
Arc::Arc(QPointF center, QPointF pt, QPointF pt2)
    : m_radius(QLineF(center, pt).length())
{
    m_paths.resize(1);
    sh = { new Handler(this, true), new Handler(this), new Handler(this) };
    sh[Center]->setPos(center);
    sh[Point1]->setPos(pt);
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


Arc::~Arc() { }

void Arc::redraw()
{
    const QLineF l1(sh[Center]->pos(), sh[Point1]->pos());
    const QLineF l2(sh[Center]->pos(), sh[Point2]->pos());

    m_radius = l1.length();

    const int intSteps = GlobalSettings::gbrGcCircleSegments(m_radius);
    const cInt radius = static_cast<cInt>(m_radius * uScale);
    const IntPoint center(toIntPoint(sh[Center]->pos()));
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
            path.append(IntPoint(
                static_cast<cInt>(radius * cos(angle2)) + center.X,
                static_cast<cInt>(radius * sin(angle2)) + center.Y));
            break;
        }
        path.append(IntPoint(
            static_cast<cInt>(radius * cos(angle1 + theta)) + center.X,
            static_cast<cInt>(radius * sin(angle1 + theta)) + center.Y));
    }

    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(path));
    m_rect = m_shape.boundingRect();

    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QPointF Arc::calcPos(Handler* sh_) const
{
    QLineF l(sh[Center]->pos(), sh_->pos());
    m_radius = l.length();

    QLineF l1(sh[Center]->pos(),
        sh[Center]->pos() == sh[Point1]->pos() //если залипло на центр
            ? sh[Center]->pos() + QPointF(1.0, 0.0)
            : sh[Point1]->pos());
    QLineF l2(sh[Center]->pos(),
        sh[Center]->pos() == sh[Point2]->pos() //если залипло на центр
            ? sh[Center]->pos() + QPointF(1.0, 0.0)
            : sh[Point2]->pos());

    switch (sh.indexOf(sh_)) {
    case Center:
        break;
    case Point1:
        l2.setLength(m_radius);
        sh[Point2]->QGraphicsItem::setPos(l2.p2());
        break;
    case Point2:
        l1.setLength(m_radius);
        sh[Point1]->QGraphicsItem::setPos(l1.p2());
        break;
    }
    return sh_->pos();
}

void Arc::setPt(const QPointF& pt)
{
    {
        sh[Point1]->setPos(pt);
        QLineF l(sh[Center]->pos(), sh[Point1]->pos());
        m_radius = l.length();
    }
    {
        QLineF l(sh[Center]->pos(), sh[Point2]->pos());
        l.setLength(m_radius);
        sh[Point2]->QGraphicsItem::setPos(l.p2());
    }
    redraw();
}

void Arc::setPt2(const QPointF& pt)
{
    QLineF l(sh[Center]->pos(), pt);
    l.setLength(m_radius);

    sh[Point2]->QGraphicsItem::setPos(l.p2());
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
}
