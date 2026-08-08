/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "svg_node.h"

#include "ft_view.h"
#include "project.h"
#include "svg_file.h"

#include <QMenu>

namespace Svg {

Node::Node(File* file)
    : FileTree::Node{FileTree::File}
    , file{file} {
}

Node::~Node() { App::project().deleteFile(file->id()); }

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch(role) {
    case Qt::CheckStateRole:
        file->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
        return true;
    case Qt::EditRole:
        if(index.column() == FileTree::Column::Side) {
            file->setSide(static_cast<Side>(value.toBool()));
            return true;
        }
        break;
    case FileTree::Select:
        for(auto ig: file->itemGroups())
            ig->setZValue((value.toBool() ? +(file->id() + 1) : -(file->id() + 1)) * 1000);
        return true;
    }
    return {};
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible: return f | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side            : return f | Qt::ItemIsEditable;
    default                                : return f;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const {
    if(!file) return {};
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole   : return file->shortName();
        case Qt::ToolTipRole   : return QString{file->shortName() + u'\n' + file->name()};
        case Qt::CheckStateRole: return file->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole: return file->icon();
        case FileTree::Id      : return id();
        default                : return {};
        }
    case FileTree::Column::Side:
        switch(role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: return sideStrList[file->side()];
        case Qt::EditRole   : return static_cast<bool>(file->side());
        case FileTree::Id   : return id();
        default             : return {};
        }
    default: return {};
    }
}

void Node::menu(QMenu& menu, FileTree::View* tv) {
    menu.addAction(QIcon::fromTheme(u"hint"_s), QObject::tr("&Hide other"), tv, &FileTree::View::hideOther);
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme(u"document-close"_s), QObject::tr("&Close"), tv, &FileTree::View::closeFile);
}

int32_t Node::id() const { return file->id(); }

} // namespace Svg
