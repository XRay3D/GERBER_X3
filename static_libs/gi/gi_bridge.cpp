// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gi_bridge.h"

#include "graphicsview.h"
#include <QPainter>

GiBridge::GiBridge(double& lenght, double& size, GCode::SideOfMilling& side, GiBridge*& ptr)
    : ptr_(ptr)
    , side_(side)
    , lenght_(lenght)
    , size_(size) {
    connect(App::graphicsView(), &GraphicsView::mouseMove, this, &GiBridge::setNewPos);
    path_.addEllipse(QPointF(), lenght_ / 2, lenght_ / 2);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(std::numeric_limits<double>::max());
}

QRectF GiBridge::boundingRect() const {
    return path_.boundingRect();
}

void GiBridge::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setBrush(!ok_ ? Qt::red : Qt::green);
    painter->setTransform(QTransform().rotate(-(angle_ - 360)), true);
    painter->setPen(Qt::NoPen);
    painter->drawPath(path_);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::white, 2 * App::graphicsView()->scaleFactor()));

    const double halfSize = size_ / 2;

    QLineF l(0, 0, lenght_ / 2 + halfSize, 0);
    switch (side_) {
    case GCode::On:
        break;
    case GCode::Outer:
        l.translate(halfSize, 0);
        break;
    case GCode::Inner:
        l.translate(-halfSize, 0);
        break;
    }

    auto drawEllipse = [painter, halfSize](const QPointF& pt, bool fl = false) {
        const QRectF rectangle(pt + QPointF {halfSize, halfSize}, pt - QPointF {halfSize, halfSize});
        const int startAngle = (fl ? 0 : 180) * 16;
        const int spanAngle = 180 * 16;
        painter->drawArc(rectangle, startAngle, spanAngle);
    };

    l.setAngle(+90);
    drawEllipse(l.p2());
    l.setAngle(-90);
    drawEllipse(l.p2(), true);
}

void GiBridge::setNewPos(const QPointF& pos) { setPos(pos); }

QVariant GiBridge::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionChange) {
        return calculate(value.toPointF());
    } else
        return QGraphicsItem::itemChange(change, value);
}

void GiBridge::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    lastPos_ = pos();
    disconnect(App::graphicsView(), &GraphicsView::mouseMove, this, &GiBridge::setNewPos);
    QGraphicsItem::mousePressEvent(event);
}

QPointF GiBridge::calculate(const QPointF& pos) {
    QList<QGraphicsItem*> col(scene()->collidingItems(this));
    if (col.isEmpty())
        return pos;

    QPointF pt;
    double l = std::numeric_limits<double>::max();
    //    double lastAngle = 0.0;
    for (QGraphicsItem* item : col) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        if (gi && gi->isSelected()) {
            if (auto type(static_cast<GiType>(item->type()));
                type >= GiType::ShCircle ||  //
                type == GiType::Drill ||     //
                type == GiType::DataSolid || //
                type == GiType::DataPath) {
                for (const Path& path : gi->paths()) {
                    for (size_t i = 0, s = path.size(); i < s; ++i) {
                        const QPointF pt1(path[i]);
                        const QPointF pt2(path[(i + 1) % s]);
                        const QLineF l1(pos, pt1);
                        const QLineF l2(pos, pt2);
                        const QLineF l3(pt2, pt1);
                        // pvs   if (lastAngle == 0.0)
                        // pvs        lastAngle = l3.normalVector().angle();
                        const double p = (l1.length() + l2.length() + l3.length()) / 2;
                        if (l1.length() < l3.length() && l2.length() < l3.length()) {
                            const double h = (2 / l3.length()) * sqrt(p * (p - l1.length()) * (p - l2.length()) * (p - l3.length()));
                            if (l > h) {
                                l = h;
                                QLineF line(pt1, pt2);
                                line.setLength(sqrt(l1.length() * l1.length() - h * h));
                                pt = line.p2();
                                const QPointF center(l3.center());
                                if (QLineF(center, pt).length() < lenght_ / 2)
                                    pt = center;
                                angle_ = line.normalVector().angle();
                            }
                        }
                        //                        lastAngle = l3.normalVector().angle();
                    }
                }
            }
        }
    }
    if (l < lenght_ / 2) {
        ok_ = true;
        return pt;
    }
    ok_ = false;
    return pos;
}

void GiBridge::setOk(bool ok) { ok_ = ok; }

double GiBridge::angle() const { return angle_; }

void GiBridge::update() {
    path_ = QPainterPath();
    path_.addEllipse(QPointF(), lenght_ / 2, lenght_ / 2);
    QGraphicsItem::update();
}

IntPoint GiBridge::getPoint(const int side) const {
    QLineF l2(0, 0, size_ / 2, 0);
    l2.translate(pos());
    switch (side) {
    case GCode::On:
        return (pos());
    case GCode::Outer:
        l2.setAngle(angle_ + 180);
        return (l2.p2());
    case GCode::Inner:
        l2.setAngle(angle_);
        return (l2.p2());
    }
    return IntPoint();
}

QLineF GiBridge::getPath() const {
    QLineF retLine(QLineF::fromPolar(size_ * 0.51, angle_).p2(), QLineF::fromPolar(size_ * 0.51, angle_ + 180).p2());
    retLine.translate(pos());
    return retLine;
}

double GiBridge::lenght() const { return lenght_; }

bool GiBridge::ok() const { return ok_; }

QPainterPath GiBridge::shape() const { return path_; }

void GiBridge::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* /*event*/) { deleteLater(); }

void GiBridge::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (ok_ && pos() == lastPos_) {
        ptr_ = new GiBridge(lenght_, size_, side_, ptr_);
        scene()->addItem(ptr_);
        ptr_->setPos(pos());
        ptr_->setVisible(true);
    } else if (!ok_) {
        deleteLater();
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

Paths GiBridge::paths(int) const { return Paths(); }

int GiBridge::type() const { return static_cast<int>(GiType::Bridge); }
