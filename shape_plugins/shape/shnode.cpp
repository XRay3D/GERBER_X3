// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "shnode.h"

#include "shape.h"

#include "ft_view.h"
#include "graphicsitem.h"

#include <QMenu>

namespace Shapes {

Node::Node(Shape* shape, int* id)
    : FileTree::Node(id, FileTree::Shape)
    , shape(shape)
{
    //    shape()->setNode(this);
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return shape->setData(index, value, role);
    //    switch (FileTree::Column(index.column())) {
    //    case FileTree::Column::NameColorVisible:
    //        switch (role) {
    //        case Qt::CheckStateRole:
    //            shape()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
    //            return true;
    //        case Qt::EditRole:
    //            if (auto text = dynamic_cast<Text*>(shape()); text)
    //                text->setText(value.toString());
    //            return true;
    //        }
    //        return false;
    //    case FileTree::Column::SideType:
    //        if (auto text = dynamic_cast<Text*>(shape()); text && role == Qt::EditRole) {
    //            text->setSide(static_cast<Side>(value.toBool()));
    //            return true;
    //        }
    //        return false;
    //    default:
    //        return false;
    //    }
}
QVariant Node::data(const QModelIndex& index, int role) const
{
    return shape->data(index, role);
    //    switch (FileTree::Column(index.column())) {
    //    case FileTree::Column::NameColorVisible:
    //        switch (role) {
    //        case Qt::DisplayRole:
    //            if (static_cast<GiType>(shape()->type()) == GiType::ShText)
    //                return QString("%1 (%2, %3)")
    //                    .arg(shape()->name())
    //                    .arg(m_id)
    //                    .arg(static_cast<Text*>(shape())->text());
    //            else
    //                return QString("%1 (%2)")
    //                    .arg(shape()->name())
    //                    .arg(m_id);
    //            //        case Qt::ToolTipRole:
    //            //            return file()->shortName() + "\n" + file()->name();
    //        case Qt::CheckStateRole:
    //            return shape()->isVisible() ? Qt::Checked : Qt::Unchecked;
    //        case Qt::DecorationRole:
    //            return shape()->icon();
    //        case FileTree::Id:
    //            return *m_id;
    //        case Qt::EditRole:
    //            if (static_cast<GiType>(shape()->type()) == GiType::ShText)
    //                return static_cast<Text*>(shape())->text();
    //            return QVariant();
    //        default:
    //            return QVariant();
    //        }
    //    case FileTree::Column::SideType:
    //        if (auto text = dynamic_cast<Text*>(shape()); text) {
    //            switch (role) {
    //            case Qt::DisplayRole:
    //            case Qt::ToolTipRole:
    //                return sideStrList[text->side()];
    //            case Qt::EditRole:
    //                return static_cast<bool>(text->side());
    //            default:
    //                return QVariant();
    //            }
    //        }
    //    default:
    //        return QVariant();
    //    }
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const
{
    return shape->flags(index);
    //    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    //    switch (FileTree::Column(index.column())) {
    //    case FileTree::Column::NameColorVisible:
    //        return itemFlag | Qt::ItemIsUserCheckable
    //            | (static_cast<GiType>(shape()->type()) == GiType::ShText
    //                    ? Qt::ItemIsEditable
    //                    : Qt::NoItemFlags);
    //    case FileTree::Column::SideType:
    //        return itemFlag
    //            | (static_cast<GiType>(shape()->type()) == GiType::ShText
    //                    ? Qt::ItemIsEditable
    //                    : Qt::NoItemFlags);
    //    default:
    //        return itemFlag;
    //    }
}

void Node::menu(QMenu& menu, FileTree::View* tv) const
{
    shape->menu(menu, tv);
    //    menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete object \"%1\"").arg(shape()->name()), [this] {
    //        App::fileModel()->removeRow(row(), index().parent());
    //    });
    //    if (static_cast<GiType>(shape()->type()) == GiType::ShText) {
    //        menu.addAction(QIcon::fromTheme("draw-text"), QObject::tr("&Edit Text"), [this, tv] {
    //            ShTextDialog dlg({ static_cast<Text*>(shape()) }, tv);
    //            dlg.exec();
    //        });
    //    }
}

}
