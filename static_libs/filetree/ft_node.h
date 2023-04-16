/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

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
    virtual void menu(QMenu& menu, View* tv) const = 0;

    virtual int32_t id() const { return id__; }
    virtual void setId(int32_t id) { id__ = id; }

    QModelIndex index(int column = 0) const;

    const QStringList sideStrList{QObject::tr("Top|Bottom").split('|')};
    const Type type_;

protected:
    int32_t id__{-1};
    Node* parent_ = nullptr;
    mvector<std::unique_ptr<Node, Deleter>> childs;
};

} // namespace FileTree
