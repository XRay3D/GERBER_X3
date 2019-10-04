#ifndef POINT_H
#define POINT_H

#include "scene.h"

#include <QBrush>
#include <QGraphicsItem>
#include <QPen>

class Point : public QGraphicsItem { //Object {
    //    Q_OBJECT

public:
    Point(int type);
    ~Point() override;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;
    int type() const override;

    void resetPos(bool fl = true);
    void setPosX(double x);
    void setPosY(double y);

    enum {
        Null = -1,
        Zero,
        Home
    };

private:
    QRectF m_rect;
    QPainterPath m_path;
    QPainterPath m_shape;
    int m_type = Null;
    void updateGCPForm();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};

class Pin : public QObject, public QGraphicsItem {

public:
    Pin();
    ~Pin() override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;
    int type() const override;

    static QVector<Pin*> pins();
    static double min() { return qMin(m_pins[0]->pos().x(), m_pins[1]->pos().x()); }
    static double max() { return qMax(m_pins[0]->pos().x(), m_pins[1]->pos().x()); }
    static QRectF worckRect;

    void resetPos(bool fl = true);
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
};

#endif // POINT_H
