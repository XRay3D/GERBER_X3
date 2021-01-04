/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "interfaces/node.h"

namespace Dxf {

class File;
class Layer;

class Node : public NodeInterface {
    mutable bool header = true;
    mutable bool layer = true;

    File* dxfFile() const;

public:
    explicit Node(int id);
    ~Node() override = default;

    // NodeInterface interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTreeView* tv) const override;
};

class NodeLayer : public NodeInterface {
    friend class Node;
    const QString name;
    Layer* const layer;

public:
    explicit NodeLayer(const QString& name, Layer* layer);
    ~NodeLayer() override = default;

    // NodeInterface interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTreeView* tv) const override;
};

}
