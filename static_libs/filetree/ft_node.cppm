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
module;
#include "ft_node.h"
#include "ft_model.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QPainter>
#include <QSharedPointer>
#include <QStyle>
#include <QVariant>
#include <project.h>

#include "utils.h"
#include <memory>
#include <mvector.h>

#include <QChar>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QVariant>

class QMenu;

namespace FileTree {

class View;

enum Column : int {
    NameColorVisible,
    Side,
    ItemsType,
    Count
};

enum Role : int {
    Id = Qt::UserRole,
    Select,
    NodeType,
    ContentType,
};

enum Type : int {
    Null,
    Folder,
    File,
    SubFile,
    AbstractShape,
    PathGroup
};

class Node {
    Node& operator=(Node&&) = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&) = delete;
    Node(const Node&) = delete;

public:
    explicit Node(Type type);
    virtual ~Node();

    Node* child(int row) const;
    Node* parent() const;

    void setChild(int row, Node* item);

    int childCount() const;
    int row() const;

    void addChild(Node* item, Deleter::Polycy delPolycy = Deleter::Delete);
    void remove(int row);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) = 0;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const = 0;
    virtual QVariant data(const QModelIndex& index, int role) const = 0;
    virtual void menu(QMenu& menu, View* tv) = 0;

    virtual int32_t id() const { return id__; }
    virtual void setId(int32_t id) { id__ = id; }

    QModelIndex index(int column = 0) const;

    const QStringList sideStrList{QObject::tr("Top|Bottom").split('|')};
    const Type type_;

protected:
    int32_t id__{-1};
    Node* parent_{nullptr};
    mvector<std::unique_ptr<Node, Deleter>> childs;
};

} // namespace FileTree

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
