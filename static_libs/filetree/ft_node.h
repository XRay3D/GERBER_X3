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

#include <memory>
#include <mvector.h>

#include <QChar>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QVariant>

class QMenu;

inline QPixmap decoration(QColor color, QChar chr = {}) {

    QPixmap pixmap(22, 22);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    color.setAlpha(255);
    p.setBrush(color);
    p.drawRect(2, 2, 18, 18);
    if (!chr.isNull()) {
        QFont f;
        f.setBold(true);
        f.setPixelSize(18);
        p.setFont(f);
        // p.setPen(Qt::white);
        p.drawText(QRect(2, 2, 18, 18), Qt::AlignCenter, {chr});
    }
    return pixmap;
}

namespace FileTree_ {

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
    Shape,
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

    void addChild(Node* item);
    void remove(int row);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) = 0;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const = 0;
    virtual QVariant data(const QModelIndex& index, int role) const = 0;
    virtual void menu(QMenu& menu, View* tv) const = 0;

    virtual int id() const = 0; //{ return id_; }
    virtual void setId(int id) { id__ = id; }

    QModelIndex index(int column = 0) const;

    const QStringList sideStrList {QObject::tr("Top|Bottom").split('|')};
    const Type type;

protected:
    int id__ {-1};
    Node* parent_ = nullptr;
    mvector<std::unique_ptr<Node>> childs;
};

} // namespace FileTree_
