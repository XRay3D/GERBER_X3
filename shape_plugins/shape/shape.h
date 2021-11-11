/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
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
#include <ft_node.h>
#include <interfaces/shapepluginin.h>
#include <memory>

namespace Shapes {

class Handler;
class Node;

class Shape : public ShapeInterface {
    friend class Node;
    friend class Handler;
    friend QDataStream& operator<<(QDataStream& stream, const ShapeInterface& shape);
    friend QDataStream& operator>>(QDataStream& stream, ShapeInterface& shape);

public:
    Shape();
    ~Shape();
    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    // GraphicsItem interface
    Paths paths(int alternate = {}) const override;
    void changeColor() override;
    // Shape interface
    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;
    Node* node() const override;
    void finalize() { isFinal = true; }

protected:
    mutable mvector<std::unique_ptr<Handler>> handlers;
    Paths m_paths;
    Node* m_node;
    std::map<Shape*, mvector<QPointF>> hInitPos; // групповое перемещение
    QPointF initPos; // групповое перемещение
    bool isFinal {};

    // QGraphicsItem interface
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    // Shape interface
    virtual void updateOtherHandlers(Handler* handler);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual void menu(QMenu& menu, FileTree::View* tv) const;
    virtual void write([[maybe_unused]] QDataStream& stream) const { }
    virtual void read([[maybe_unused]] QDataStream& stream) { }

    void write_(QDataStream& stream) const override;
    void read_(QDataStream& stream) override;
};
}
