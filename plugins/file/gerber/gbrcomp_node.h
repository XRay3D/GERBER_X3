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

namespace Gerber::Comp {

class Component;

class sNode {
    sNode& operator=(sNode&&) = delete;
    sNode& operator=(const sNode&) = delete;
    sNode(sNode&&) = delete;
    sNode(const sNode&) = delete;

public:
    sNode(const QString& name);
    sNode(const Component& component);
    virtual ~sNode();

    sNode* child(int row);
    sNode* parentItem();

    void setChild(int row, sNode* item);

    int childCount() const;
    int row() const;

    void append(sNode* item);
    void remove(int row);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;

    QGraphicsRectItem* const item;
    const Component& component;
    const QString name;

protected:
    sNode* parentItem_ = nullptr;
    QList<QSharedPointer<sNode>> childItems;
};

} // namespace Gerber::Comp
