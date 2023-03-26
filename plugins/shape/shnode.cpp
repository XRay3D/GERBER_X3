// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "shnode.h"

#include "shape.h"

#include "ft_view.h"
#include "gi.h"

#include <QMenu>

namespace Shapes {

Node::Node(Shape* shape)
    : FileTree_::Node(FileTree_::Shape)
    , shape(shape) {
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {
    return shape->setData(index, value, role);
    //    switch (FileTree_::Column(index.column())) {
    //    case FileTree_::Column::NameColorVisible:
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
    //    case FileTree_::Column::SideType:
    //        if (auto text = dynamic_cast<Text*>(shape()); text && role == Qt::EditRole) {
    //            text->setSide(static_cast<Side>(value.toBool()));
    //            return true;
    //        }
    //        return false;
    //    default:
    //        return false;
    //    }
}
QVariant Node::data(const QModelIndex& index, int role) const {
    return shape->data(index, role);
    //    switch (FileTree_::Column(index.column())) {
    //    case FileTree_::Column::NameColorVisible:
    //        switch (role) {
    //        case Qt::DisplayRole:
    //            if (shape()->type() == GiType::ShText)
    //                return QString("%1 (%2, %3)")
    //                    .arg(shape()->name())
    //                    .arg(id_)
    //                    .arg(static_cast<Text*>(shape())->text());
    //            else
    //                return QString("%1 (%2)")
    //                    .arg(shape()->name())
    //                    .arg(id_);
    //            //        case Qt::ToolTipRole:
    //            //            return file()->shortName() + "\n" + file()->name();
    //        case Qt::CheckStateRole:
    //            return shape()->isVisible() ? Qt::Checked : Qt::Unchecked;
    //        case Qt::DecorationRole:
    //            return shape()->icon();
    //        case FileTree_::Id:
    //            return *id_;
    //        case Qt::EditRole:
    //            if (shape()->type() == GiType::ShText)
    //                return static_cast<Text*>(shape())->text();
    //            return QVariant();
    //        default:
    //            return QVariant();
    //        }
    //    case FileTree_::Column::SideType:
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

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    return shape->flags(index);
    //    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    //    switch (FileTree_::Column(index.column())) {
    //    case FileTree_::Column::NameColorVisible:
    //        return itemFlag | Qt::ItemIsUserCheckable
    //            | (shape()->type() == GiType::ShText
    //                    ? Qt::ItemIsEditable
    //                    : Qt::NoItemFlags);
    //    case FileTree_::Column::SideType:
    //        return itemFlag
    //            | (shape()->type() == GiType::ShText
    //                    ? Qt::ItemIsEditable
    //                    : Qt::NoItemFlags);
    //    default:
    //        return itemFlag;
    //    }
}

void Node::menu(QMenu& menu, FileTree_::View* tv) const {
    shape->menu(menu, tv);
    //    menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete object \"%1\"").arg(shape()->name()), [this] {
    //        App::fileModel()->removeRow(row(), index().parent());
    //    });
    //    if (shape()->type() == GiType::ShText) {
    //        menu.addAction(QIcon::fromTheme("draw-text"), QObject::tr("&Edit Text"), [this, tv] {
    //            ShTextDialog dlg({ static_cast<Text*>(shape()) }, tv);
    //            dlg.exec();
    //        });
    //    }
}

int Node::id() const { return shape->id(); }

} // namespace Shapes
