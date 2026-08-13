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

void Shape::rebuild() {
    if(handles.empty()) return;
    // Раскладка: [0] — маркер центра, углы на нечётных индексах, средние
    // ручки сегментов между ними; у замкнутой полилинии в хвосте лежит
    // средняя ручка замыкающего сегмента (последний угол -> первый).
    if(curHandle && QGraphicsItem::flags() & ItemIsMovable) {
        if(curHandle->type() == Handle::Adder) {
            if(static_cast<Editor*>(plugin->editor())->arcAddMode()) {
                // в режиме дуг средняя ручка не делит сегмент, а гнёт его
                curHandle->setType(Handle::Center2);
            } else {
                // потащили среднюю ручку — сегмент делится новым углом
                const bool tail = curHandle == &handles.back(); // замыкающий сегмент
                const QPointF next = tail ? handles[1] : curHandle[+1];
                *curHandle = (curHandle[-1] + *curHandle) / 2;
                std::initializer_list<Handle> pts{
                    {*curHandle,              Handle::Corner},
                    {(next + *curHandle) / 2, Handle::Adder },
                };
                HIt it{++curHandle};
                curHandle = handles.insert(it, pts).base();
            }
        }
        if(curHandle->type() == Handle::Center2) {
            // центр дуги живёт на серединном перпендикуляре хорды
            *curHandle = arcCenter(size_t(curHandle - handles.data()));
        } else if(curHandle->type() == Handle::Corner) {
            const size_t minSize = closed ? 7 : 4; // замкнутой оставляем минимум треугольник
            if(curHandle != handles.data() + 1) {
                if(handles.size() > minSize && *curHandle == curHandle[-2]) {
                    curHandle = handles.erase(HIt{curHandle - 2}, HIt{curHandle}).base();
                } else {
                    updMiddle(size_t(curHandle - handles.data()) - 1);
                }
            }
            if(curHandle != lastCorner()) {
                if(handles.size() > minSize && *curHandle == curHandle[+2]) {
                    curHandle = handles.erase(HIt{curHandle}, HIt{curHandle + 2}).base();
                } else {
                    updMiddle(size_t(curHandle - handles.data()) + 1);
                }
            }
            if(closed) // замыкающая средняя ручка следует за крайними углами
                updMiddle(handles.size() - 1);
        }
    }

    Geo::Polyline curve;
    curve.reserve(handles.size() / 2);
    // прогиб сегмента пишется на его начальной вершине; последняя средняя
    // ручка есть только у замкнутой — у открытой последний угол без прогиба
    for(size_t i = 1; i < handles.size(); i += 2)
        curve.emplace_back(handles[i], i + 1 < handles.size() ? segBulge(i + 1) : 0.0);

    if(closed) curve.close();
    curves_ = {std::move(curve)};
    shape_ = Geo::toPath(curves_);

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
    if(handles.size() > 4 && pt == handles[1]) {
        setClosed(true); // клик в первую точку замыкает контур
        return false;
    }
    handles.emplace_back((handles.back() + pt) / 2, Handle::Adder);
    handles.emplace_back(pt);
    redraw();
    return !isClosed();
}

bool Shape::isClosed() const { return closed; }

void Shape::setClosed(bool fl) {
    if(closed == fl) return;
    closed = fl;
    if(handles.size() > 2) {
        if(closed) // средняя ручка замыкающего сегмента
            handles.emplace_back((handles[1] + handles.back()) / 2, Handle::Adder);
        else if(handles.back().type() != Handle::Corner)
            handles.pop_back();
    }
    curHandle = {};
    redraw();
}

Shapes::Handle* Shape::lastCorner() const {
    return handles.data() + handles.size() - (closed ? 2 : 1);
}

QPointF Shape::nextCorner(size_t midIdx) const {
    return midIdx + 1 < handles.size() ? handles[midIdx + 1] : handles[1];
}

QPointF Shape::arcCenter(size_t midIdx) const {
    const QPointF a = handles[midIdx - 1], b = nextCorner(midIdx);
    const QPointF mid = (a + b) / 2;
    const QPointF n{a.y() - b.y(), b.x() - a.x()}; // левая нормаль хорды
    const double nn = QPointF::dotProduct(n, n);
    if(qFuzzyIsNull(nn)) return mid; // вырожденный сегмент
    const double t = QPointF::dotProduct(handles[midIdx] - mid, n) / nn;
    return mid + n * t;
}

double Shape::segBulge(size_t midIdx) const {
    const Handle& h = handles[midIdx];
    if(h.type() != Handle::Center2) return 0.0;
    const QPointF a = handles[midIdx - 1], b = nextCorner(midIdx);
    if(a == b) return 0.0;
    const QPointF n{a.y() - b.y(), b.x() - a.x()};
    const double t = QPointF::dotProduct(QPointF{h} - (a + b) / 2, n);
    // центр слева от хорды — обход Ccw: из двух дуг между углами берётся
    // меньшая (до полуокружности), выгнутая от центра
    return Geo::bulgeOf(a, b, h, t > 0 ? Geo::Vertex::Ccw : Geo::Vertex::Cw);
}

void Shape::updMiddle(size_t midIdx) {
    Handle& h = handles[midIdx];
    if(h.type() == Handle::Center2) h = arcCenter(midIdx);
    else h = (handles[midIdx - 1] + nextCorner(midIdx)) / 2;
}

QPointF Shape::centroid() {
    return {};
    QPointF centroid;
    double signedArea{};
    double a{}; // Partial signed area
    std::vector<QPointF> vertices;
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
    std::vector<QPointF> vertices{path.begin(), path.end()};

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
