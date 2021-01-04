/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "scene.h"
#include <QGraphicsItem>

bool updateRect();

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
    static Marker* get(Type type) { return m_markers[type]; }

private:
    QRectF m_rect;
    QPainterPath m_path;
    QPainterPath m_shape;
    Type m_type;
    static Marker* m_markers[2];
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

    static QVector<Pin*> pins();
    static double minX() { return qMin(m_pins[0]->pos().x(), m_pins[1]->pos().x()); }
    static double maxX() { return qMax(m_pins[0]->pos().x(), m_pins[1]->pos().x()); }
    static double minY() { return qMin(m_pins[0]->pos().y(), m_pins[2]->pos().y()); }
    static double maxY() { return qMax(m_pins[0]->pos().y(), m_pins[2]->pos().y()); }

    static void resetPos(bool fl = true);
    void updateToolTip();
    void setPos(const QPointF& pos);

private:
    QRectF m_rect;
    QPainterPath m_path;
    QPainterPath m_shape;
    static QVector<Pin*> m_pins;
    QPointF m_lastPos;
    const int m_index;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
};

class LayoutFrames : public QGraphicsObject {
    Q_OBJECT
    QPainterPath m_path;
    QRectF m_rect;

public:
    LayoutFrames();
    ~LayoutFrames() override;
    int type() const override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void updateRect(bool fl = false);
};
