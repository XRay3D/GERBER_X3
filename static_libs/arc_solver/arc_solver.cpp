// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "arc_solver.h"
#include "app.h"
#include "cavc/plinesegment.hpp"
#include "cavc/polyline.hpp"
#include "cavc/polylineoffset.hpp"
#include "cavc/vector.hpp"
#include "cavc/vector2.hpp"
#include "graphicsview.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <mutex>
#include <qglobal.h>
#include <qline.h>
#include <qnamespace.h>
#include <qpolygon.h>
#include <qtransform.h>
#include <utility>

#include <QDebug>
#include <QPen>
#include <qmath.h>

std::tuple<Vec2, Vec2, double> arcToBulge(const Vec2& center, double startAngle, double endAngle, double radius) { // FIXME

    const auto startPoint = Vec2{cos(startAngle), sin(startAngle)} * radius + center; //   QLineF::fromPolar(radius, startAngle).p2() + ~center;
    const auto endPoint = Vec2{cos(endAngle), sin(endAngle)} * radius + center;       // QLineF::fromPolar(radius, endAngle).p2() + ~center;
    constexpr double pi2 = std::numbers::pi * 2;
    // const double a = std::fmod((pi2 + (endAngle - startAngle)), pi2) / 4.0;
    // const double bulge = std::sin(a) / std::cos(a);

    double a = std::fmod((pi2 + (endAngle - startAngle)), pi2) / 4.0;
    double bulge = std::sin(a) / std::cos(a);

    if(std::abs(bulge) > 1.0) {
        a = std::fmod((pi2 + (startAngle - endAngle)), pi2) / 4.0;
        bulge = -std::sin(a) / std::cos(a);
    }

    return std::tuple{startPoint, endPoint, bulge};
}

void RotatePoly(Poly& poly, double angle, const Vec2& center) {
    const bool fl = cavc::getArea(poly) < 0;
    for(auto&& pt: poly.vertexes()) {
        const double dAangle = qDegreesToRadians(angle - cavc::angle(center, pt.pos()));
        const double length = std::sqrt(cavc::distSquared(center, pt.pos()));
        pt = {
            cos(dAangle) * length + center.x(),
            sin(dAangle) * length + center.y(),
            pt.bulge(),
        };
    }
    if(fl != (cavc::getArea(poly) < 0))
        ; // FIXME  ReversePath(poly);
}

Poly& ReversePoly(Poly& poly) {
    cavc::invertDirection(poly);
    return poly;
}

Poly CirclePoly(double diametr, const Vec2& center) {
    if(diametr == 0.0)
        return {};

    // closed polyline representing a circle going counter clockwise
    const double radius = diametr / 2;
    Poly ccwCircle;
    ccwCircle.addVertex(center.x() - radius, center.y(), 1);
    ccwCircle.addVertex(center.x() + radius, center.y(), 1);
    ccwCircle.isClosed() = true;
    return ccwCircle;
    // closed polyline representing a circle going clockwise
    // Polyline<double> cwCircle;
    // cwCircle.addVertex(0, 0, -1);
    // cwCircle.addVertex(2.0 * radius, 0, -1);
    // cwCircle.isClosed() = true;
}

Poly RectanglePoly(double width, double height, const Vec2& center) {
    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Poly poligon;
    poligon.vertexes() = {
        PlineVert{-halfWidth + center.x(), +halfHeight + center.y(), 0.0},
        PlineVert{-halfWidth + center.x(), -halfHeight + center.y(), 0.0},
        PlineVert{+halfWidth + center.x(), -halfHeight + center.y(), 0.0},
        PlineVert{+halfWidth + center.x(), +halfHeight + center.y(), 0.0},
        // PlineVert{-halfWidth + center.x(), +halfHeight + center.y(), 0.0},
    };
    poligon.isClosed() = true;
    // if(Area(poligon) < 0.0)
    // ReversePoly(poligon);

    return poligon;
}

