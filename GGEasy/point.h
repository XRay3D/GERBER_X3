#pragma once

#include "scene.h"
#include <QGraphicsItem>

bool updateRect();

class Marker : public QGraphicsItem {
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

    void resetPos(bool fl = true);
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

class Pin : public QObject, public QGraphicsItem {
public:
    Pin(QObject* parent);
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

class LayoutFrames : public QObject, public QGraphicsItem {
    QPainterPath m_path;

public:
    LayoutFrames();
    virtual ~LayoutFrames() override;
    int type() const override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void updateRect(bool fl = false);
};
