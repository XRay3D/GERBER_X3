#pragma once

#include "settings.h"
#include "shape.h"
#include "shrectangle.h"
#include <QGraphicsItem>

namespace Shapes {
class Handler final : public QGraphicsItem {
    friend class Shape;
    friend QDataStream& operator<<(QDataStream& stream, const Shape& sh);

public:
    enum HType {
        Adder,
        Center,
        Corner,
    };
    explicit Handler(Shapes::Shape* shape, HType type = Corner);
    ~Handler();
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setPos(const QPointF& pos);

    HType hType() const;
    void setHType(const HType& value);

private:
    Shape* shape;
    HType m_hType;
    QVector<QPointF> pt;
    inline QRectF rect() const;
    QPointF lastPos;
    inline static QVector<Handler*> hhh;
    void savePos();

    // QGraphicsItem interface
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};
}
