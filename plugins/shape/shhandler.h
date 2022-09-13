/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "settings.h"
#include "shape.h"
#include <QGraphicsItem>

namespace Shapes {
class Handler final : public QGraphicsItem {
    friend class Shape;
    friend QDataStream& operator<<(QDataStream& stream, const ShapeInterface& shape);
    friend QDataStream& operator>>(QDataStream& stream, ShapeInterface& shape);

public:
    enum Type : int {
        Adder,
        Center,
        Corner,
    };
    explicit Handler(Shapes::Shape* shape, Type type = Corner);
    ~Handler();

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    // QGraphicsItem interface
    int type() const override;

    //    void setPos(const QPointF& pos);

    Type hType() const;
    void setHType(Type value);

private:
    QRectF rect;
    Shape* const shape;
    Type hType_;
    QPointF lastPos;
    bool pressed {};

protected:
    // QGraphicsItem interface
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    //    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

} // namespace Shapes
