#include "shpolyline.h"
#include "scene.h"
#include "shhandler.h"
#include <QIcon>

#include "leakdetector.h"

namespace Shapes {

PolyLine::PolyLine(QPointF pt1, QPointF pt2)
{
    m_paths.resize(1);
    handlers = { new Handler(this, Handler::Center), new Handler(this), new Handler(this, Handler::Adder), new Handler(this) };
    handlers[1]->setPos(pt1);
    handlers[3]->setPos(pt2);

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(-std::numeric_limits<double>::max());
    App::scene()->addItem(this);
    //    for (auto var : handlers) {

    //    }
}

void PolyLine::redraw()
{
    Path& path = m_paths.first();
    path.clear();
    for (int i = 1, e = handlers.size(); i < e; ++i) {
        if (handlers[i]->hType() == Handler::Corner)
            path.append((handlers[i]->pos()));
    }
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(path));
    m_rect = m_shape.boundingRect();
    if (handlers.size() > 4) {
        QPointF c(centroidFast());
        if (qIsNaN(c.x()) || qIsNaN(c.y()))
            c = {};
        handlers[0]->QGraphicsItem::setPos(m_shape.boundingRect().contains(c) && !c.isNull() ? c : m_shape.boundingRect().center());
        handlers[0]->setVisible(true);
    } else
        handlers[0]->setVisible(false);
    m_scale = std::numeric_limits<double>::max();
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

QString PolyLine::name() const { return QObject::tr("Line"); }

QIcon PolyLine::icon() const { return QIcon::fromTheme("draw-line"); }

QPointF PolyLine::calcPos(Handler* handler)
{
    if (handler->hType() == Handler::Adder) {
        int idx = handlers.indexOf(handler);
        Handler* h;
        {
            Handler* h1 = handlers[idx + 1];
            handlers.insert(idx + 1, h = new Handler(this, Handler::Adder));
            h->QGraphicsItem::setPos(
                QLineF(handler->pos(), h1->pos()).center());
        }
        {
            Handler* h1 = handlers[idx];
            handlers.insert(idx, h = new Handler(this, Handler::Adder));
            h->QGraphicsItem::setPos(
                QLineF(handler->pos(), h1->pos()).center());
        }
        handler->setHType(Handler::Corner);
    } else if (handler->hType() == Handler::Corner) {
        int idx = handlers.indexOf(handler);
        if (handler != handlers[1]) {
            if (handlers.size() > 4
                && handler->pos() == handlers[idx - 2]->pos()) {
                delete handlers.takeAt(idx - 1);
                delete handlers.takeAt(idx - 2);
                idx -= 2;
            } else
                handlers[idx - 1]->QGraphicsItem::setPos(
                    QLineF(handler->pos(), handlers[idx - 2]->pos()).center());
        }
        if (handler != handlers.last()) {
            if (handlers.size() > 4
                && handler->pos() == handlers[idx + 2]->pos()) {
                delete handlers.takeAt(idx + 1);
                delete handlers.takeAt(idx + 1);
            } else
                handlers[idx + 1]->QGraphicsItem::setPos(
                    QLineF(handler->pos(), handlers[idx + 2]->pos()).center());
        }
    }
    return handler->pos();
}

void PolyLine::setPt(const QPointF& pt)
{
    handlers[handlers.size() - 2]->QGraphicsItem::setPos(QLineF(handlers[handlers.size() - 3]->pos(), pt).center());
    handlers.last()->Handler::setPos(pt);
    redraw();
}

void PolyLine::addPt(const QPointF& pt)
{
    Handler* h1 = handlers.last();
    Handler* h2;
    handlers.append(h2 = new Handler(this, Handler::Adder));
    handlers.last()->QGraphicsItem::setPos(pt);

    handlers.append(new Handler(this));
    handlers.last()->QGraphicsItem::setPos(pt);
    h2->QGraphicsItem::setPos(QLineF(h1->pos(), pt).center());

    redraw();
}

bool PolyLine::closed() { return handlers[1]->pos() == handlers.last()->pos(); }

QPointF PolyLine::centroid()
{
    QPointF centroid;
    double signedArea = 0.0;
    double a = 0.0; // Partial signed area
    QPolygonF vertices;
    vertices.reserve(handlers.size() / 2);
    for (auto h : handlers) {
        if (h->hType() == Handler::Corner)
            vertices.append(h->pos());
    }
    // For all vertices
    for (int i = 0; i < vertices.size(); ++i) {
        QPointF p0(vertices[i]);
        QPointF p1(vertices[(i + 1) % vertices.size()]);
        a = p0.x() * p1.y() - p1.x() * p0.y();
        signedArea += a;
        centroid += (p0 + p1) * a;
    }

    signedArea *= 0.5;
    centroid /= (6.0 * signedArea);
    return centroid;
}

QPointF PolyLine::centroidFast()
{
    QPointF centroid;
    double signedArea = 0.0;
    double a = 0.0; // Partial signed area
    QPolygonF vertices;
    vertices.reserve(handlers.size() / 2);
    for (auto h : handlers) {
        if (h->hType() == Handler::Corner)
            vertices.append(h->pos());
    }
    // For all vertices except last
    int i = 0;
    for (; i < vertices.size() - 1; ++i) {
        QPointF p0(vertices[i]);
        QPointF p1(vertices[i + 1]);
        a = p0.x() * p1.y() - p1.x() * p0.y();
        signedArea += a;
        centroid += (p0 + p1) * a;
    }
    // Do last vertex separately to avoid performing an expensive
    // modulus operation in each iteration.
    QPointF p0(vertices[i]);
    QPointF p1(vertices[0]);
    a = p0.x() * p1.y() - p1.x() * p0.y();
    signedArea += a;
    centroid += (p0 + p1) * a;
    signedArea *= 0.5;
    centroid /= (6.0 * signedArea);
    return centroid;
}

}
