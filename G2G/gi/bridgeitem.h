#pragma once
//#ifndef BRIDGEITEM_H
//#define BRIDGEITEM_H

#include "graphicsitem.h"
#include <QObject>
#include <gctypes.h>

class GraphicsView;

class BridgeItem : public QObject, public GraphicsItem {
    Q_OBJECT
    friend class ProfileForm;

public:
    explicit BridgeItem(double& lenght, double& size, GCode::SideOfMilling& side, BridgeItem*& ptr);
    ~BridgeItem() override { m_ptr = nullptr; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    void setNewPos(const QPointF& pos);
    // GraphicsItem interface
    Paths paths() const override;

    bool ok() const;
    double lenght() const;
    double angle() const;

    void update();

    IntPoint getPoint(const int side) const;
    QLineF getPath() const;

    void setOk(bool ok);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    BridgeItem*& m_ptr;
    QPainterPath m_path;
    QPointF calculate(const QPointF& pos);
    bool m_ok = false;
    double& m_lenght;
    double& m_size;
    GCode::SideOfMilling& m_side;
    double m_angle = 0.0;
    QPointF m_lastPos;
};

//#endif // BRIDGEITEM_H
