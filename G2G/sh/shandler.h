#pragma once

#include "rectangle.h"
#include "settings.h"
#include "shape.h"
#include <QGraphicsItem>

namespace Shapes {
class Handler : public QGraphicsItem {
    friend class Shape;
    friend QDataStream& operator<<(QDataStream& stream, const Shape& sh);

public:
    enum Type {
        Adder,
        Center,
        Corner,
    };
    explicit Handler(Shapes::Shape* shape, Type type = Corner);
    ~Handler();
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setPos(const QPointF& pos);

    Type getHType() const;
    void setHType(const Type& value);

private:
    Shape* shape;
    Type hType;
    QVector<QPointF> pt;
    inline QRectF rect() const;
    QPointF lastPos;
    inline static QVector<Handler*> hhh;

    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};
}
