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

Node::Node(Type type)
    : type_{type} { }

Node::~Node() {
    //    if (id__ > -1) {
    //        switch (type) {
    //        case File:
    //            App::project().deleteFile(id__);
    //            break;
    //        case AbstractShape:
    //            App::project().deleteShape(id__);
    //            break;
    //        default:
    //            break;
    //        }
    //    }
    childs.clear();
}

Node* Node::child(int row) const {
    if(row < 0 || row >= static_cast<int>(childs.size()))
        return nullptr;
    return childs[row].get();
}

Node* Node::parent() const { return parent_; }

void Node::setChild(int row, Node* item) {
    if(item) {
        item->parent_ = this;
        if(row < static_cast<int>(childs.size()))
            childs[row].reset(item);
    }
}

int Node::childCount() const { return static_cast<int>(childs.size()); }

int Node::row() const {
    if(parent_)
        return parent_->childs.indexOf(this);
    //    for (int i = 0, size = parent_->childs.size(); i < size; ++i)
    //        if (parent_->childs[i].get() == this)
    //            return i;
    return -1;
}

void Node::addChild(Node* item, Deleter::Polycy delPolycy) {
    item->parent_ = this;
    childs.emplace_back(item).get_deleter().del = delPolycy;

    //    childs.resize(childs.size() + 1);
    //    childs.back().reset(item);
    //    childs.back().get_deleter().del = delPolycy; // swap(std::unique_ptr<Node, Deleter>(item, Deleter {!dontDelete}));
}

void Node::remove(int row) { childs.takeAt(row); }

QModelIndex Node::index(int column) const {
    return App::fileModel().createIndex(row(), column, reinterpret_cast<quintptr>(this));
}

} // namespace FileTree
