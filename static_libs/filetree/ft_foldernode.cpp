// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "ft_foldernode.h"
#include "gi.h"

#include <QIcon>

namespace FileTree {

constexpr int FolderNodeId{-1};

FolderNode::FolderNode(const QString& name)
    : FileTree::Node{Folder}
    , name(name) {
    setId(FolderNodeId);
}

FolderNode::FolderNode(const QString& name, int32_t id)
    : FileTree::Node{Folder}
    , name(name) {
    setId(id);
}

FolderNode::~FolderNode() { }

QVariant FolderNode::data(const QModelIndex& index, int role) const {
    if(!index.column()) {
        switch(role) {
        case Qt::DisplayRole:
            return name;
        case Qt::DecorationRole:
            return QIcon::fromTheme("folder");
        default:
            break;
        }
    }
    switch(role) {
    case Role::Id:
        return id();
    case Role::NodeType:
        return Folder;
    case Role::ContentType:
        return childs.size() ? childs.front()->type_ : Type::Null;
    default:
        return {};
    }
}

Qt::ItemFlags FolderNode::flags(const QModelIndex& /*index*/) const {
    return Qt::ItemIsEnabled /*| Qt::ItemIsDropEnabled*/;
}

bool FolderNode::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(index.column())
        return false;

    switch(role) {
    case Qt::CheckStateRole:
        checkState_ = value.value<Qt::CheckState>();
        return true;
    default:
        return false;
    }
}

void FolderNode::menu(QMenu& /*menu*/, View* /*tv*/) { }

///////////////////////////////////////////////////////////////
/// \brief ItemNode::ItemNode
/// \param item
/// \param id
///
ItemNode::ItemNode(Gi::Item* item)
    : FileTree::Node{PathGroup}
    , item{item} { }

ItemNode::~ItemNode() { }

QVariant ItemNode::data(const QModelIndex& index, int role) const {
    if(!index.column()) {
        switch(role) {
        case Qt::DisplayRole:
            return "name";
        case Qt::DecorationRole:
            return drawIcon(item->paths(), item->color());
        default:
            break;
        }
    }
    switch(role) {
    case Role::Id:
        return item->id();
    case Role::NodeType:
        return Folder;
    case Role::ContentType:
        return childs.size() ? childs.front()->type_ : Type::Null;
    default:
        return {};
    }
}

bool ItemNode::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(index.column())
        return false;

    switch(role) {
    case Qt::CheckStateRole:
        item->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
        return true;
    default:
        return false;
    }
}

Qt::ItemFlags ItemNode::flags(const QModelIndex& index) const {
    return Qt::ItemIsEnabled /*| Qt::ItemIsDropEnabled*/;
}

void ItemNode::menu(QMenu& menu, View* tv) { }

} // namespace FileTree
