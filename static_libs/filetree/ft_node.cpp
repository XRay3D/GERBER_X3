// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "ft_node.h"
#include "ft_model.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QPainter>
#include <QSharedPointer>
#include <QStyle>
#include <QVariant>
#include <project.h>

namespace FileTree {

Node::Node(std::reference_wrapper<const int> id, Type type)
    : type(type)
    , id_(id) {
}

Node::~Node() {
    if (id_ > -1) {
        switch (type) {
        case File:
            App::project()->deleteFile(id_);
            break;
        case Shape:
            App::project()->deleteShape(id_);
            break;
        default:
            break;
        }
    }
    childs.clear();
}

Node* Node::child(int row) const {
    if (row < 0 || row >= static_cast<int>(childs.size()))
        return nullptr;
    return childs[row].get();
}

Node* Node::parent() const { return parent_; }

void Node::setChild(int row, Node* item) {
    if (item) {
        item->parent_ = this;
        if (row < static_cast<int>(childs.size()))
            childs[row].reset(item);
    }
}

int Node::childCount() const {
    return static_cast<int>(childs.size());
}

int Node::row() const {
    if (parent_)
        return parent_->childs.indexOf(this);
    //    for (int i = 0, size = parent_->childs.size(); i < size; ++i)
    //        if (parent_->childs[i].get() == this)
    //            return i;
    return -1;
}

void Node::addChild(Node* item) {
    item->parent_ = this;
    childs.resize(childs.size() + 1);
    childs.back().reset(item);
}

void Node::remove(int row) { childs.takeAt(row); }

QModelIndex Node::index(int column) const {
    return App::fileModel()->createIndex(row(), column, reinterpret_cast<quintptr>(this));
}

} // namespace FileTree
