// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shpolyline.h"
#include "scene.h"
#include "shhandler.h"
#include <QIcon>

namespace Shapes {

PolyLine::PolyLine(QPointF pt1, QPointF pt2) {
    paths_.resize(1);
    handlers.reserve(4);

    handlers.emplace_back(std::make_unique<Handler>(this, Handler::Center));
    handlers.emplace_back(std::make_unique<Handler>(this));
    handlers.emplace_back(std::make_unique<Handler>(this, Handler::Adder));
    handlers.emplace_back(std::make_unique<Handler>(this));

    handlers[1]->setPos(pt1);
    handlers[3]->setPos(pt2);

    redraw();

     App::graphicsView()->scene()->addItem(this);
}

void PolyLine::redraw() {
    Path& path = paths_.front();
    path.clear();
    for (size_t i = 1, e = handlers.size(); i < e; ++i) {
        if (handlers[i]->hType() == Handler::Corner)
            path.emplace_back((handlers[i]->pos()));
    }
    shape_ = QPainterPath();
    shape_.addPolygon(path);
    //    rect_ = shape_.boundingRect();
    if (handlers.size() > 4) {
        QPointF c(centroidFast());
        if (qIsNaN(c.x()) || qIsNaN(c.y()))
            c = {};
        handlers[0]->QGraphicsItem::setPos(shape_.boundingRect().contains(c) && !c.isNull() ? c : shape_.boundingRect().center());
        handlers[0]->setVisible(true);
    } else
        handlers[0]->setVisible(false);
    setPos({1, 1}); //костыли    //update();
    setPos({0, 0});
}

QString PolyLine::name() const { return QObject::tr("Line"); }

QIcon PolyLine::icon() const { return QIcon::fromTheme("draw-line"); }

void PolyLine::updateOtherHandlers(Handler* handler) {
    if (handler->hType() == Handler::Adder) {
        int idx = handlers.indexOf(handler);
        Handler* h;
        {
            Handler* h1 = handlers[idx + 1].get();
            handlers.insert(handlers.begin() + idx + 1, std::make_unique<Handler>(this, Handler::Adder));
            h = handlers[idx + 1].get();
            h->QGraphicsItem::setPos(QLineF(handler->pos(), h1->pos()).center());
        }
        {
            Handler* h1 = handlers[idx].get();
            handlers.insert(handlers.begin() + idx, std::make_unique<Handler>(this, Handler::Adder));
            h = handlers[idx].get();
            h->QGraphicsItem::setPos(QLineF(handler->pos(), h1->pos()).center());
        }
        handler->setHType(Handler::Corner);
    } else if (handler->hType() == Handler::Corner /*&& !Constructor::item*/) {
        int idx = handlers.indexOf(handler);
        if (handler != handlers[1].get()) {
            if (handlers.size() > 4
                && handler->pos() == handlers[idx - 2]->pos() /*QLineF(handler->pos(), handlers[idx - 2]->pos()).length() < handler->rect().width() * 0.5*/) {
                handlers.takeAt(idx - 1);
                handlers.takeAt(idx - 2);
                idx -= 2;
            } else {
                handlers[idx - 1]->QGraphicsItem::setPos(QLineF(handler->pos(), handlers[idx - 2]->pos()).center());
            }
        }
        if (handler != handlers.back().get()) {
            if (handlers.size() > 4
                && handler->pos() == handlers[idx + 2]->pos() /*QLineF(handler->pos(), handlers[idx + 2]->pos()).length() < handler->rect().width() * 0.5*/) {
                handlers.takeAt(idx + 1);
                handlers.takeAt(idx + 1);
            } else {
                handlers[idx + 1]->QGraphicsItem::setPos(QLineF(handler->pos(), handlers[idx + 2]->pos()).center());
            }
        }
    }
}

void PolyLine::setPt(const QPointF& pt) {
    handlers[handlers.size() - 2]->QGraphicsItem::setPos(QLineF(handlers[handlers.size() - 3]->pos(), pt).center());
    handlers.back()->Handler::setPos(pt);
    redraw();
}

void PolyLine::addPt(const QPointF& pt) {
    Handler* h1 = handlers.back().get();
    handlers.emplace_back(std::make_unique<Handler>(this, Handler::Adder));

    Handler* h2 = handlers.back().get();
    handlers.back()->QGraphicsItem::setPos(pt);

    handlers.emplace_back(std::make_unique<Handler>(this));
    handlers.back()->QGraphicsItem::setPos(pt);
    h2->QGraphicsItem::setPos(QLineF(h1->pos(), pt).center());

    redraw();
}

bool PolyLine::closed() { return handlers[1]->pos() == handlers.back()->pos(); }

QPointF PolyLine::centroid() {
    QPointF centroid;
    double signedArea = 0.0;
    double a = 0.0; // Partial signed area
    mvector<QPointF> vertices;
    vertices.reserve(handlers.size() / 2);
    for (auto& h : handlers) {
        if (h->hType() == Handler::Corner)
            vertices.emplace_back(h->pos());
    }
    // For all vertices
    for (size_t i = 0; i < vertices.size(); ++i) {
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

QPointF PolyLine::centroidFast() {
    QPointF centroid;
    double signedArea = 0.0;
    double a = 0.0; // Partial signed area
    mvector<QPointF> vertices;
    vertices.reserve(handlers.size() / 2);
    for (auto& h : handlers) {
        if (h->hType() == Handler::Corner)
            vertices.emplace_back(h->pos());
    }
    // For all vertices except last
    size_t i = 0;
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

////////////////////////////////////////////////////////////
/// \brief Plugin::Plugin
///
Plugin::Plugin() { }

Plugin::~Plugin() { }

int Plugin::type() const { return GiType::ShPolyLine; }

QIcon Plugin::icon() const { return QIcon::fromTheme("draw-line"); }

Shape* Plugin::createShape() { return shape = new PolyLine(); }

Shape* Plugin::createShape(const QPointF& point) { return shape = new PolyLine(point, point); }

bool Plugin::addShapePoint(const QPointF& point) {
    if (shape->closed())
        return false;
    else
        shape->addPt(point);
    return true;
}

void Plugin::updateShape(const QPointF& point) {
    if (shape)
        shape->setPt(point);
}

void Plugin::finalizeShape() {
    if (shape)
        shape->finalize();
    shape = nullptr;
    emit actionUncheck();
}

} // namespace Shapes
