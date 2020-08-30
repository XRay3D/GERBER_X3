// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shnode.h"
#include "filetree/treeview.h"
#include "shtext.h"
#include "shtextdialog.h"
#include <QMenu>

namespace Shapes {
Node::Node(int id)
    : AbstractNode(id, 1)
{
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            shape()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        case Qt::EditRole:
            if (auto text = dynamic_cast<Text*>(shape()); text)
                text->setText(value.toString());
            return true;
        default:
            return false;
        }
    case 1:
        if (auto text = dynamic_cast<Text*>(shape()); text) {
            switch (role) {
            case Qt::EditRole:
                text->setSide(static_cast<Side>(value.toBool()));
                return true;
            default:
                return false;
            }
        }
    default:
        return false;
    }
}
QVariant Node::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            if (shape()->type() == GiShapeT)
                return QString("%1 (%2, %3)")
                    .arg(shape()->name())
                    .arg(m_id)
                    .arg(static_cast<Text*>(shape())->text());
            else
                return QString("%1 (%2)")
                    .arg(shape()->name())
                    .arg(m_id);
            //        case Qt::ToolTipRole:
            //            return file()->shortName() + "\n" + file()->name();
        case Qt::CheckStateRole:
            return shape()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return shape()->icon();
        case Qt::UserRole:
            return m_id;
        case Qt::EditRole:
            if (shape()->type() == GiShapeT)
                return static_cast<Text*>(shape())->text();
            return QVariant();
        default:
            return QVariant();
        }
    case 1:
        if (auto text = dynamic_cast<Text*>(shape()); text) {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return tbStrList[text->side()];
            case Qt::EditRole:
                return static_cast<bool>(text->side());
            default:
                return QVariant();
            }
        }
    default:
        return QVariant();
    }
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable /*| Qt::ItemIsDragEnabled*/;
    switch (index.column()) {
    case 0:
        return itemFlag | Qt::ItemIsUserCheckable
            | (shape()->type() == GiShapeT
                    ? Qt::ItemIsEditable
                    : Qt::NoItemFlags);
    case 1:
        return itemFlag
            | (shape()->type() == GiShapeT
                    ? Qt::ItemIsEditable
                    : Qt::NoItemFlags);
    default:
        return itemFlag;
    }
}

void Node::menu(QMenu* menu, TreeView* tv) const
{
    menu->addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete object \"%1\"").arg(shape()->name()), tv, &TreeView::closeFile);
    if (shape()->type() == GiShapeT)
        menu->addAction(QIcon::fromTheme("draw-text"), QObject::tr("&Edit Text"), [this, tv] {
            ShTextDialog dlg({ static_cast<Text*>(shape()) }, tv);
            dlg.exec();
        });
}
}
