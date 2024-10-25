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

#include "gi.h"
#include "shapepluginin.h"
#include <QModelIndex>
#include <ft_node.h>
#include <memory>

namespace Shapes {

class Handle;
class Node;

class AbstractShape : public Gi::Item, public ::FileTree::Node {
    friend class Node;
    friend class Handle;

    friend QDataStream& operator<<(QDataStream& stream, const AbstractShape& shape) {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        Block(out).write(
            shape.id_,
            shape.isVisible());
        shape.write(out);
        return stream << data;
    }

    friend QDataStream& operator>>(QDataStream& stream, AbstractShape& shape) {
        QByteArray data;
        stream >> data;
        QDataStream in(&data, QIODevice::ReadOnly);
        bool visible;
        Block(in).read(
            shape.id_,
            visible);
        shape.read(in);
        shape.setZValue(shape.id_);
        shape.setVisible(visible);
        shape.setToolTip(QString::number(shape.id_));
        return stream;
    }

public:
    AbstractShape();
    ~AbstractShape() override;
    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    // Gi::Item interface
    Paths paths(int alternate = {}) const override;
    void changeColor() override;
    // AbstractShape interface
    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool addPt(const QPointF& point) { return currentHandler = nullptr, false; };
    virtual void setPt(const QPointF& point) = 0;
    Node* node() const;
    void finalize() { isFinal = true; }

    //::FileTree::Node interface
    /*virtual*/ bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    /*virtual*/ Qt::ItemFlags flags(const QModelIndex& index) const override;
    /*virtual*/ QVariant data(const QModelIndex& index, int role) const override;
    /*virtual*/ void menu(QMenu& menu, FileTree::View* tv) override;
    int32_t id() const override { return id_; };

protected:
    Handle* currentHandler{};
    mutable mvector<std::unique_ptr<Handle>> handlers;
    Paths paths_;
    //    Node* node_;
    std::map<AbstractShape*, mvector<QPointF>> hInitPos; // групповое перемещение
    QPointF initPos;                                     // групповое перемещение
    bool isFinal{};

    // QGraphicsItem interface
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) override;

    // AbstractShape interface
    virtual void updateOtherHandlers(Handle* handler, int mode = {});
    virtual void write(QDataStream& stream) const;
    virtual void read(QDataStream& stream);
};

} // namespace Shapes
