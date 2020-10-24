// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "foldernode.h"
#include <QIcon>

FolderNode::FolderNode(const QString& name)
    : AbstractNode(-1)
    , name(name)
{
}

QVariant FolderNode::data(const QModelIndex& index, int role) const
{
    if (index.column())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return name;
    case Qt::DecorationRole:
        return QIcon::fromTheme("folder");
        //    case Qt::CheckStateRole:
        //        return checkState;
    default:
        return QVariant();
    }
}

Qt::ItemFlags FolderNode::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled /*| Qt::ItemIsDropEnabled*/;
}

bool FolderNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.column())
        return false;

    switch (role) {
    case Qt::CheckStateRole:
        m_checkState = value.value<Qt::CheckState>();
        return true;
    default:
        return false;
    }
}

void FolderNode::menu(QMenu* /*menu*/, TreeView* /*tv*/) const
{
}
