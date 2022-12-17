// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "sharc.h"
#include "graphicsview.h"
#include "shhandler.h"
#include "utils.h"
#include <QIcon>
#include <QVector2D>
#include <QtMath>

namespace Shapes {

Arc::Arc(QPointF center, QPointF pt1, QPointF pt2)
    : radius_(QLineF(center, pt1).length()) {
    paths_.resize(1);

    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handle>(this, Handle::Center));
    handlers.emplace_back(std::make_unique<Handle>(this));
    handlers.emplace_back(std::make_unique<Handle>(this));

    handlers[Point1]->setPos(pt1);
    handlers[Point2]->setPos(pt2);
    handlers[Center]->setPos(center);

    redraw();

    App::graphicsView()->scene()->addItem(this);
}

constexpr auto dot(auto u, auto v) { return u.x() * v.x() + u.y() * v.y(); }
constexpr auto norm(auto v) { return sqrt(dot(v, v)); } // norm = length of vector
constexpr auto d(auto u, auto v) { return norm(u - v); }

double distancePointToLine(const QPointF& pt, const QLineF& line) {
    QPointF v {line.p1() - line.p2()};
    QPointF w {pt - line.p2()};

    double c1 = dot(w, v);
    if (c1 <= 0.0)
        return d(pt, line.p2());

    double c2 = dot(v, v);
    if (c2 <= c1)
        return d(pt, line.p1());

    double b = c1 / c2;
    QPointF Pb = line.p2() + b * v;
    return d(pt, Pb);
}

void Arc::redraw() {
    shape_ = QPainterPath();

    if (currentHandler) {
        Timer t {"redraw", us_ {}};
        auto updateCenter = [this](bool isCenter = {}) {
            QLineF line {handlers[Point1]->pos(), handlers[Point2]->pos()};
            const auto angle {line.angle()};
            const auto center {line.center()};
            const auto hCenter {handlers[Center]->pos()};

            auto tmp = line.angleTo({line.p1(), hCenter}) < 180 ? -1 : 1;
            double length {};
            if (isCenter) {
                length = distancePointToLine(hCenter, line);
            } else {
                length = line.length() * 0.5;
                if (length > radius_)
                    radius_ = length, length = 0;
                else
                    length = sqrt(radius_ * radius_ - length * length);
            }
            handlers[Center]->setPos(length ? QLineF::fromPolar(length * tmp, angle - 90).translated(center).p2() : center);
            if (isCenter)
                radius_ = QLineF(handlers[Center]->pos(), handlers[Point1]->pos()).length();
        };

        switch (handlers.indexOf(currentHandler)) {
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
        shape_.moveTo(p + QPointF {+k, 0});
        shape_.lineTo(p + QPointF {-k, 0});
        shape_.moveTo(p + QPointF {0, +k});
        shape_.lineTo(p + QPointF {0, -k});
    };
    test(handlers[Center]->pos());

    const QLineF l1(handlers[Center]->pos(), handlers[Point1]->pos());
    const QLineF l2(handlers[Center]->pos(), handlers[Point2]->pos());
    //    radius_ = l1.length();
    double angle1 = two_pi - qDegreesToRadians(l1.angle());
    double angle2 = two_pi - qDegreesToRadians(l2.angle());

    if (qFuzzyCompare(angle1, two_pi))
        angle1 = 0.0;
    double angle = angle2 - angle1;
    if (angle < 0.0)
        angle = two_pi + angle;

    int intSteps = App::settings().clpCircleSegments(radius_);
    const double stepAngle = two_pi / intSteps;
    const cInt radius = static_cast<cInt>(radius_ * uScale);
    const IntPoint center((handlers[Center]->pos()));
    intSteps *= angle / two_pi;

    Path& path = paths_.front();
    path.clear();
    path.reserve(intSteps);

    for (int i {}; i <= intSteps; i++) {
        const double angle = angle1 + stepAngle * i;
        path.emplace_back(
            static_cast<cInt>(radius * cos(angle)) + center.X,
            static_cast<cInt>(radius * sin(angle)) + center.Y);
    }

    path.back() = IntPoint {
        static_cast<cInt>(radius * cos(angle2)) + center.X,
        static_cast<cInt>(radius * sin(angle2)) + center.Y};

    shape_.addPolygon(path);

    setPos({1, 1}); // костыли    //update();
    setPos({0, 0});
}

QString Arc::name() const { return QObject::tr("Arc"); }

QIcon Arc::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

bool Arc::addPt(const QPointF& pt) {
    if (!ptCtr++) {
        handlers[Point2 + ptCtr].get()->setPos(pt);
        updateOtherHandlers(handlers[Point2 + ptCtr].get());
        return true;
    }
    return currentHandler = nullptr, false;
}

void Arc::setPt(const QPointF& pt) {
    handlers[Point2 + ptCtr].get()->setPos(pt);
    updateOtherHandlers(handlers[Point2 + ptCtr].get());
}

double Arc::radius() const { return radius_; }

void Arc::setRadius(double radius) {
    if (!qFuzzyCompare(radius_, radius))
        return;
    radius_ = radius;
    redraw();
}

////////////////////////////////////////////////////////////
/// \brief PluginImpl::PluginImpl
///

int PluginImpl::type() const { return GiType::ShCirArc; }

QIcon PluginImpl::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

Shape* PluginImpl::createShape(const QPointF& point) const { return new Arc(point + QPointF {0, 5}, point, point + QPointF {0, 10}); }

} // namespace Shapes

#include "moc_sharc.cpp"
