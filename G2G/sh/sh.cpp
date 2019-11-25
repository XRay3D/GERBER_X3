#include "sh.h"
#include "constructor.h"

#include <QGraphicsSceneMouseEvent>
#include <graphicsview.h>
#include <scene.h>
#include <settings.h>
#include <math.h>

using namespace ShapePr;

SH::SH(ShapePr::Shape* shape, bool center)
    : shape(shape)
    , center(center)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
    setZValue(std::numeric_limits<double>::max() - 1);
}

QRectF SH::boundingRect() const { return rect(); }

void SH::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::red);
    painter->drawRect(rect());
}

QRectF SH::rect() const
{
    const double scale = GraphicsView::scaleFactor();
    const double k = 5 * scale;
    const double s = k * 2;
    return { QPointF(-k, -k), QSizeF(s, s) };
}

void SH::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    if (event->modifiers() & Qt::ALT || Constructor::snap()) {
        const double gs = Settings::gridStep(GraphicsView::getScale());
        QPointF px(pos() / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        setPos(px);
    }
    if (center) {
        for (int i = 1, end = shape->sh.size(); i < end; ++i)
            shape->sh[i]->setPos(pt[i] + pos() - pt.first());
        shape->redraw();
    } else {
        shape->redraw();
    }
}

void ShapePr::SH::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    if (center) {
        pt.clear();
        for (SH* item : shape->sh)
            pt.append(item->pos());
    }
}
