/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include <QModelIndex>

class QGraphicsRectItem;

namespace Gerber {

class Component;

class ComponentsNode {
    ComponentsNode& operator=(ComponentsNode&&) = delete;
    ComponentsNode& operator=(const ComponentsNode&) = delete;
    ComponentsNode(ComponentsNode&&) = delete;
    ComponentsNode(const ComponentsNode&) = delete;

public:
    ComponentsNode(const QString& name);
    ComponentsNode(const Component& component);
    virtual ~ComponentsNode();

    ComponentsNode* child(int row);
    ComponentsNode* parentItem();

    void setChild(int row, ComponentsNode* item);

    int childCount() const;
    int row() const;

    void append(ComponentsNode* item);
    void remove(int row);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;

    QGraphicsRectItem* const item;
    const Component& component;
    const QString name;

protected:
    ComponentsNode* parentItem_ = nullptr;
    QList<QSharedPointer<ComponentsNode>> childItems;
};

} // namespace Gerber
