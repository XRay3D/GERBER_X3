/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "ft_node.h"

class ExcellonDialog;
class SourceDialog;

namespace Excellon {

class File;

class Node : public FileTree::Node {
    mutable ExcellonDialog* m_exFormatDialog = nullptr;
    friend class File;
    File* file;
    mutable SourceDialog* dialog {};

public:
    explicit Node(File* file, int* id);
    ~Node() override = default;

    // FileTree::Node interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTree::View* tv) const override;
};

}