QPainterPath polyToPPath(const Polys& polys) {
    QPainterPath result;
    using std::numbers::pi;

    for(auto&& poly: polys) {

        const auto area = cavc::getArea(poly);

        // const bool isClockwise = area < 0; // poly.orientation() == CGAL::CLOCKWISE;
        // qWarning() << "area" << area;
        auto current = poly.vertexes().begin();
        auto end = poly.vertexes().end();
        int first{};
        poly.visitSegIndices([&](size_t sourceIndex, size_t targetIndex) -> bool {
            const auto& source = poly[sourceIndex];
            const auto& target = poly[targetIndex];

            if(!first++)
                result.moveTo(~source);

            if(!source.bulge()) {
                // if(curve.is_linear()) {
                result.lineTo(~target);
            } else { // if(curve.is_circular()) {

                // if(fuzzyEqual(source.pos(), target.pos())) return true;
                const bool isClockwise = source.bulge() > 0.0;

                auto arc = cavc::arcRadiusAndCenter(source, target);
                auto center = ~arc.center;
                const QRectF rect{
                    QPointF{center.x() - arc.radius, center.y() - arc.radius},
                    QPointF{center.x() + arc.radius, center.y() + arc.radius}
                };

                const double asource = std::atan2(source.y() - center.y(), source.x() - center.x());
                const double atarget = std::atan2(target.y() - center.y(), target.x() - center.x());
                double aspan = atarget - asource;
                if(aspan < -pi || (qFuzzyCompare(aspan, -pi) && !isClockwise))
                    aspan += 2.0 * pi;
                else if(aspan > pi || (qFuzzyCompare(aspan, pi) && isClockwise))
                    aspan -= 2.0 * pi;
                result.arcTo(rect, qRadiansToDegrees(-asource), qRadiansToDegrees(-aspan));
            }
            // else { // ?!?
            //     Q_UNREACHABLE();
            // }
            return true;
        });
    }

    return result;
}

Poly& CleanPoly(Poly& poly) {
    // Удаление идущих подряд одинаковых точек.
    auto& vertexes = poly.vertexes();
    auto it = vertexes.begin();

    poly.isClosed() = vertexes.front().pos() == vertexes.back().pos();

    for(; it < (vertexes.end() - 1); ++it)
        if(it->pos() == (it + 1)->pos()) {
            // (it + 1)->bulge() = it->bulge();
            // ((it + 1)->bulge())
            //     ? (it + 1)->bulge() = it->bulge()
            //     : it->bulge() = (it + 1)->bulge();
            vertexes.erase(it--);
        }

    if(it->pos() == vertexes.begin()->pos()) {
        if(qFuzzyIsNull(vertexes.begin()->bulge()) && !qFuzzyIsNull(it->bulge()))
            vertexes.begin()->bulge() = it->bulge();
        vertexes.erase(it--);
    }

    return poly;
}

