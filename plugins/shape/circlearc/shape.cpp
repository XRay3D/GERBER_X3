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
#include <assert.h>

using Shapes::Handle;

namespace ShArc {
Shape::Shape(Shapes::Plugin* plugin,
    QPointF center, QPointF pt1, QPointF pt2)
    : AbstractShape{plugin}
    , radius_{Geo::distance(center, pt1)} {
    if(!std::isnan(center.x())) {
        handles = {
            Handle{pt1},
            Handle{pt2},
            Handle{center, Handle::Center}
        };
        curHandle = handles.data() + 1;
        redraw();
    }
    App::grView().addItem(this);
}

constexpr auto norm(auto v) { return sqrt(QPointF::dotProduct(v, v)); } // norm = length of vector
constexpr auto d(auto u, auto v) { return norm(u - v); }

constexpr double distancePointToLine(const QPointF& pt, const QLineF& line) {
    QPointF v = line.p1() - line.p2();
    QPointF w = pt - line.p2();

    double c1 = QPointF::dotProduct(w, v);
    if(c1 <= 0.0) return d(pt, line.p2());

    double c2 = QPointF::dotProduct(v, v);
    if(c2 <= c1) return d(pt, line.p1());

    double b = c1 / c2;
    QPointF Pb = line.p2() + b * v;
    return d(pt, Pb);
}

void Shape::rebuild() {
    if(handles.empty()) return;
    if(curHandle) {
        Timer_uS t{"redraw"};
        auto updateCenter = [this](bool isCenter = {}) {
            QLineF line{handles[Point1], handles[Point2]};
            const auto angle{line.angle()};
            const auto center{line.center()};
            const auto hCenter{handles[Center]};

            auto tmp = line.angleTo({line.p1(), hCenter}) < 180 ? -1 : 1;
            double radius{};
            if(isCenter) {
                radius = distancePointToLine(hCenter, line);
            } else {
                radius = line.length() * 0.5;
                if(radius > radius_)
                    radius_ = radius, radius = 0;
                else
                    radius = sqrt(radius_ * radius_ - radius * radius);
            }
            handles[Center] = radius ? QLineF::fromPolar(radius * tmp, angle - 90).translated(center).p2() : center;
            if(isCenter)
                radius_ = Geo::distance(handles[Center], handles[Point1]);
        };

        switch(std::distance(handles.data(), curHandle)) {
        case Center: updateCenter(true); break;
        case Point1:
        case Point2: updateCenter(); break;
        }
    } else {
        radius_ = Geo::distance(handles[Center], handles[Point1]);
    }

    // Направление обхода — знак векторного произведения радиус-векторов:
    // из двух дуг между Point1 и Point2 берётся меньшая (< pi).
    const double cross
        = (handles[Point1].x() - handles[Center].x()) * (handles[Point2].y() - handles[Center].y())
        - (handles[Point1].y() - handles[Center].y()) * (handles[Point2].x() - handles[Center].x());
    Geo::Polyline curve{
        {handles[Point1],
         Geo::bulgeOf(handles[Point1], handles[Point2], handles[Center],
         cross > 0 ? Geo::Vertex::Ccw : Geo::Vertex::Cw)},
        {handles[Point2]},
    };
    if(closed) curve.close(); // замыкает хордой
    curves_ = {std::move(curve)};
    shape_ = Geo::toPath(curves_);

    // shape_.clear();
    // shape_.moveTo(handles[Point1]);

    // QRectF rect{
    //     handles[Center].x() - radius_,
    //     handles[Center].y() - radius_,
    //     radius_ * 2,
    //     radius_ * 2,
    // };

    // QLineF l1{handles[Center], handles[Point1]};
    // shape_.arcTo(rect,
    //     l1.angle(),
    //     l1.angleTo({handles[Center], handles[Point2]}));

    // assert(handles.size() == PtCount);
}

QString Shape::name() const { return QObject::tr("CircleArc"); }

QIcon Shape::icon() const { return QIcon::fromTheme(u"draw-ellipse-arc"_s); }

bool Shape::addPt(const QPointF& pt) {
    if(!std::isnan(pt.x()) && ++curHandle < handles.end().base()) {
        *curHandle = pt;
        redraw();
        return true;
    } else
        return curHandle = {}, false;
}

void Shape::setPt(const QPointF& pt) {
    if(curHandle) *curHandle = pt;
    redraw();
}

double Shape::radius() const { return radius_; }

void Shape::setRadius(double radius) {
    if(qFuzzyIsNull(radius) || qFuzzyCompare(radius_, radius))
        return;
    radius_ = radius;
    {
        QLineF line{handles[Center], handles[Point1]};
        line.setLength(radius);
        handles[Point1] = line.p2();
    }
    {
        QLineF line{handles[Center], handles[Point2]};
        line.setLength(radius);
        handles[Point2] = line.p2();
    }
    curHandle = handles.data() + Point2;
    redraw();
}

double Shape::angle(int i) const {
    assert(i < 2);
    return QLineF{handles[Center], handles[i]}.angle();
}

void Shape::setAngle(int i, double radius) {
    assert(i < 2);
    QLineF line{handles[Center], handles[i]};
    line.setAngle(radius);
    handles[i] = line.p2();
    curHandle = handles.data() + i;
    redraw();
}

void Shape::postLoad() {
    radius_ = Geo::distance(handles[Center], handles[Point1]);
    AbstractShape::postLoad();
}

} // namespace ShArc

#include "moc_shape.cpp"
