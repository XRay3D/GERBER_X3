// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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
#include "gi_bridge.h"

#include "graphicsview.h"
#include <QPainter>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>

GiBridge::GiBridge(double& lenght, double& toolDiam, GCode::SideOfMilling& side)
    : side_(side)
    , lenght_(lenght)
    , toolDiam_(toolDiam) {
    pPath.addEllipse(QPointF(), lenght_ / 2, lenght_ / 2);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(std::numeric_limits<double>::max());
}

void GiBridge::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {

    painter->setPen({ok_ ? Qt::green : Qt::red, toolDiam_, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin});
    painter->drawPoint(0, 0);

    painter->setPen({Qt::magenta, 0.0});

    painter->drawPath(pPath);

    //    painter->setBrush(!ok_ ? Qt::red : Qt::green);
    //    painter->setPen(Qt::NoPen);
    //    painter->setTransform(QTransform {}.rotate(-(angle_ - 360)), true);
    //    painter->drawPath(path_);

    //    painter->setBrush(Qt::NoBrush);
    //    painter->setPen(QPen(Qt::white, 2 * App::graphicsView()->scaleFactor()));

    //    const double toolRadius = toolDiam_ / 2;

    //    QLineF line({}, {lenght_ / 2 + toolRadius, 0});
    //    switch (side_) {
    //    case GCode::On:
    //        break;
    //    case GCode::Outer:
    //        line.translate(-toolRadius, 0);
    //        break;
    //    case GCode::Inner:
    //        line.translate(+toolRadius, 0);
    //        break;
    //    }

    //    painter->drawLine(line);

    //    auto drawEllipse = [painter, toolRadius](const QPointF& pt, bool fl) {
    //        const QRectF rectangle(pt + QPointF {toolRadius, toolRadius}, pt - QPointF {toolRadius, toolRadius});
    //        const int startAngle = (fl ? 0 : 180) * 16;
    //        const int spanAngle = 180 * 16;
    //        painter->drawArc(rectangle, startAngle, spanAngle);
    //    };

    //    line.setAngle(+90);
    //    drawEllipse(line.p2(), false);
    //    line.setAngle(-90);
    //    drawEllipse(line.p2(), true);
}

QVariant GiBridge::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionChange)
        return snapedPos(value.toPointF());
    return QGraphicsItem::itemChange(change, value);
}

void GiBridge::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mousePressEvent(event);
    lastPos = pos();
}

QPointF GiBridge::snapedPos(const QPointF& pos) {
    auto col = scene()->collidingItems(this);
    if (col.isEmpty())
        return pos;

    auto retPos {pos};
    auto lenght = std::numeric_limits<double>::max();

    QLineF line;
    ok_ = false;

    auto filter = [](auto* item) {
        auto ty = item->type();
        using enum GiType;
        return item->isSelected() && (ty >= ShCircle || ty == Drill || ty == DataSolid || ty == DataPath);
    };

    auto transform = [](auto* item) { return static_cast<GraphicsItem*>(item); };

    for (GraphicsItem* gi : col | rviews::filter(filter) | rviews::transform(transform)) {
        for (const Path& path : gi->paths()) {
            for (size_t i {}, s = path.size(); i < s; ++i) {
                QLineF tmpLine(path[i], path[(i + 1) % s]);
                double tmp = LineABC(tmpLine).distance(pos);
                if (lenght > tmp && tmp < lenght_)
                    lenght = tmp, line = tmpLine;
            }
        }
    }

    if (!line.isNull()) {
        lenght = line.length() - lenght_ / 2;
        if (QLineF(line.center(), pos).length() < lenght_) {
            retPos = line.center();
            ok_ = true;
        } else if (QLineF(line.p1(), pos).length() < lenght && QLineF(line.p2(), pos).length() < lenght) {
            // точка пересечения на прямой перпендикуляра из 3 точки
            auto k1 = (line.p2().x() - line.p1().x());
            auto k2 = (line.p2().y() - line.p1().y());
            auto k = ((pos.x() - line.p1().x()) * k1 + (pos.y() - line.p1().y()) * k2) / (pow(k1, 2) + pow(k2, 2));
            auto x = line.p1().x() + k * k1;
            auto y = line.p1().y() + k * k2;
            retPos = {x, y};
            ok_ = true;
        }
    }

    return retPos;
}

double GiBridge::angle() const { return angle_; }

void GiBridge::update() {
    pPath = QPainterPath();
    pPath.addEllipse(QPointF(), lenght_ / 2, lenght_ / 2);
    QGraphicsItem::update();
}

// Point GiBridge::getPoint(const int side) const {
//     QLineF l2(0, 0, toolDiam_ / 2, 0);
//     l2.translate(pos());
//     switch (side) {
//     case GCode::On:
//         return (pos());
//     case GCode::Outer:
//         l2.setAngle(angle_ + 180);
//         return (l2.p2());
//     case GCode::Inner:
//         l2.setAngle(angle_);
//         return (l2.p2());
//     }
//     return Point();
// }

// QLineF GiBridge::getPath() const {
//     QLineF retLine(QLineF::fromPolar(toolDiam_ * 0.51, angle_).p2(), QLineF::fromPolar(toolDiam_ * 0.51, angle_ + 180).p2());
//     retLine.translate(pos());
//     return retLine;
// }

double GiBridge::lenght() const { return lenght_; }

bool GiBridge::ok() const { return ok_; }

void GiBridge::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseReleaseEvent(event);
    if (ok_ && pos() == lastPos) {
        moveBrPtr = new GiBridge(lenght_, toolDiam_, side_);
        scene()->addItem(moveBrPtr);
        moveBrPtr->setPos(pos());
        moveBrPtr->setVisible(true);
    } else if (!ok_) {
        scene()->removeItem(this);
        delete this;
    }
}

Paths GiBridge::paths(int) const {
    return {CirclePath((lenght_ + toolDiam_) * uScale, pos())};
}

int GiBridge::type() const { return GiType::Bridge; }

#include "moc_gi_bridge.cpp"
