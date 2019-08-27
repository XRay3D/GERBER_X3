#include "rectangle.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <graphicsview.h>
#include <math.h>
#include <scene.h>
#include <settings.h>

namespace ShapePr {
Rectangle::Rectangle(QPointF center, QPointF rh)
    : GraphicsItem(nullptr)
    , m_rh(rh)
{
    setPos(center);
    redraw();
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());
    Scene::addItem(this);
}

QRectF Rectangle::boundingRect() const { return m_rect; }

QPainterPath Rectangle::shape() const { return m_shape; }

void Rectangle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
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
    painter->drawPolyline(toQPolygon(m_path));
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

Paths Rectangle::paths() const
{
    Path p(m_path);
    TranslatePath(p, toIntPoint(pos()));
    return { p };
}

void Rectangle::redraw()
{
    IntPoint p1;
    IntPoint p2(toIntPoint(m_rh - pos()));
    m_path = {
        IntPoint{ p1.X, p1.Y },
        IntPoint{ p2.X, p1.Y },
        IntPoint{ p2.X, p2.Y },
        IntPoint{ p1.X, p2.Y },
        IntPoint{ p1.X, p1.Y },
    };
    if (Area(m_path) < 0)
        ReversePath(m_path);
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(m_path));
    m_rect = m_shape.boundingRect();
    update();
}

QPointF Rectangle::rh() const
{
    return m_rh;
}

void Rectangle::setRh(const QPointF& rh)
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

QPointF Rectangle::center() const
{
    return pos();
}

void Rectangle::setCenter(const QPointF& center)
{
    if (pos() == center)
        return;
    setPos(center);
}
}

void ShapePr::Rectangle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseMoveEvent(event);
    if (event->modifiers() & Qt::AltModifier) {
        const double gs = Settings::gridStep(GraphicsView::getScale());
        QPointF px(pos() / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        setPos(px);
    }
}

void ShapePr::Rectangle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mousePressEvent(event);
}

void ShapePr::Rectangle::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseReleaseEvent(event);
    scene()->setSceneRect(scene()->itemsBoundingRect());
}

void ShapePr::Rectangle::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    GraphicsItem::mouseDoubleClickEvent(event);
    delete this;
}
