/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "shape.h"
#include <QGraphicsItem>

namespace Shapes {
class Handle final : public QGraphicsItem {
    friend class AbstractShape;
    friend QDataStream& operator<<(QDataStream& stream, const Shapes::AbstractShape& shape);
    friend QDataStream& operator>>(QDataStream& stream, Shapes::AbstractShape& shape);

public:
    enum Type : int {
        Adder,
        Center,
        Corner,
    };
    explicit Handle(Shapes::AbstractShape* shape, Type type = Corner);
    ~Handle();

    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return GiType::ShHandler; }

    Type hType() const;
    void setHType(Type value);

private:
    QRectF rect;
    AbstractShape* const shape;
    Type type_;
    QPointF lastPos;
    bool pressed{};

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
