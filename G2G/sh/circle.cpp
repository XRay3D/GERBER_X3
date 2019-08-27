#include "circle.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <graphicsview.h>
#include <math.h>
#include <scene.h>
#include <settings.h>

namespace ShapePr {
Circle::Circle(QPointF center, QPointF rh)
    : GraphicsItem(nullptr)
    , m_rh(rh)
    , m_radius(QLineF(center, rh).length())
{
    setPos(center);
    redraw();
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());
    Scene::addItem(this);
}

QRectF Circle::boundingRect() const { return m_rect; }

QPainterPath Circle::shape() const { return m_shape; }

void Circle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (m_penColor)
        m_pen.setColor(*m_penColor);
    if (m_brushColor)
        m_brush.setColor(*m_brushColor);

    QColor color(m_pen.color());
    QPen pen(m_pen);

    if (option->state & QStyle::State_Selected) {
        color.setAlpha(255);
        pen.setColor(color);
        pen.setWidthF(2.0 * GraphicsView::scaleFactor());
    }
    if (option->state & QStyle::State_MouseOver) {
        pen.setColor(Qt::red);
        //        pen.setWidthF(2.0 * GraphicsView::scaleFactor());
        //        pen.setStyle(Qt::CustomDashLine);
        //        pen.setCapStyle(Qt::FlatCap);
        //        pen.setDashPattern({ 3.0, 3.0 });
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    //    painter->drawPolyline(toQPolygon(m_path));
    painter->drawPath(m_shape);

    if (option->state & QStyle::State_Selected) {
        const double scale = GraphicsView::scaleFactor();
        const double k = 5 * scale;
        const double s = k * 2;
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::red);
        painter->drawRect({ m_rh + QPointF(-k, -k), QSizeF(s, s) });
        painter->drawRect({ QPointF(-k, -k), QSizeF(s, s) });
    }
}

Paths Circle::paths() const
{
    Path p(m_path);
    TranslatePath(p, toIntPoint(pos()));
    return { p };
}

void Circle::redraw()
{
    m_radius = (QLineF(pos(), m_rh).length());
    const int intSteps = Settings::circleSegments(m_radius);
    const cInt radius = static_cast<cInt>(m_radius * uScale);
    const double delta_angle = (2.0 * M_PI) / intSteps;
    m_path.clear();
    for (int i = 0; i < intSteps; i++) {
        const double theta = delta_angle * i;
        m_path.append(IntPoint(
            static_cast<cInt>(radius * cos(theta)),
            static_cast<cInt>(radius * sin(theta))));
    }
    m_path.append(m_path.first());
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(m_path));
    m_rect = m_shape.boundingRect();
    update();
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
    redraw();
    QPointF p(pos());
    setPos(pos() * 0.99);
    setPos(p);
    m_rh -= pos();
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
    redraw();
}

QRectF Circle::handle()
{
    const double scale = GraphicsView::scaleFactor();
    const double k = 5 * scale;
    const double s = k * 2;
    return { QPointF(-k, -k), QSizeF(s, s) };
}
}

void ShapePr::Circle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    //    if (!handle().contains(event->pos())) {
    //        event->accept();
    //        return;
    //    }
    GraphicsItem::mouseMoveEvent(event);
    if (event->modifiers() & Qt::AltModifier) {
        const double gs = Settings::gridStep(GraphicsView::getScale());
        QPointF px(pos() / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        setPos(px);
    }
}

void ShapePr::Circle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mousePressEvent(event);
}

void ShapePr::Circle::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseReleaseEvent(event);
    scene()->setSceneRect(scene()->itemsBoundingRect());
}

void ShapePr::Circle::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseDoubleClickEvent(event);
    delete this;
}
