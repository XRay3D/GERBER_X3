// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "ft_foldernode.h"

#include <QIcon>

namespace FileTree {

constexpr int FolderNodeId {-1};

FolderNode::FolderNode(const QString& name)
    : FileTree::Node(FolderNodeId, Folder)
    , name(name) {
}

FolderNode::FolderNode(const QString& name, int& id)
    : FileTree::Node(id, Folder)
    , name(name) {
}

FolderNode::~FolderNode() { }

QVariant FolderNode::data(const QModelIndex& index, int role) const {
    if (!index.column()) {
        switch (role) {
        case Qt::DisplayRole:
            return name;
        case Qt::DecorationRole:
            return QIcon::fromTheme("folder");
        default:
            break;
        }
    }
    switch (role) {
    case Role::Id:
        return id_.get();
    case Role::NodeType:
        return Folder;
    case Role::ContentType:
        return childs.size() ? childs.front()->type : Type::Null;
    default:
        return QVariant();
    }
}

Qt::ItemFlags FolderNode::flags(const QModelIndex& /*index*/) const {
    return Qt::ItemIsEnabled /*| Qt::ItemIsDropEnabled*/;
}

bool FolderNode::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.column())
        return false;

    switch (role) {
    case Qt::CheckStateRole:
        checkState_ = value.value<Qt::CheckState>();
        return true;
    default:
        return false;
    }
}

void FolderNode::menu(QMenu& /*menu*/, View* /*tv*/) const {
}

} // namespace FileTree
