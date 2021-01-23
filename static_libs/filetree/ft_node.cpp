// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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

Node::Node(int* id, Type type)
    : sideStrList(QObject::tr("Top|Bottom").split('|'))
    , type(type)
    , m_id(id)
{
}

Node::~Node()
{
    if (m_id && *m_id > -1) {
        switch (type) {
        case File:
            App::project()->deleteFile(*m_id);
            break;
        case Shape:
            App::project()->deleteShape(*m_id);
            break;
        default:
            break;
        }
    }
    childs.clear();
}

Node* Node::child(int row) const
{
    if (row < 0 || row >= static_cast<int>(childs.size()))
        return nullptr;
    return childs[row].get();
}

Node* Node::parent() const { return m_parent; }

void Node::setChild(int row, Node* item)
{
    if (item) {
        item->m_parent = this;
        if (row < static_cast<int>(childs.size()))
            childs[row].reset(item);
    }
}

int Node::childCount() const
{
    return static_cast<int>(childs.size());
}

int Node::row() const
{
    if (m_parent)
        return m_parent->childs.indexOf(this);
    //    for (int i = 0, size = m_parent->childs.size(); i < size; ++i)
    //        if (m_parent->childs[i].get() == this)
    //            return i;
    return -1;
}

void Node::addChild(Node* item)
{
    item->m_parent = this;
    childs.resize(childs.size() + 1);
    childs.back().reset(item);
}

void Node::remove(int row) { childs.takeAt(row); }

QModelIndex Node::index(int column) const
{
    return App::fileModel()->createIndex(row(), column, reinterpret_cast<quintptr>(this));
}

void Node::setId(int* id) { m_id = id; }

}
