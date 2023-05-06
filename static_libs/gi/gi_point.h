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
#pragma once

#include "graphicsview.h"
#include "mvector.h"

#include <span>

bool updateRect();

namespace Gi {

class Marker : public QGraphicsObject {
    Q_OBJECT
public:
    enum Type {
        Zero,
        Home
    };

    Marker(Type type);
    ~Marker() override;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;
    int type() const override;

    void resetPos(bool flUpdateRect = true);
    void setPosX(double x);
    void setPosY(double y);
    inline void setPos(const QPointF& pos) { QGraphicsItem::setPos(pos); }

private:
    QRectF rect_;
    QPainterPath path_;
    QPainterPath shape_;
    const Type type_;
    void updateGCPForm();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
};

class Pin : public QGraphicsObject {
    Q_OBJECT
public:
    Pin();
    ~Pin() override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;
    int type() const override;

    static auto pins() { return std::span(pins_, 4); }
    static void setPinsPos(QPointF pos[4]);

    static double minX() { return qMin(pins_[0]->pos().x(), pins_[1]->pos().x()); }
    static double maxX() { return std::max(pins_[0]->pos().x(), pins_[1]->pos().x()); }
    static double minY() { return qMin(pins_[0]->pos().y(), pins_[2]->pos().y()); }
    static double maxY() { return std::max(pins_[0]->pos().y(), pins_[2]->pos().y()); }

    static void resetPos(bool fl = true);
    static void setPos(const QPointF pos[4]);
    void updateToolTip();
    void setPos(const QPointF& pos);

private:
    QPainterPath path_;
    QPainterPath shape_;
    QRectF rect_;
    QPointF lastPos_;
    const uint index_;
    static inline Pin* pins_[4];
    static inline int ctr_ = 0;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
};

} // namespace Gi

class LayoutFrames : public QGraphicsObject {
    Q_OBJECT
    QPainterPath path_;
    QRectF rect_;

public:
    LayoutFrames();
    ~LayoutFrames() override;
    int type() const override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void updateRect(bool fl = false);
};
