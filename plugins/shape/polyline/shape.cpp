/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
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

Shape::Shape(Shapes::Plugin* plugin, QPointF pt1, QPointF pt2)
    : AbstractShape{plugin} {
    if(!std::isnan(pt1.x())) {
        handles = {
            Handle{(pt1 + pt1) / 2, Handle::Center},
            Handle{pt1},
            Handle{(pt1 + pt2) / 2, Handle::Adder},
            Handle{pt2}
        };
        redraw();
    }
    App::grView().addItem(this);
}

void Shape::redraw() {
    if(curHandle && QGraphicsItem::flags() & ItemIsMovable) {
        if(curHandle->type() == Handle::Adder) {
            *curHandle = (curHandle[-1] + *curHandle) / 2;
            std::initializer_list<Handle> pts{
                // (curHandle[-1] + *curHandle) / 2,
                {*curHandle,                       Handle::Corner},
                {(curHandle[+1] + *curHandle) / 2, Handle::Adder },
            };
            HIt it{++curHandle};
            curHandle = handles.insert(it, pts).base();
        } else if(curHandle->type() == Handle::Corner) {
            if(curHandle != handles.data() + 1) {
                if(handles.size() > 4 && *curHandle == curHandle[-2]) {
                    curHandle = handles.erase(HIt{curHandle - 2}, HIt{curHandle}).base();
                } else { // update adder
                    curHandle[-1] = (*curHandle + curHandle[-2]) / 2;
                }
            }
            if(curHandle != (--handles.end()).base()) {
                if(handles.size() > 4 && *curHandle == curHandle[+2]) {
                    curHandle = handles.erase(HIt{curHandle}, HIt{curHandle + 2}).base();
                } else { // update adder
                    curHandle[+1] = (*curHandle + curHandle[+2]) / 2;
                }
            }
        }
    }

    Curve curve{
        std::from_range,
        v::filter(handles, std::bind(std::equal_to{}, Handle::Corner, _1))
            | v::transform([](QPointF& pt) { return geo::Vertex{pt}; }),
    };

    if(closed && !curve.isClosed()) curve.emplace_back(curve.front().pt);
    curves_ = {std::move(curve)};
    shape_ = toPPath(curves_);

    if(handles.size() > 4) {
        QPointF c = centroidFast();
        if(qIsNaN(c.x()) || qIsNaN(c.y())) c = {};
        handles[0] = shape_.boundingRect().contains(c) && !c.isNull()
            ? c
            : shape_.boundingRect().center();
    }
}

QString Shape::name() const { return QObject::tr("Line"); }

QIcon Shape::icon() const { return QIcon::fromTheme(u"draw-line"_s); }

void Shape::setPt(const QPointF& pt) {
    curHandle = (--handles.end()).base();
    *curHandle = pt;
    curHandle[-1] = QLineF{curHandle[-2], pt}.center();
    curHandle = {};
    redraw();
}

bool Shape::addPt(const QPointF& pt) {
    if(std::isnan(pt.x())) return false;
    handles.emplace_back((handles.back() + pt) / 2, Handle::Adder);
    handles.emplace_back(pt);
    redraw();
    return !isClosed();
}

bool Shape::isClosed() const { return closed || handles[1] == handles.back(); }

QPointF Shape::centroid() {
    return {};
    QPointF centroid;
    double signedArea{};
    double a{}; // Partial signed area
    mvector<QPointF> vertices;
    vertices.reserve(handles.size() / 2);
    for(auto& h: handles)
        if(h.type() == Handle::Corner)
            vertices.emplace_back(h);
    // For all vertices
    for(size_t i{}; i < vertices.size(); ++i) {
        QPointF p0{vertices[i]};
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
    auto path = handles | v::filter(filter);
    mvector<QPointF> vertices{path.begin(), path.end()};

    auto calc = [&](const QPointF& p0, const QPointF& p1) {
        a = p0.x() * p1.y() - p1.x() * p0.y();
        signedArea += a;
        centroid += (p0 + p1) * a;
    };

    // For all vertices except last
    for(auto&& range: v::slide(vertices, 2))
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
