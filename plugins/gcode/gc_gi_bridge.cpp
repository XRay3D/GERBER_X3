/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_gi_bridge.h"

#include "gcode.h"
#include "graphicsview.h"

#include "geo/util.h"

#include <QPainter>
#include <cmath>

namespace Gi {

Bridge::Bridge() {
    pPath.addEllipse(QPointF(), lenght / 2, lenght / 2);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(std::numeric_limits<double>::max());
}

void Bridge::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setBrush(!ok_ ? Qt::red : Qt::green);
    painter->setPen(Qt::NoPen);
    painter->drawPath(pPath);

    if(!ok_)
        return;
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::gray, 2 * App::grView().scaleFactor()));

    try {
        painter->drawPath(cutoff);
    } catch(...) {
        // qDebug() << std::current_exception();
    }
}

QVariant Bridge::itemChange(GraphicsItemChange change, const QVariant& value) {
    if(change == ItemPositionChange)
        return snapedPos(value.toPointF());
    return QGraphicsItem::itemChange(change, value);
}

void Bridge::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mousePressEvent(event);
    lastPos = pos();
}

// Прилипание к контуру: проекция на ближайший СЕГМЕНТ выделенной геометрии --
// хорду или дугу. Прежний перебор мерил расстояние до бесконечной прямой по
// одним хордам: в bulge-домене дуга -- один сегмент, у полуокружности хорда --
// диаметр, и мост лип не к дуге, а к диаметру.
QPointF Bridge::snapedPos(const QPointF& pos) {
    auto col = scene()->collidingItems(this);
    if(col.isEmpty()) {
        ok_ = false;
        update();
        return pos;
    }

    auto filter = [](auto* item) {
        auto ty = item->type();
        return item->isSelected() && (ty >= Type::ShCircle || ty == Type::Drill || ty == Type::DataSolid || ty == Type::DataPath);
    };

    auto transform = [](auto* item) { return static_cast<Item*>(item); };

    QPointF retPos{pos};
    double minDist{lenght}; // дальше длины моста не прилипаем
    double angle{};
    ok_ = false;

    auto accept = [&](double dist, QPointF proj, double tangentAngle) {
        if(dist >= minDist) return;
        minDist = dist;
        retPos = proj;
        angle = tangentAngle;
        ok_ = true;
    };

    for(Item* gi: col | v::filter(filter) | v::transform(transform)) {
        auto curves = gi->curves();
        if(gi->type() == Type::DataPath
            && curves.size() == 1
            && curves.front().closed
            && curves.front().isPositive()) // fix direction for drawing
            curves.front().reverse();
        for(const Geo::Polyline& curve: curves) {
            for(auto&& [fr, to]: Geo::segments(curve)) {
                if(auto arc = Geo::arcOf(fr, to, fr.bulge)) {
                    const QPointF v = pos - arc->center;
                    const double len = std::hypot(v.x(), v.y());
                    if(len <= 0.0) continue;
                    const QPointF proj = arc->center + v * (arc->radius / len);
                    // Укладывается ли проекция в размах дуги: arcSweep отдаёт
                    // угол от начала сегмента до проекции в направлении обхода.
                    if(std::abs(Geo::arcSweep(fr, proj, arc->center, arc->dir())) > std::abs(arc->theta)) continue;
                    // Касательная -- перпендикуляр к радиусу по ходу обхода.
                    const QPointF tangent = arc->theta > 0.0 ? QPointF{-v.y(), v.x()} : QPointF{v.y(), -v.x()};
                    accept(std::abs(len - arc->radius), proj, QLineF{proj, proj + tangent}.angle());
                } else {
                    const QPointF dir = to - fr;
                    const double len2 = QPointF::dotProduct(dir, dir);
                    if(len2 <= 0.0) continue;
                    const double t = QPointF::dotProduct(pos - fr, dir) / len2;
                    if(t < 0.0 || t > 1.0) continue;
                    const QPointF proj = fr + dir * t;
                    accept(Geo::distance(pos, proj), proj, QLineF{fr, to}.angle());
                }
            }
        }
    }

    if(ok_)
        angle_ = angle;
    update();
    return retPos;
}

void Bridge::update() {
    pPath = QPainterPath();
    pPath.addEllipse(QPointF(), lenght / 2, lenght / 2);

    cutoff.clear();

    if(!ok_) {
        QGraphicsItem::update();
        return;
    }

    QLineF lTool, lCenter = QLineF::fromPolar(toolDiam + lenght, angle_);
    double start{}, span = 180;
    switch(side) {
    case GCode::On:
        lCenter.translate(-lCenter.center());
        lTool = QLineF::fromPolar(toolDiam, start = angle_ - 90);
        break;
    case GCode::Outer:
        lTool = QLineF::fromPolar(toolDiam, start = angle_ - 90);
        lCenter.translate(lTool.center() - lCenter.center());
        break;
    case GCode::Inner:
        lTool = QLineF::fromPolar(toolDiam, start = angle_ + 90);
        lCenter.translate(lTool.center() - lCenter.center());
        span = -180;
        break;
    }

    const QPointF offset{toolDiam / 2., toolDiam / 2};
    const QSizeF size{toolDiam, toolDiam};

    lTool.translate(lCenter.p1() - lTool.center());
    cutoff.moveTo(lTool.p2());
    cutoff.arcTo(QRectF{lCenter.p1() - offset, size}, start + 000, span);
    lTool.translate(lCenter.p2() - lTool.center());
    cutoff.lineTo(lTool.p1());
    cutoff.arcTo(QRectF{lCenter.p2() - offset, size}, start + 180, span);
    lTool.translate(lCenter.p1() - lTool.center());
    cutoff.lineTo(lTool.p2());

    QGraphicsItem::update();
}

bool Bridge::ok() const { return ok_; }

void Bridge::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseReleaseEvent(event);
    if(ok_ && pos() == lastPos) {
        moveBrPtr = new Bridge;
        scene()->addItem(moveBrPtr);
        moveBrPtr->setPos(pos());
        moveBrPtr->setVisible(true);
    } else if(!ok_) {
        scene()->removeItem(this);
        delete this;
    }
}

int Bridge::type() const { return Type::Bridge; }

// След моста на плане -- круг длиной моста плюс диаметр фрезы вокруг позиции.
// В расчёт УП мосты уходят не отсюда, а центрами (см. комментарий к классу).
Geo::Polylines Bridge::curves(int /*alternate*/) const { return {Geo::circle(lenght + toolDiam, pos())}; }

} // namespace Gi
