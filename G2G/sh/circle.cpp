#include "circle.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <settings.h>
namespace Shape {
Circle::Circle(QPointF center, QPointF rh)
    : GraphicsItem(nullptr)
    , m_rh(rh)
    , m_radius(QLineF(center, rh).length())
{
    setPos(center);
    calc();
}

QRectF Circle::boundingRect() const
{
    return m_rect;
}

QPainterPath Circle::shape() const
{
    return m_shape;
}

void Circle::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::white, 0.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_shape);
}

Paths Circle::paths() const
{
    return { m_path };
}

void Circle::calc()
{
    Path points;
    const int intSteps = Settings::circleSegments(m_radius); //MinStepsPerCircle;
    const cInt radius = static_cast<cInt>(m_radius * uScale);
    double angle = 2.0 * M_PI; // 360.0
    //    double steps = qMax(static_cast<int>(ceil(angle / (2.0 * M_PI) * intSteps)), 2);
    double delta_angle = angle / intSteps; // * 1.0 / steps;
    for (int i = 0; i <= intSteps /*steps*/; i++) {
        double theta = delta_angle * i;
        m_path.append(IntPoint(
            static_cast<cInt>(/*center.X +*/ radius * cos(theta)),
            static_cast<cInt>(/*center.Y +*/ radius * sin(theta))));
    }
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(m_path));
    m_rect = m_shape.boundingRect();
}

QPointF Circle::rh() const
{
    return m_rh;
}

void Circle::setRh(const QPointF& rh)
{
    if (m_rh == rh)
        return;
    m_rh = rh;
    calc();
}

QPointF Circle::center() const
{
    return pos();
}

void Circle::setCenter(const QPointF& center)
{
    if (pos() == center)
        return;
    setPos(center);
    //    calc();
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
    calc();
}
}

void Shape::Circle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseMoveEvent(event);
}

void Shape::Circle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mousePressEvent(event);
}

void Shape::Circle::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseReleaseEvent(event);
    scene()->setSceneRect(scene()->itemsBoundingRect());
}

void Shape::Circle::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseDoubleClickEvent(event);
}
