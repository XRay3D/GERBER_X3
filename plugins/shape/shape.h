/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"
#include "shapepluginin.h"
#include <QModelIndex>
#include <ft_node.h>
#include <memory>

namespace Shapes {

class Handle;
class Node;

class Shape : public GraphicsItem {
    friend class Node;
    friend class Handle;

    friend QDataStream& operator<<(QDataStream& stream, const Shape& shape) {
        stream << shape.type();
        stream << shape.id_;
        stream << shape.isVisible();
        shape.write(stream);
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, Shape& shape) {
        stream >> shape.id_;
        bool visible;
        stream >> visible;
        shape.setZValue(shape.id_);
        shape.setVisible(visible);
        shape.setToolTip(QString::number(shape.id_));
        shape.read(stream);
        return stream;
    }

public:
    Shape();
    ~Shape() override;
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
    virtual bool addPt(const QPointF& point) { return currentHandler = nullptr, false; };
    virtual void setPt(const QPointF& point) = 0;
    Node* node() const;
    void finalize() { isFinal = true; }

protected:
    Handle* currentHandler {};
    mutable mvector<std::unique_ptr<Handle>> handlers;
    Paths paths_;
    Node* node_;
    std::map<Shape*, mvector<QPointF>> hInitPos; // групповое перемещение
    QPointF initPos;                             // групповое перемещение
    bool isFinal {};

    // QGraphicsItem interface
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    // Shape interface
    virtual void updateOtherHandlers(Handle* handler, int mode = {});

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual void menu(QMenu& menu, FileTree::View* tv) const;

    virtual void write(QDataStream& stream) const;
    virtual void read(QDataStream& stream);
};

} // namespace Shapes
