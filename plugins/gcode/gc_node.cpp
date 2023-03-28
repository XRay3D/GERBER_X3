// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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
#include "gc_node.h"
#include "abstract_file.h"
#include "gc_highlighter.h"
#include "project.h"

#include <QFileInfo>
#include <QIcon>
#include <QMenu>

#include "ft_view.h"

namespace GCode {

Node::Node(AbstractFile* file)
    : FileTree::Node(FileTree::File)
    , file(file) {
}

Node::~Node() { App::project()->deleteFile(file->id()); }

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {

    switch (index.column()) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        case Qt::CheckStateRole:
            file->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        case Qt::EditRole:
            file->setFileName(value.toString());
            return true;
        default:;
        }
    case FileTree::Column::Side:
        switch (role) {
        case Qt::EditRole:
            file->setSide(static_cast<Side>(value.toBool()));
            return true;
        default:;
        }
    default:
        if (role == FileTree::Select) {
            file->itemGroup()->setZValue((value.toBool() ? +(file->id() + 1) : -(file->id() + 1)) * 1000);
            return true;
        }
        return false;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const {
    switch (index.column()) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
            //            if (file->shortName().endsWith(Settings::fileExtension()))
            //                return file->shortName();
            //            else
            return file->shortName() + QStringList({"_TS", "_BS"})[file->side()];
        case Qt::EditRole:
            return file->shortName();
        case Qt::ToolTipRole:
            return file->shortName() + "\n" + file->name();
        case Qt::CheckStateRole:
            return file->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            return file->icon();
        case FileTree::Id:
            return id();
        default:
            return QVariant();
        }
    case FileTree::Column::Side:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return sideStrList[file->side()];
        case Qt::EditRole:
            return static_cast<bool>(file->side());
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable /*| Qt::ItemIsDragEnabled*/;
    switch (index.column()) {
    case FileTree::Column::NameColorVisible:
        //        if (file->shortName().endsWith(Settings::fileExtension()))
        //            return itemFlag | Qt::ItemIsUserCheckable;
        return itemFlag | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    case FileTree::Column::Side: {
        //        if (file->shortName().endsWith(Settings::fileExtension()))
        //            return itemFlag;
        return itemFlag | Qt::ItemIsEditable;
    }
    default:
        return itemFlag;
    }
}

void Node::menu(QMenu& menu, FileTree::View* tv) const {
    static std::unordered_map<int, Dialog*> dialog;
    menu.addAction(QIcon::fromTheme("document-save"), QObject::tr("&Save Toolpath"), [tv, this] {
        emit tv->saveGCodeFile(id());
    });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"),
        tv, &FileTree::View::hideOther);
    if (!dialog[id()])
        menu.addAction(QIcon(), QObject::tr("&Show source"), [tv, this] {
            dialog[id()] = new Dialog(file->lines2(), file->name(), tv);
            auto destroy = [this] {
                delete dialog[id()];
                dialog[id()] = nullptr;
            };
            QObject::connect(dialog[id()], &QDialog::finished, destroy);
            dialog[id()]->show();
        });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete Toolpath"), tv, &FileTree::View::closeFile);
}

int Node::id() const { return file->id(); }

} // namespace GCode
