#pragma once

#include <gi/graphicsitem.h>

namespace Shapes {

class Handler;
class Node;

class Shape : public GraphicsItem {
    friend class Handler;
    friend class Constructor;
    friend QDataStream& operator<<(QDataStream& stream, const Shape& shape);
    friend QDataStream& operator>>(QDataStream& stream, Shape& shape);

public:
    Shape();
    ~Shape();
    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    Paths paths() const override;

    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;

    void setNode(Node* node);

private:
    Node* m_node = nullptr;

protected:
    mutable double m_scale = std::numeric_limits<double>::max();
    mutable QVector<Handler*> handlers;
    Paths m_paths;

    std::map<Shape*, QVector<QPointF>> hInitPos; // групповое перемещение
    QPointF initPos; // групповое перемещение

    // QGraphicsItem interface
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    virtual void write(QDataStream& stream) const = 0;
    virtual void read(QDataStream& stream) = 0;
    virtual QPointF calcPos(Handler* sh) = 0;

    void changeColor() override;
};
}
