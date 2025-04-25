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

#include "ft_node.h"

namespace Gi {
class Item;
} // namespace Gi

namespace FileTree {

class FolderNode final : public Node {
    QString name;
    Qt::CheckState checkState_ = Qt::Checked;

public:
    explicit FolderNode(const QString& name);
    explicit FolderNode(const QString& name, int32_t id);
    ~FolderNode() override;

    // Node interface
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    void menu(QMenu& menu, View* tv) override;
};

class ItemNode final : public Node {
    Gi::Item* item;

public:
    explicit ItemNode(Gi::Item* item);
    ~ItemNode() override;

    // Node interface
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void menu(QMenu& menu, View* tv) override;
};

} // namespace FileTree
