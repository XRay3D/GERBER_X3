// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shandler.h"
#include "constructor.h"
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <app.h>
#include <graphicsview.h>
#include <math.h>
#include <scene.h>
#include <settings.h>

namespace Shapes {

void drawPos(QPainter* painter, const QPointF& pt1)
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

Handler::Handler(Shape* shape, Type type)
    : shape(shape)
    , hType(type)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable);
    switch (hType) {
    case Adder:
        setZValue(std::numeric_limits<double>::max());
        break;
    case Center:
        setZValue(std::numeric_limits<double>::max());
        break;
    case Corner:
        setZValue(std::numeric_limits<double>::max());
        break;
    }
    hhh.append(this);
}

Handler::~Handler()
{
    hhh.removeOne(this);
    qDebug("~Handler");
}

QRectF Handler::boundingRect() const { return rect(); }

void Handler::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    painter->setPen(Qt::NoPen);
    QColor c;
    switch (hType) {
    case Adder:
        c = QColor(Qt::yellow);
        break;
    case Center:
        c = QColor(Qt::red);
        break;
    case Corner:
        c = QColor(Qt::green);
        break;
    }
    c.setAlpha(200);
    painter->setBrush(c);
    painter->drawEllipse(rect());
    if (option->state & QStyle::State_MouseOver) {
        drawPos(painter, pos());
    }
}

void Handler::setPos(const QPointF& pos)
{
    QGraphicsItem::setPos(pos);
    for (Handler* sh : hhh) { // прилипание
        if (sh->hType == Corner && sh->shape->isVisible() && QLineF(sh->pos(), pos).length() < App::graphicsView()->scaleFactor() * 20) {
            QGraphicsItem::setPos(sh->pos());
            return;
        }
    }
    QGraphicsItem::setPos(shape->calcPos(this));
}

Handler::Type Handler::getHType() const { return hType; }

void Handler::setHType(const Type& value) { hType = value; }

QRectF Handler::rect() const
{
    const double scale = App::graphicsView()->scaleFactor();
    const double k = 5 * scale;
    const double s = k * 2;
    return { QPointF(-k, -k), QSizeF(s, s) };
}

void Handler::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    qDebug() << this;
    shape->m_scale = std::numeric_limits<double>::max();
    QGraphicsItem::mouseMoveEvent(event);
    if (event->modifiers() & Qt::ALT || Constructor::snap()) {
        const double gs = GlobalSettings::gridStep(App::graphicsView()->getScale());
        QPointF px(pos() / gs);
        px.setX(gs * round(px.x()));
        px.setY(gs * round(px.y()));
        setPos(px);
    }
    if (hType == Center) {
        for (int i = 1, end = shape->sh.size(); i < end; ++i)
            shape->sh[i]->QGraphicsItem::setPos(pt[i] + pos() - pt.first());
        shape->redraw();
    } else {
        int idx = shape->sh.indexOf(this);
        for (int i = 1; i < hhh.size(); ++i) {
            Handler* h = hhh[i];
            if (h != this
                && h->hType == Corner
                && h->shape->isVisible()
                && QLineF(h->pos(), pos()).length() < App::graphicsView()->scaleFactor() * 20) { // прилипание
                QGraphicsItem::setPos(h->pos());
                if (shape->type() == GiShapeL) {
                    /*  */ if (auto s = shape->sh.value(idx - 2); s
                               && h->shape == s->shape
                               && h == s
                               && shape->sh.size() > 4) {
                        delete shape->sh.takeAt(idx - 1);
                        delete shape->sh.takeAt(idx - 2);
                        break;
                    } else if (auto s = shape->sh.value(idx + 2); s
                               && h->shape == s->shape
                               && h == s
                               && shape->sh.size() > 4) {
                        delete shape->sh.takeAt(idx + 1);
                        delete shape->sh.takeAt(idx + 1);
                        break;
                    }
                }
            }
        }
        QGraphicsItem::setPos(shape->calcPos(this));
        shape->redraw();
    }
}

void Handler::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    shape->m_scale = std::numeric_limits<double>::max();
    QGraphicsItem::mousePressEvent(event);
    if (hType == Center) {
        pt.clear();
        for (Handler* item : shape->sh)
            pt.append(item->pos());
    }
}

void Handler::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    //    int idx = shape->sh.indexOf(this);
    //    qDebug() << idx;
    //    if (shape->type() == GiShapeL && hType == Corner) {
    //        if (this != shape->sh.last() && this != shape->sh[1]) {
    //            delete shape->sh.takeAt(idx + 1);
    //            delete shape->sh.takeAt(idx - 1);
    //            --idx;
    //            hType = Adder;
    //            QGraphicsItem::setPos(QLineF(shape->sh[idx - 1]->pos(), shape->sh[idx + 1]->pos()).center());
    //            shape->redraw();
    //        }
    //    }
}
}
