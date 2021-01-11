/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "graphicsitem.h"
#include <QModelIndex>

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
    // GraphicsItem interface
    Paths paths() const override;
    void changeColor() override;
    // Shape interface
    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;

    void setNode(Node* node);

    QModelIndex index() const { return m_index; }
    void setFileIndex(const QModelIndex& index) { m_index = index; }

private:
    Node* m_node = nullptr;
    QModelIndex m_index;

protected:
    mutable QVector<Handler*> handlers;
    Paths m_paths;

    std::map<Shape*, QVector<QPointF>> hInitPos; // групповое перемещение
    QPointF initPos; // групповое перемещение

    // QGraphicsItem interface
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    // Shape interface
    virtual void write(QDataStream& stream) const;
    virtual void read(QDataStream& stream);
    virtual void updateOtherHandlers(Handler* handler);
};
}
