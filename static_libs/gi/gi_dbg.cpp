/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_dbg.h"
#include "app.h"
#include "graphicsview.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#define QT_DEBUG
// #undef QT_DEBUG

namespace Gi {

bool isPointOnCircleDistance(
    QPointF pt, QPointF center,
    double radius, double epsilon = 1e-4) {

    pt -= center;
    double distance = pt.x() * pt.x() + pt.y() * pt.y();
    return std::abs(distance - radius * radius) < epsilon;
}

bool isPointOnCircle(
    QPointF pt,
    QPointF center, QPointF cPt,
    double epsilon = 1e-4) {

    double radius = QLineF{center, cPt}.length();
    pt -= center;
    double distance = pt.x() * pt.x() + pt.y() * pt.y();
    return std::abs(distance - radius * radius) < epsilon;
}

bool isPointOnCircleInt(int x, int y, int centerX, int centerY, int diameter) {
    int radius = diameter / 2;
    int dx = x - centerX;
    int dy = y - centerY;
    int radiusSquared = radius * radius;
    // Проверяем точное равенство для целых чисел
    return (dx * dx + dy * dy) == radiusSquared;
}

////////////////////////////////////////////////////////////////////////////////

//  Пересечение с прямой (бесконечной линией)
QPolygonF lineCircleIntersection(
    QPointF center, double radius,
    QPointF lineStart, QPointF lineEnd) {

    QPolygonF intersections;

    // Вектор направления линии
    double dx = lineEnd.x() - lineStart.x();
    double dy = lineEnd.y() - lineStart.y();

    // Переносим центр окружности в начало координат
    double fx = lineStart.x() - center.x();
    double fy = lineStart.y() - center.y();

    // Коэффициенты квадратного уравнения: a*t² + b*t + c = 0
    double a = dx * dx + dy * dy;
    double b = 2 * (fx * dx + fy * dy);
    double c = fx * fx + fy * fy - radius * radius;

    // Решаем квадратное уравнение
    double discriminant = b * b - 4 * a * c;

    if(discriminant < 0) {
        return intersections; // Нет пересечений
    }

    if(std::abs(discriminant) < 1e-10) {
        // Одно решение (касательная)
        double t = -b / (2 * a);
        intersections.push_back(QPointF(
            lineStart.x() + t * dx,
            lineStart.y() + t * dy));
    } else {
        // Два решения
        double sqrtDisc = std::sqrt(discriminant);
        double t1 = (-b + sqrtDisc) / (2 * a);
        double t2 = (-b - sqrtDisc) / (2 * a);

        intersections.push_back(QPointF(
            lineStart.x() + t1 * dx,
            lineStart.y() + t1 * dy));
        intersections.push_back(QPointF(
            lineStart.x() + t2 * dx,
            lineStart.y() + t2 * dy));
    }

    return intersections;
}
// Пересечение с отрезком (ограниченной линией)
QPolygonF segmentCircleIntersection(QPointF center, double radius, QPointF segStart, QPointF segEnd) {

    auto points = lineCircleIntersection(center, radius, segStart, segEnd);
    QPolygonF result;

    // Проверяем, попадают ли точки на отрезок
    for(const auto& p: points) {
        // Проверяем, что точка лежит между началом и концом отрезка
        double minX = std::min(segStart.x(), segEnd.x()) - 1e-10;
        double maxX = std::max(segStart.x(), segEnd.x()) + 1e-10;
        double minY = std::min(segStart.y(), segEnd.y()) - 1e-10;
        double maxY = std::max(segStart.y(), segEnd.y()) + 1e-10;

        if(p.x() >= minX && p.x() <= maxX && p.y() >= minY && p.y() <= maxY) {
            result.push_back(p);
        }
    }

    return result;
}
////////////////////////////////////////////////////////////////////////////////

Curve toCurve(Path path) {
    // if(path.size() != 72) return {};

    Curve curve;

    QPainterPath pp;

    static auto eqCenter = [](Point& l, Point& r) {
        auto cl{~GetZ(l)}, cr{~GetZ(r)};
        // qInfo() << ~cl << ~cr;
        // qCritical() << QLineF{~l, cl}.length() << QLineF{~r, cr}.length();
        // auto pt = ~GetZ(l) - ~GetZ(r);
        // double distance = pt.x() * pt.x() + pt.y() * pt.y();

        double epsilon = 0.01; // mm
        // return std::abs(distance - 1e-3 * 1e-3) < epsilon;

        if(cl.isNull() && cr.isNull())
            return false;

        if(cl.isNull() && std::abs(QLineF{~l, cr}.length() - QLineF{~r, cr}.length()) < epsilon)
            return SetZForce(l, ~cr), true;

        if(cr.isNull() && std::abs(QLineF{~l, cl}.length() - QLineF{~r, cl}.length()) < epsilon)
            return SetZForce(r, ~cl), true;

        return cl == cr
            && std::abs(QLineF{~l, cl}.length() - QLineF{~r, cr}.length()) < epsilon;
    };

    static auto notEqCenter = [](Point& l, Point& r) {
        // auto cl{~GetZ(l)}, cr{~GetZ(r)};
        // qInfo() << ~cl << ~cr;
        // qCritical() << QLineF{~l, cl}.length() << QLineF{~r, cr}.length();
        return !eqCenter(l, r) /*cl != cr*/;
        // && std::abs(QLineF{~l, cl}.length() - QLineF{~r, cr}.length()) < 1.e-2;
    };

    if(auto it = r::adjacent_find(path, notEqCenter);
        it != path.end())
        r::rotate(path, r::adjacent_find(path, notEqCenter));

    auto chunks = v::chunk_by(path, eqCenter);

    qInfo() << "toCurve" << path.size();

    auto getDir = [](const Path& path) {
        return Area(path) > .0 ? Vertex::Ccw : Vertex::Cw;
    };

    for(auto&& [p1, p2, p3]: chunks | v::adjacent<3>) { // NOTE fix centers
        if(                                             /*(p1.size() > 2 || p3.size() > 2)
                                                         &&*/ p2.size() == 1
            && ~GetZ(p1.back()) == ~GetZ(p3.front()))
            SetZForce(p2.front(), GetZ(p1.back()));
    }

    chunks = v::chunk_by(path, eqCenter);

    if(chunks.front().size() == path.size()) {
        qWarning() << "circle" << path.size();
        auto center = ~GetZ(path.front());
        auto dir = getDir({path[0], path[1], ~center});
        double radius = QLineF{center, ~path[0]}.length();
        curve.emplace_back(center + QPointF{0., +radius});
        curve.emplace_back(center + QPointF{0., -radius}, center, dir);
        curve.emplace_back(center + QPointF{0., +radius}, center, dir);
    } else {
        Vertex::Type prevType{};
        for(auto&& path: chunks) {
            if(path.size() > 2) {
                qWarning() << "arc 1" << path.size();
                Point center = GetZ(path.front());
                prevType = getDir({path[0], path[1], center});
                curve.emplace_back(~path.front());
                curve.emplace_back(~path.back(), ~center, prevType);
            } else if(path.size() == 1) {
                qWarning() << "arc 2" << path.size();
                if(curve.size() && isPointOnCircle(~path.front(), curve.back().center, curve.back().pt))
                    curve.back().pt = ~path.front(); // NOTE update prev arc
                else
                    curve.emplace_back(~path.front());
            } else {
                qWarning() << "arc 3" << path.size();
                curve.emplace_back(~path.front()); // FIXME m
                curve.emplace_back(~path.back());
            }
        }
        if(curve.size() > 2
            && curve.back().type
            // && isPointOnCircle(curve.front().pt, curve.back().center, curve.back().pt, 0.01)
            && !curve[0].type
            && !curve[1].type) {

            double radius = QLineF{curve.back().center, curve.back().pt}.length();
            auto pts = lineCircleIntersection(curve.back().center, radius, curve[0].pt, curve[1].pt);
            qCritical() << "Fix end" << pts;

            if(pts.empty()) {
                if(QLineF{pts.front(), curve.front().pt}.length()
                    < QLineF{pts.back(), curve.front().pt}.length()) { // TODO fix backward.
                    pp.addEllipse(QRectF{-5e-2, -5e-2, 1e-1, 1e-1}.translated(pts.front()));
                    curve.front().pt = curve.back().pt = pts.front();
                } else
                    curve.front().pt = curve.back().pt = pts.back();
            }
        }

        // curve.emplace_back(~path.front());
    }

    new Debug{toPPath(curve), Qt::green};

    // for(auto&& v: curve)
    // pp.addEllipse(QRectF{-5e-2, -5e-2, 1e-1, 1e-1}.translated(v.pt));

    new Debug{pp, Qt::cyan};
    return curve;
}

Debug::Debug(const QColor& color, double width) {
    pen_ = {color, width};
    App::grView().addItem(this);
    setZValue(std::numeric_limits<double>::max());
    setVisible(true);
}

Debug::Debug(const Path& path, const QColor& color, double width)
    : Debug{Paths{path}, color, width} { }

Debug::Debug(const Paths& paths, const QColor& color, double width)
    : Debug{color, width} {
    paths_ = paths;
#if 0
    for(const Path& path: paths)
        shape_.addPolygon(~path);
    boundingRect_ = shape_.boundingRect();
#else
    for(const Path& path: paths) {
        toCurve(path);
        for(const Point& pt: path) {
            // auto qp = ~GetZ(pt);
            // shape_.addEllipse(qp.x() - 0.05, qp.y() - 0.05, 0.1, 0.1);
        }
    }

    auto bounds = GetBounds(paths);
    boundingRect_ = QRectF{
        dScale * bounds.left,
        dScale * bounds.top,
        dScale * (bounds.right - bounds.left),
        dScale * (bounds.bottom - bounds.top),
    };
#endif
}

Debug::Debug(const QPainterPath& path, const QColor& color, double width)
    : Debug{color, width} {
    shape_ = path;
    boundingRect_ = shape_.boundingRect();
    if(path.isEmpty()) delete this; // NOTE
}

QRectF Debug::boundingRect() const { return boundingRect_; }

void Debug::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    Q_UNUSED(option)

