// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "arc.h"
#include "constructor.h"
#include "sh.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>
#include <math.h>
#include <scene.h>
#include <settings.h>

namespace Shapes {
Arc::Arc(QPointF center, QPointF pt, QPointF pt2)
    : m_radius(QLineF(center, pt).length())
{
    m_paths.resize(1);
    sh = { new SH(this, true), new SH(this), new SH(this) };
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

Arc::Arc(QDataStream& stream)
    : Shape(stream)
{
}

Arc::~Arc() { qDebug(Q_FUNC_INFO); }

void Arc::redraw()
{
    m_radius = QLineF(sh[Center]->pos(), sh[Point1]->pos()).length();
    const int intSteps = GlobalSettings::gbrGcCircleSegments(m_radius);
    const cInt radius = static_cast<cInt>(m_radius * uScale);
    const IntPoint center(toIntPoint(sh[Center]->pos()));
    const double stepAngle = (2.0 * M_PI) / intSteps;

    double angle1 = 360.0 - QLineF(sh[Center]->pos(), sh[Point1]->pos()).angle();
    double angle2 = 360.0 - QLineF(sh[Center]->pos(), sh[Point2]->pos()).angle();

    if (angle1 == 360)
        angle1 = 0;
    double angle = angle2 - angle1;
    if (angle < 0)
        angle = 360.0 + angle;
    qDebug() << "\nA1" << angle1 << "\nA2" << angle2 << "\nA3" << angle;
    angle = qDegreesToRadians(angle);

    Path& path = m_paths.first();
    path.clear();
    path.reserve(intSteps);

    for (int i = 0; i < intSteps; i++) {
        const double theta = stepAngle * i;
        if (theta > angle)
            break;
        path.append(IntPoint(
            static_cast<cInt>(radius * cos(qDegreesToRadians(angle1) + theta)) + center.X,
            static_cast<cInt>(radius * sin(qDegreesToRadians(angle1) + theta)) + center.Y));
    }

    //    path.first() = IntPoint(
    //        static_cast<cInt>(radius * cos(qDegreesToRadians(angle1))) + center.X,
    //        static_cast<cInt>(radius * sin(qDegreesToRadians(angle1))) + center.Y);

    path.last() = IntPoint(
        static_cast<cInt>(radius * cos(qDegreesToRadians(angle2))) + center.X,
        static_cast<cInt>(radius * sin(qDegreesToRadians(angle2))) + center.Y);

    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(path));
    m_rect = m_shape.boundingRect();

    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QPointF Arc::calcPos(SH* sh_) const
{
    qDebug() << Q_FUNC_INFO;
    QLineF l(sh[Center]->pos(), sh_->pos());
    m_radius = l.length();

    QLineF l1(sh[Center]->pos(), sh[Point1]->pos());
    QLineF l2(sh[Center]->pos(), sh[Point2]->pos());

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
    if (sh[Point1]->pos() == pt)
        return;
    sh[Point1]->setPos(pt);
    {
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

    if (sh[Point2]->pos() == l.p2())
        return;

    sh[Point2]->setPos(l.p2());
    redraw();
    sh[Point2]->QGraphicsItem::setPos(l.p2());
}

double Arc::radius() const
{
    return m_radius;
}

void Arc::setRadius(double radius)
{
    if (!qFuzzyCompare(m_radius, radius))
        return;
    m_radius = radius;
    redraw();
}
}
