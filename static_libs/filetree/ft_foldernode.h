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

#include "ft_node.h"

namespace FileTree {

class FolderNode : public FileTree::Node {
    QString name;
    Qt::CheckState checkState_ = Qt::Checked;

public:
    explicit FolderNode(const QString& name);
    explicit FolderNode(const QString& name, int& id);
    ~FolderNode() override;

    // FileTree::Node interface
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    void menu(QMenu& menu, View* tv) const override;
};

} // namespace FileTree