    if(pnColorPrt_)
        pen_.setColor(*pnColorPrt_);
    if(colorPtr_)
        color_ = *colorPtr_;

    double scale = scaleFactor();

    if(pen_.widthF() == 0) {
        QPen pen(pen_);
        pen.setWidthF(1.5 * scale);
        painter->setPen(pen);
    } else
        painter->setPen(pen_);

    // if(option->state & QStyle::State_MouseOver) {
    //     QPen pen(pen_);
    //     pen.setWidthF(2.0 * scale);
    //     pen.setStyle(Qt::CustomDashLine);
    //     pen.setCapStyle(Qt::FlatCap);
    //     pen.setDashPattern({2.0, 2.0});
    //     painter->setPen(pen);
    // }

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(shape_);

    //    painter->setPen(QPen(Qt::magenta, 0.0));
    //    painter->drawRect(rect_);

    double len = 10 * scale;
    for(const Path& path: paths_) {
        for(const Point& pt: path) {
            QPointF p = ~GetZ(pt);
            painter->drawLine(p, p + QPointF{.0, -len});
            painter->drawLine(p, p + QPointF{.0, +len});
            painter->drawLine(p, p + QPointF{-len, .0});
            painter->drawLine(p, p + QPointF{+len, .0});
        }
    }

    ////////////////////////////////////////////////////// for debug cut direction
    if(sc_ != scale) updateArrows();
    painter->drawPath(arrows_);
}

int Debug::type() const { return Type::Debug; }

Paths Debug::paths(int) const { return {} /*paths_*/; }

void Debug::updateArrows() {
    sc_ = scaleFactor();
    arrows_ = QPainterPath(); //.clear();
    if(qFuzzyIsNull(pen_.widthF())) {
        for(const QPolygonF& path: shape_.toSubpathPolygons()) {
            for(int i{}; i < path.size() - 1; ++i) {
                QLineF line(path[i + 1], path[i]);
                double length = 30 * scaleFactor();
                if(line.length() < length && i)
                    continue;
                if(length > 0.5)
                    length = 0.5;
                const double angle = line.angle();
                line.setLength(length);
                line.setAngle(angle + 10);
                arrows_.moveTo(line.p1());
                arrows_.lineTo(line.p2());
                // painter->drawLine(line);
                line.setAngle(angle - 10);
                arrows_.moveTo(line.p1());
                arrows_.lineTo(line.p2());
                // painter->drawLine(line);
            }
        }
    }
}

} // namespace Gi
