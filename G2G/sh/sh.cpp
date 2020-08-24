// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "sh.h"
#include "constructor.h"

#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <app.h>
#include <graphicsview.h>
#include <math.h>
#include <scene.h>
#include <settings.h>

using namespace Shapes;

SH::SH(Shapes::Shape* shape, bool center)
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
    painter->setBrush(center ? Qt::red : Qt::green);
    painter->drawEllipse(rect());
}

void SH::setPos(const QPointF& pos, bool fl)
{
    QGraphicsItem::setPos(pos);
    if (fl)
        for (SH* sh : shape->sh) { // прилипание
            if (QLineF(sh->pos(), pos).length() < App::graphicsView()->scaleFactor() * 20) {
                QGraphicsItem::setPos(sh->pos());
                return;
            }
        }
    QGraphicsItem::setPos(pos);
}

QRectF SH::rect() const
{
    const double scale = App::graphicsView()->scaleFactor();
    const double k = 5 * scale;
    const double s = k * 2;
    return { QPointF(-k, -k), QSizeF(s, s) };
}

void SH::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    if (event->modifiers() & Qt::ALT || Constructor::snap()) {
        const double gs = GlobalSettings::gridStep(App::graphicsView()->getScale());
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
        for (SH* sh : shape->sh) { // прилипание
            if (QLineF(sh->pos(), pos()).length() < App::graphicsView()->scaleFactor() * 20) {
                QGraphicsItem::setPos(sh->pos());
            }
        }
        shape->redraw();
    }
}

void Shapes::SH::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    if (center) {
        pt.clear();
        for (SH* item : shape->sh)
            pt.append(item->pos());
    }
}