Polys OffsetPoly(const Poly& poly, double offset, PolyType line) {
    Polys solution;

    auto scene = App::grView().scene();

    if(poly.isClosed()) {
        // return solution;

        // qCritical() << "Area" << cavc::getArea(poly);
        auto offsetIn = cavc::parallelOffset(poly, +offset);
        for(auto&& poly: offsetIn) {
            // scene->addPath(polyToPPath(poly), QPen{Qt::yellow, 0.05}, Qt::NoBrush);
            // scene->addPath(polyToPPath(CirclePoly(0.2, poly[0].pos())), Qt::NoPen, Qt::yellow);
        }

        auto offsetOut = cavc::parallelOffset(poly, -offset);
        for(auto&& poly: offsetOut) {
            ReversePoly(poly);
            // scene->addPath(polyToPPath(poly), QPen{Qt::green, 0.05}, Qt::NoBrush);
            // scene->addPath(polyToPPath(CirclePoly(0.2, poly[0].pos())), Qt::NoPen, Qt::green);
        }

        solution.reserve(offsetIn.size() + offsetOut.size());
        solution.insert(solution.end(), offsetIn.begin(), offsetIn.end());
        solution.insert(solution.end(), offsetOut.begin(), offsetOut.end());
        // std::ranges::move(std::move(offsetIn), std::back_inserter(solution));
        // std::ranges::move(std::move(offsetOut), std::back_inserter(solution));
    } else {
        // qCritical() << "Area" << cavc::getArea(poly);
        auto offsetIn = cavc::parallelOffset(poly, +offset);
        auto offsetOut = cavc::parallelOffset(poly, -offset);
        solution.reserve(offsetIn.size() + offsetOut.size());

        for(auto it = offsetIn.begin(); it != offsetIn.end(); ++it) {
            auto& poly = *it;
            // for(auto&& poly: offsetIn) {
            // scene->addPath(polyToPPath(poly), QPen{Qt::cyan, 0.05}, Qt::NoBrush);
            // scene->addPath(polyToPPath(CirclePoly(0.2, poly[0].pos())), Qt::NoPen, Qt::cyan);
            if(poly.isClosed()) {
                solution.emplace_back(std::move(poly));
                offsetIn.erase(it--);
            }
        }
        for(auto it = offsetOut.begin(); it != offsetOut.end(); ++it) {
            auto& poly = *it;
            // for(auto&& poly: offsetOut) {
            ReversePoly(poly);
            // scene->addPath(polyToPPath(poly), QPen{Qt::magenta, 0.05}, Qt::NoBrush);
            // scene->addPath(polyToPPath(CirclePoly(0.2, poly[0].pos())), Qt::NoPen, Qt::magenta);
            if(poly.isClosed()) {
                solution.emplace_back(std::move(poly));
                offsetOut.erase(it--);
            }
        }

        if(offsetIn.size() == 1 && offsetOut.size() == 1) {
            offsetIn.front().lastVertex().bulge() = 1;
            offsetOut.front().lastVertex().bulge() = 1;
            std::ranges::move(std::move(offsetOut.front().vertexes()), std::back_inserter(offsetIn.front().vertexes()));
            offsetIn.front().isClosed() = true;
            std::ranges::move(std::move(offsetIn), std::back_inserter(solution));
        }

        // std::ranges::move(std::move(offsetIn), std::back_inserter(solution));
        // std::ranges::move(std::move(offsetOut), std::back_inserter(solution));
    }

    return solution;
}

QIcon drawIcon(const Polys& polys, const QColor& color) {
    enum {
        IconSize = 24
    };
    static std::mutex mutex;
    std::unique_lock lock{mutex};

    QPainterPath painterPath;

    for(auto&& poly: polys)
        painterPath += polyToPPath(poly);

    const QRectF rect = painterPath.boundingRect();

    double scale = static_cast<double>(IconSize) / std::max(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    if(rect.width() > rect.height())
        ky += (static_cast<double>(IconSize) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(IconSize) - rect.width() * scale) / 2;

    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(painterPath);
    return pixmap;
}

void TestPoly(const Polys& polys, const QColor& color, const QString& toolTip) {
    auto scene = App::grView().scene();
    for(auto&& poly: polys) {
        if(poly.size() < 2) continue;

        auto pPath = polyToPPath(poly);

        for(int i = 0; i < poly.size() - 1; ++i) {
            QLineF line{~poly[i + 1], ~poly[i]};
            double length = 0.3; // 30 * scaleFactor();
            // if(line.length() < length && i) continue;
            // if(length > 0.5) length = 0.5;
            const double angle = line.angle();
            line.setLength(length);
            line.setAngle(angle + 10);
            pPath.moveTo(line.p1());
            pPath.lineTo(line.p2());
            // painter->drawLine(line);
            line.setAngle(angle - 10);
            pPath.moveTo(line.p1());
            pPath.lineTo(line.p2());
            // painter->drawLine(line);
        }
        scene->addPath(pPath, QPen{color, 0.05}, Qt::NoBrush)->setToolTip(toolTip);

        // auto pt1 = ~poly[0].pos();
        // auto pt2 = ~poly[1].pos();

        // auto angle = cavc::angle(poly[0].pos(), poly[1].pos());
        // QPolygonF arrow{
        //     QVector<QPointF>{
        //                      QLineF::fromPolar(0.5, angle + 30).p2() + pt1,
        //                      pt2,
        //                      QLineF::fromPolar(0.5, angle - 30).p2() + pt1,
        //                      }
        // };

        // scene->addPolygon(arrow, QPen{color, 0.05}, Qt::NoBrush);

        // scene->addPath(polyToPPath(CirclePoly(0.2, poly.vertexes().front().pos())), Qt::NoPen, Qt::green);
        // scene->addPath(polyToPPath(CirclePoly(0.2, poly.vertexes().back().pos())), Qt::NoPen, Qt::red);
    }
}
