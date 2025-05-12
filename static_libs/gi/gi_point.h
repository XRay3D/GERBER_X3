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

    static double minX() { return std::min(App::pin0().x(), App::pin2().x()); }
    static double maxX() { return std::max(App::pin0().x(), App::pin2().x()); }
    static double minY() { return std::min(App::pin0().y(), App::pin2().y()); }
    static double maxY() { return std::max(App::pin0().y(), App::pin2().y()); }

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
    static inline int ctr_;

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
