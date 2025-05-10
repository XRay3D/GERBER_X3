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
#include <assert.h>

using Shapes::Handle;

namespace ShArc {
// clang-format off
Shape::Shape(Shapes::Plugin* plugin,QPointF center, QPointF pt1, QPointF pt2)
    : AbstractShape{plugin}
    , radius_{QLineF{center, pt1}.length()} {
    paths_.resize(1);
    // clang-format on
    if(!std::isnan(center.x())) {
        handles = {
            Handle{pt1},
            Handle{pt2},
            Handle{center, Handle::Center}
        };
        curHandle = ++handles.begin();
        redraw();
    }
    App::grView().addItem(this);
}

constexpr auto dot(auto u, auto v) { return u.x() * v.x() + u.y() * v.y(); }
constexpr auto norm(auto v) { return sqrt(dot(v, v)); } // norm = length of vector
constexpr auto d(auto u, auto v) { return norm(u - v); }

constexpr double distancePointToLine(const QPointF& pt, const QLineF& line) {
    QPointF v = line.p1() - line.p2();
    QPointF w = pt - line.p2();

    double c1 = dot(w, v);
    if(c1 <= 0.0) return d(pt, line.p2());

    double c2 = dot(v, v);
    if(c2 <= c1) return d(pt, line.p1());

    double b = c1 / c2;
    QPointF Pb = line.p2() + b * v;
    return d(pt, Pb);
}

void Shape::redraw() {
    shape_.clear();

    if(curHandle.base()) {
        Timer_uS t{"redraw"};
        auto updateCenter = [this](bool isCenter = {}) {
            QLineF line{handles[Point1], handles[Point2]};
            const auto angle{line.angle()};
            const auto center{line.center()};
            const auto hCenter{handles[Center]};

            auto tmp = line.angleTo({line.p1(), hCenter}) < 180 ? -1 : 1;
            double length{};
            if(isCenter) {
                length = distancePointToLine(hCenter, line);
            } else {
                length = line.length() * 0.5;
                if(length > radius_)
                    radius_ = length, length = 0;
                else
                    length = sqrt(radius_ * radius_ - length * length);
            }
            handles[Center] = length ? QLineF::fromPolar(length * tmp, angle - 90).translated(center).p2() : center;
            if(isCenter)
                radius_ = QLineF{handles[Center], handles[Point1]}.length();
        };

        switch(std::distance(handles.begin(), curHandle)) {
        case Center:
            updateCenter(true);
            break;
        case Point1:
        case Point2:
            updateCenter();
            break;
        }
    }

    auto test = [this](const QPointF& p) {
        auto k = 10 * scaleFactor();
        shape_.moveTo(p + QPointF{+k, 0});
        shape_.lineTo(p + QPointF{-k, 0});
        shape_.moveTo(p + QPointF{0, +k});
        shape_.lineTo(p + QPointF{0, -k});
    };
    test(handles[Center]);

    const QLineF l1{handles[Center], handles[Point1]};
    const QLineF l2{handles[Center], handles[Point2]};

    double angle1 = two_pi - qDegreesToRadians(l1.angle());
    double angle2 = two_pi - qDegreesToRadians(l2.angle());

    if(qFuzzyCompare(angle1, two_pi)) angle1 = 0.0;
    double angle = angle2 - angle1;
    if(angle < 0.0) angle = two_pi + angle;

    size_t intSteps = App::settings().clpCircleSegments(radius_);
    const double stepAngle = two_pi / intSteps;
    const /*Point::Type*/ int32_t radius = static_cast</*Point::Type*/ int32_t>(radius_ * uScale);
    const Point center = ~handles[Center];
    intSteps *= angle / two_pi;

    Path& path = paths_.front();
    path.clear();
    path.reserve(intSteps);

    for(size_t i{}; i <= intSteps; i++) {
        const double angle = angle1 + stepAngle * i;
        path.emplace_back(
            static_cast<int32_t>(radius * cos(angle)) + center.x,
            static_cast<int32_t>(radius * sin(angle)) + center.y);
    }

    path.back() = Point{
        static_cast<int32_t>(radius * cos(angle2)) + center.x,
        static_cast<int32_t>(radius * sin(angle2)) + center.y,
    };

    shape_.addPolygon(~path);

    assert(handles.size() == PtCount);
    assert(paths_.size() == 1);
}

QString Shape::name() const { return QObject::tr("CircleArc"); }

QIcon Shape::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

bool Shape::addPt(const QPointF& pt) {
    if(!std::isnan(pt.x()) && ++curHandle < handles.end()) {
        *curHandle = pt;
        redraw();
        return true;
    }
    return curHandle = {}, false;
}

void Shape::setPt(const QPointF& pt) {
    if(curHandle.base()) *curHandle = pt;
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
    curHandle = handles.begin() + Point2;
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
    curHandle = handles.begin() + i;
    redraw();
}

void Shape::readAndInit(QDataStream& stream [[maybe_unused]]) {
    curHandle = handles.begin() + Center;
    radius_ = QLineF{handles[Center], handles[Point1]}.length();
    redraw();
}

} // namespace ShArc

#include "moc_shape.cpp"
