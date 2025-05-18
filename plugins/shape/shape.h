/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "gi.h"
#include "shapepluginin.h"
#include <QModelIndex>
#include <ft_node.h>

using namespace std::placeholders;

namespace Shapes {

struct Handle final : QPointF {
    enum Type : int {
        Adder,
        Center,
        Corner,
        Null
    } type_{Null};

    using QPointF::operator=;

    Handle(const QPointF& p, Type t)
        : QPointF{p}, type_{t} { }
    Handle(const QPointF& p)
        : Handle{p, Corner} { }
    Handle()
        : Handle{{}, Corner} { }

    QColor color() const noexcept {
        static const QColor colors[]{
            Qt::yellow,
            Qt::red,
            Qt::green,
            Qt::magenta,
        };
        return colors[type_];
    };

    operator bool() const noexcept { return type_ != Null; }

    void setType(Type type) noexcept { type_ = type; }
    Type type() const noexcept { return type_; }
};

class AbstractShape : public Gi::Item, public ::FileTree::Node {
    friend class Node;

    friend QDataStream& operator<<(QDataStream& stream, const AbstractShape& shape);
    friend QDataStream& operator>>(QDataStream& stream, AbstractShape& shape);
    QPainterPath pPathHandle;

public:
    AbstractShape(Plugin* plugin);
    ~AbstractShape() override;

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) final;
    QRectF boundingRect() const final;
    QPainterPath shape() const final;

    // Gi::Item interface
    Paths paths(int alternate = {}) const override;
    void changeColor() override;
    void redraw() override;

    // AbstractShape interface
    virtual QString name() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool addPt(const QPointF& point [[maybe_unused]]) { return curHandle = {}, false; };
    virtual void setPt(const QPointF& point [[maybe_unused]]) = 0;
    Node* node() const;

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void menu(QMenu& menu, FileTree::View* tv) override;

    int32_t id() const final { return Item::id(); }
    void setId(int32_t id) final { Item::setId(id); }

protected:
    double scale(bool* hasUpdate = nullptr) const;
    bool test(const QPointF& point);

    Plugin* const plugin;
    mutable mvector<Handle> handles;
    using HIter = mvector<Handle>::iterator;
    HIter curHandle{};
    QPointF curHandlePos;
    mutable double hSize{1};

    Paths paths_;
    std::unordered_map<AbstractShape*, mvector<QPointF>> hInitPos; // групповое перемещение
    QPointF initPos;                                               // групповое перемещение

    // QGraphicsItem interface
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) final;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) final;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) final;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final;

    // AbstractShape interface
    virtual void write(QDataStream& stream [[maybe_unused]]) const { };                          // write to project
    virtual void readAndInit(QDataStream& stream [[maybe_unused]]) { AbstractShape::redraw(); }; // read from project
};

} // namespace Shapes
