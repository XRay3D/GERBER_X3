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

GiBridge::GiBridge() {
    pPath.addEllipse(QPointF(), lenght / 2, lenght / 2);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(std::numeric_limits<double>::max());
}

void GiBridge::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setBrush(!ok_ ? Qt::red : Qt::green);
    painter->setPen(Qt::NoPen);
    painter->drawPath(pPath);

    if (!ok_)
        return;

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::gray, 2 * App::graphicsView()->scaleFactor()));
    painter->drawPath(cutoff);
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
    auto minLenght = std::numeric_limits<double>::max();

    QLineF line;
    ok_ = false;

    auto filter = [](auto* item) {
        auto ty = item->type();
        using enum GiType;
        return item->isSelected() && (ty >= ShCircle || ty == Drill || ty == DataSolid || ty == DataPath);
    };

    auto transform = [](auto* item) { return static_cast<GraphicsItem*>(item); };

    for (GraphicsItem* gi : col | rviews::filter(filter) | rviews::transform(transform)) {
        auto paths = gi->paths();
        if (gi->type() == GiType::DataPath
            && paths.size() == 1
            && paths.front().front() == paths.front().back()
            && IsPositive(paths.front())) // fix direction for drawing
            ReversePath(paths.front());
        for (Path& path : paths) {
            for (size_t i {}, s = path.size(); i < s; ++i) {
                QLineF tmpLine(path[i], path[(i + 1) % s]);
                double tmp = LineABC(tmpLine).distance(pos);
                if (minLenght > tmp && tmp < lenght)
                    minLenght = tmp, line = tmpLine;
            }
        }
    }

    if (!line.isNull()) {
        minLenght = line.length() - lenght / 2;
        angle_ = line.angle();
        if (QLineF(line.center(), pos).length() < lenght / 2 && line.length() < lenght) {
            // точка центра прямой
            retPos = line.center();
            ok_ = true;
            update(); // Cutoff
        } else if (QLineF(line.p1(), pos).length() < minLenght && QLineF(line.p2(), pos).length() < minLenght) {
            // точка пересечения на прямой перпендикуляра из 3 точки
            auto k1 = (line.p2().x() - line.p1().x());
            auto k2 = (line.p2().y() - line.p1().y());
            auto k = ((pos.x() - line.p1().x()) * k1 + (pos.y() - line.p1().y()) * k2) / (pow(k1, 2) + pow(k2, 2));
            auto x = line.p1().x() + k * k1;
            auto y = line.p1().y() + k * k2;
            retPos = {x, y};
            ok_ = true;
            update(); // Cutoff
        }
    }

    return retPos;
}

void GiBridge::update() {
    pPath = QPainterPath();
    pPath.addEllipse(QPointF(), lenght / 2, lenght / 2);

    cutoff.clear();

    if (!ok_)
        return;

    QLineF lTool, lCenter = QLineF::fromPolar(toolDiam + lenght, angle_);
    double start, span = 180;
    switch (side) {
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

    if (0) { // test
        QLineF lTool2 = testLine();
        lTool2.translate(pos() - lTool2.center());
        cutoff.moveTo(lTool2.p1());
        cutoff.lineTo(lTool2.p2());
    }

    const QPointF offset {toolDiam / 2., toolDiam / 2};
    const QSizeF size {toolDiam, toolDiam};

    lTool.translate(lCenter.p1() - lTool.center());
    cutoff.moveTo(lTool.p2());
    cutoff.arcTo(QRectF {lCenter.p1() - offset, size}, start + 000, span);
    lTool.translate(lCenter.p2() - lTool.center());
    cutoff.lineTo(lTool.p1());
    cutoff.arcTo(QRectF {lCenter.p2() - offset, size}, start + 180, span);
    lTool.translate(lCenter.p1() - lTool.center());
    cutoff.lineTo(lTool.p2());

    QGraphicsItem::update();
}

bool GiBridge::test(const Path& path) { return pointOnPolygon(testLine(), path, &intersectPoint); }

QLineF GiBridge::testLine() const {
    QLineF lTool2 = QLineF::fromPolar(toolDiam * 1.2, angle_ - 90);
    return lTool2.translated(pos() - lTool2.center());
}

bool GiBridge::ok() const { return ok_; }

void GiBridge::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseReleaseEvent(event);
    if (ok_ && pos() == lastPos) {
        moveBrPtr = new GiBridge;
        scene()->addItem(moveBrPtr);
        moveBrPtr->setPos(pos());
        moveBrPtr->setVisible(true);
    } else if (!ok_) {
        scene()->removeItem(this);
        delete this;
    }
}

int GiBridge::type() const { return GiType::Bridge; }

Paths GiBridge::paths(int alternate) const { return {CirclePath((lenght + toolDiam) * uScale, intersectPoint)}; }

#include "moc_gi_bridge.cpp"
