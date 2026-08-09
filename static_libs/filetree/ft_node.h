/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "serial.h"
#include "utils.h"
#include <memory>

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

// Узел дерева файлов -- целиком рантайм-структура (родитель, дети, модель):
// движок сериализации в него не заходит, хотя AbstractShape его наследует.
class[[= Serial::skip]] Node {
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

    // Перенос узла между папками. remove() ребёнка УДАЛЯЕТ (владение живёт в
    // unique_ptr), поэтому забирать его надо отдельной парой: takeChild отдаёт
    // владение наружу, adoptChild принимает вместе с прежней политикой удаления.
    std::unique_ptr<Node, Deleter> takeChild(int row);
    void adoptChild(std::unique_ptr<Node, Deleter>&& child);

    virtual QVariant data(const QModelIndex& index, int role) const = 0;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) = 0;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const = 0;
    virtual void menu(QMenu& menu, View* tv) = 0;

    // Двойной клик по колонке имени. Вернуть true, если узел обработал его сам;
    // иначе View гасит остальные файлы (поведение по умолчанию для всех файлов,
    // кроме УП -- та открывает форму своего плагина).
    virtual bool doubleClicked(View* /*tv*/) { return false; }

    virtual int32_t id() const { return id__; }
    virtual void setId(int32_t id) { id__ = id; }

    QModelIndex index(int column = 0) const;

    const QStringList sideStrList{QObject::tr("Top|Bottom").split(u'|')};
    const Type type_;

protected:
    int32_t id__{-1};
    Node* parent_{nullptr};
    std::vector<std::unique_ptr<Node, Deleter>> childs;
};

} // namespace FileTree
