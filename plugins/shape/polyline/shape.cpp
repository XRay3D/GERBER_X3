/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "shape.h"
#include "graphicsview.h"
#include "math.h"

#include <QIcon>

using Shapes::Handle;

namespace ShPoly {

Shape::Shape(QPointF pt1, QPointF pt2) {
    paths_.resize(1);

    if(!std::isnan(pt1.x())) {
        handles = {
            Handle{(pt1 + pt1) / 2, Handle::Center},
            Handle{pt1},
            Handle{(pt1 + pt2) / 2, Handle::Adder},
            Handle{pt2}
        };
        AbstractShape::redraw();
    }

    App::grView().addItem(this);
}

Shape::~Shape() {
    std::erase(model->shapes, this);
    qobject_cast<QTableView*>(model->parent())->reset();
}

QVariant Shape::itemChange(GraphicsItemChange change, const QVariant& value) {
    if(change == GraphicsItemChange::ItemSelectedChange)
        qobject_cast<QTableView*>(model->parent())->reset();
    return Shapes::AbstractShape::itemChange(change, value);
}

void Shape::redraw() {
    if(curHandle.base() && QGraphicsItem::flags() & ItemIsMovable) {
        if(curHandle->type() == Handle::Adder) {
            QPointF pts[]{
                QLineF{*(curHandle - 1), *curHandle}
                    .center(),
                *curHandle,
                QLineF{*(curHandle + 1), *curHandle}
                    .center(),
            };
            curHandle->setPos(pts[0]);
            // NOTE may invalidate the pointer↓, update↓
            curHandle = handles.insert(++curHandle, {
                                                        Handle{pts[1], Handle::Corner},
                                                        Handle{pts[2], Handle::Adder }
            });
        } else if(curHandle->type() == Handle::Corner) {
            if(curHandle != handles.begin() + 1) {
                if(handles.size() > 4 && *curHandle == (curHandle - 2)->pos()) {
                    // NOTE may invalidate the pointer↓, update↓
                    curHandle = handles.erase(curHandle - 2, curHandle);
                } else { // update adder
                    (curHandle - 1)->setPos(QLineF{*curHandle, *(curHandle - 2)}.center());
                }
            }
            if(curHandle != handles.end() - 1) {
                if(handles.size() > 4 && *curHandle == (curHandle + 2)->pos()) {
                    // NOTE may invalidate the pointer↓, update↓
                    curHandle = handles.erase(curHandle, curHandle + 2);
                } else { // update adder
                    (curHandle + 1)->setPos(QLineF{*curHandle, *(curHandle + 2)}.center());
                }
            }
        }
    }

    auto filter = [](const auto& h) { return h.type() == Handle::Corner; };
    auto transform = [](const auto& h) { return ~h.pos(); };
    auto path = handles
        | std::views::filter(filter)
        | std::views::transform(transform);

    paths_.front() = {path.begin(), path.end()};
    shape_.clear();
    shape_.addPolygon(~paths_.front());
    //    rect_ = shape_.boundingRect();
    if(handles.size() > 4) {
        auto c = centroidFast();
        if(qIsNaN(c.x()) || qIsNaN(c.y())) c = {};
        handles[0].setPos(shape_.boundingRect().contains(c) && !c.isNull() ? c : shape_.boundingRect().center());
        // handles[0].setVisible(true);
    } else {
        // handles[0].setVisible(false);
    }

    //    if(model)
    //        qobject_cast<QTableView*>(model->parent())->reset();
    assert(paths_.size() == 1);
}

QString Shape::name() const { return QObject::tr("Line"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-line"); }

void Shape::setPt(const QPointF& pt) {
    handles.back().setPos(pt);
    curHandle = handles.end() - 2; // center handle
    curHandle->setPos(QLineF{*--curHandle, pt}.center());
    updateOtherhandles(/*curHandle.base()*/);
}

bool Shape::addPt(const QPointF& pt) {
    handles.emplace_back((handles.back() + pt) / 2, Handle::Adder);
    handles.emplace_back(pt);
    updateOtherhandles(/*&handles.back()*/);
    return !closed();
}

bool Shape::closed() const { return handles[1] == handles.back(); }

QPointF Shape::centroid() {
    return {};
    QPointF centroid;
    double signedArea = 0.0;
    double a = 0.0; // Partial signed area
    mvector<QPointF> vertices;
    vertices.reserve(handles.size() / 2);
    for(auto& h: handles)
        if(h.type() == Handle::Corner)
            vertices.emplace_back(h);
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

QPointF Shape::centroidFast() {
    return {};
    QPointF centroid;
    double signedArea{}, a{}; // Partial signed area

    auto filter = [](const auto& h) { return h.type() == Handle::Corner; };
    auto path = handles | std::views::filter(filter);
    mvector<QPointF> vertices{path.begin(), path.end()};

    auto calc = [&](const QPointF& p0, const QPointF& p1) {
        a = p0.x() * p1.y() - p1.x() * p0.y();
        signedArea += a;
        centroid += (p0 + p1) * a;
    };

    // For all vertices except last
    for(auto&& range: std::views::slide(vertices, 2))
        calc(range.front(), range.back());
    // Do last vertex separately to avoid performing an expensive
    // modulus operation in each iteration.
    calc(vertices.back(), vertices.front());
    signedArea *= 0.5;
    centroid /= (6.0 * signedArea);
    return centroid;
}

} // namespace ShPoly

#include "moc_shape.cpp"
