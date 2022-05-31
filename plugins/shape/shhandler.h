/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
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
    enum HType : int {
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
    QRectF rect() const;

private:
    Shape* const shape;
    HType m_hType;
    mvector<QPointF> pt;
    QPointF lastPos;
    void savePos();
    bool pressed {};

protected:
    // QGraphicsItem interface
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};
} // namespace Shapes
