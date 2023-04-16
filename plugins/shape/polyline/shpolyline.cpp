// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shpolyline.h"
#include "graphicsview.h"
#include "shhandler.h"
#include <QIcon>

namespace Shapes {

PolyLine::PolyLine(QPointF pt1, QPointF pt2) {
    paths_.resize(1);
    handlers.reserve(4);

    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Center));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Adder));
    handlers.emplace_back(std::make_unique<Handle>(this));

    handlers[1]->setPos(pt1);
    handlers[2]->setPos(pt1);
    handlers[3]->setPos(pt2);

    redraw();

    App::graphicsView().addItem(this);
}

void PolyLine::redraw() {
    if(currentHandler) {
        if(currentHandler->hType() == Handle::Adder) {
            int idx = handlers.indexOf(currentHandler);
            Handle* h;
            {
                Handle* h1 = handlers[idx + 1].get();
                handlers.insert(handlers.begin() + idx + 1, std::make_unique<Handle>(this, Handle::Adder));
                h = handlers[idx + 1].get();
                h->QGraphicsItem::setPos(QLineF(currentHandler->pos(), h1->pos()).center());
            }
            {
                Handle* h1 = handlers[idx].get();
                handlers.insert(handlers.begin() + idx, std::make_unique<Handle>(this, Handle::Adder));
                h = handlers[idx].get();
                h->QGraphicsItem::setPos(QLineF(currentHandler->pos(), h1->pos()).center());
            }
            currentHandler->setHType(Handle::Corner);
        } else if(currentHandler->hType() == Handle::Corner /*&& !Constructor::item*/) {
            int idx = handlers.indexOf(currentHandler);
            if(currentHandler != handlers[1].get()) {
                if(handlers.size() > 4
                    && currentHandler->pos() == handlers[idx - 2]->pos() /*QLineF(handler->pos(), handlers[idx - 2]->pos()).length() < handler->rect().width() * 0.5*/) {
                    handlers.takeAt(idx - 1);
                    handlers.takeAt(idx - 2);
                    idx -= 2;
                } else {
                    handlers[idx - 1]->QGraphicsItem::setPos(QLineF(currentHandler->pos(), handlers[idx - 2]->pos()).center());
                }
            }
            if(currentHandler != handlers.back().get()) {
                if(handlers.size() > 4
                    && currentHandler->pos() == handlers[idx + 2]->pos() /*QLineF(handler->pos(), handlers[idx + 2]->pos()).length() < handler->rect().width() * 0.5*/) {
                    handlers.takeAt(idx + 1);
                    handlers.takeAt(idx + 1);
                } else {
                    handlers[idx + 1]->QGraphicsItem::setPos(QLineF(currentHandler->pos(), handlers[idx + 2]->pos()).center());
                }
            }
        }
    }

    Path& path = paths_.front();
    path.clear();
    for(size_t i{1}, e = handlers.size(); i < e; ++i)
        if(handlers[i]->hType() == Handle::Corner)
            path.emplace_back((handlers[i]->pos()));
    shape_ = QPainterPath();
    shape_.addPolygon(path);
    //    rect_ = shape_.boundingRect();
    if(handlers.size() > 4) {
        QPointF c(centroidFast());
        if(qIsNaN(c.x()) || qIsNaN(c.y()))
            c = {};
        handlers[0]->QGraphicsItem::setPos(shape_.boundingRect().contains(c) && !c.isNull() ? c : shape_.boundingRect().center());
        handlers[0]->setVisible(true);
    } else
        handlers[0]->setVisible(false);
    setPos({1, 1}); // костыли    //update();
    setPos({0, 0});
}

QString PolyLine::name() const { return QObject::tr("Line"); }

QIcon PolyLine::icon() const { return QIcon::fromTheme("draw-line"); }

void PolyLine::setPt(const QPointF& pt) {
    handlers[handlers.size() - 2]->setPos(QLineF(handlers[handlers.size() - 3]->pos(), pt).center());
    handlers.back()->setPos(pt);
    updateOtherHandlers(nullptr);
}

bool PolyLine::addPt(const QPointF& pt) {
    auto pos = handlers.back()->pos();
    auto adder = handlers.emplace_back(std::make_unique<Handle>(this, Handle::Adder)).get();
    handlers.emplace_back(std::make_unique<Handle>(this))->setPos(pt);
    adder->setPos(QLineF(pos, pt).center());
    updateOtherHandlers(nullptr);
    return !closed();
}

bool PolyLine::closed() const { return handlers[1]->pos() == handlers.back()->pos(); }

QPointF PolyLine::centroid() {
    return {};
    QPointF centroid;
    double signedArea = 0.0;
    double a = 0.0; // Partial signed area
    mvector<QPointF> vertices;
    vertices.reserve(handlers.size() / 2);
    for(auto& h: handlers)
        if(h->hType() == Handle::Corner)
            vertices.emplace_back(h->pos());
    // For all vertices
    for(size_t i = 0; i < vertices.size(); ++i) {
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
    return {};
    QPointF centroid;
    double signedArea = 0.0;
    double a = 0.0; // Partial signed area
    mvector<QPointF> vertices;
    vertices.reserve(handlers.size() / 2);
    for(auto& h: handlers)
        if(h->hType() == Handle::Corner)
            vertices.emplace_back(h->pos());
    // For all vertices except last
    size_t i = 0;
    for(; i < vertices.size() - 1; ++i) {
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

int PluginImpl::type() const { return GiType::ShPolyLine; }

QIcon PluginImpl::icon() const { return QIcon::fromTheme("draw-line"); }

AbstractShape* PluginImpl::createShape(const QPointF& point) const { return new PolyLine(point, point); }

} // namespace Shapes

#include "moc_shpolyline.cpp"
