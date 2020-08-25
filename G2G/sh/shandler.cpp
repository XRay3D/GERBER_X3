// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shandler.h"
#include "constructor.h"
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <QStyleOptionGraphicsItem>
#include <app.h>
#include <graphicsview.h>
#include <math.h>
#include <scene.h>
#include <settings.h>

using namespace Shapes;

void drawRuller(QPainter* painter, const QPointF& pt1)
{
    QFont font;
    font.setPixelSize(16);
    const QString text = QString(::GlobalSettings::inch() ? "  X = %1 in\n"
                                                            "  Y = %2 in\n"
                                                          : "  X = %1 mm\n"
                                                            "  Y = %2 mm\n")
                             .arg(pt1.x() / (::GlobalSettings::inch() ? 25.4 : 1.0), 4, 'f', 3, '0')
                             .arg(pt1.y() / (::GlobalSettings::inch() ? 25.4 : 1.0), 4, 'f', 3, '0');

    const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, text);
    const double k = App::graphicsView()->scaleFactor();
    //    painter->drawLine(line);
    //    line.setLength(20.0 * k);
    //    line.setAngle(angle + 10);
    //    painter->drawLine(line);
    //    line.setAngle(angle - 10);
    //    painter->drawLine(line);
    // draw text
    //painter->setFont(font);
    //painter->drawText(textRect, Qt::AlignLeft, text);
    //    QPointF pt(pt1);
    //    if ((pt.x() + textRect.width() * k) > App::graphicsView()->visibleRegion().boundingRect().right())
    //        pt.rx() -= textRect.width() * k;
    //    if ((pt.y() - textRect.height() * k) < App::graphicsView()->visibleRegion().boundingRect().top())
    //        pt.ry() += textRect.height() * k;
    //    painter->translate(pt);
    painter->scale(k, -k);
    int i = 0;
    for (QString txt : text.split('\n')) {
        QPainterPath path;
        path.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height() * 0.25 * ++i), font, txt);
        painter->setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::white);
        painter->drawPath(path);
    }
}

Handler::Handler(Shape* shape, bool center)
    : shape(shape)
    , center(center)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
    setZValue(std::numeric_limits<double>::max() - 1);
}

QRectF Handler::boundingRect() const { return rect(); }

void Handler::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    painter->setPen(Qt::NoPen);
    QColor c(center ? Qt::red : Qt::green);
    c.setAlpha(127);
    painter->setBrush(c);
    painter->drawEllipse(rect());
    if (option->state & QStyle::State_MouseOver) {
        drawRuller(painter, pos());
    }
}

void Handler::setPos(QPointF pos)
{
    QGraphicsItem::setPos(pos);
    for (Handler* sh : shape->sh) { // прилипание
        if (!sh->center && QLineF(sh->pos(), pos).length() < App::graphicsView()->scaleFactor() * 20) {
            QGraphicsItem::setPos(sh->pos());
            return;
        }
    }
    pos = shape->calcPos(this);
    QGraphicsItem::setPos(pos);
}

QRectF Handler::rect() const
{
    const double scale = App::graphicsView()->scaleFactor();
    const double k = 5 * scale;
    const double s = k * 2;
    return { QPointF(-k, -k), QSizeF(s, s) };
}

void Handler::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    shape->m_scale = std::numeric_limits<double>::max();
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
            shape->sh[i]->QGraphicsItem::setPos(pt[i] + pos() - pt.first());
        shape->redraw();
    } else {
        for (Handler* sh : shape->sh) {
            if (!sh->center && QLineF(sh->pos(), pos()).length() < App::graphicsView()->scaleFactor() * 20) { // прилипание
                QGraphicsItem::setPos(sh->pos());
            }
        }
        if (shape)
            QGraphicsItem::setPos(shape->calcPos(this));
        shape->redraw();
    }
}

void Shapes::Handler::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    shape->m_scale = std::numeric_limits<double>::max();
    QGraphicsItem::mousePressEvent(event);
    if (center) {
        pt.clear();
        for (Handler* item : shape->sh)
            pt.append(item->pos());
    }
}
