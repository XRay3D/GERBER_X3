/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "g2_node.h"
#include "g2_editor.h"
#include "g2_file.h"

#include "ft_view.h"

#include <QColorDialog>
#include <QMenu>

namespace Gerber2 {

Node::Node(File* file)
    : FileTree::Node{FileTree::File}
    , file{file} { }

Node::~Node() { App::project().deleteFile(file->id()); }

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch(role) {
    case Qt::CheckStateRole:
        file->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
        return true;
    case Qt::EditRole:
        switch(FileTree::Column(index.column())) {
        case FileTree::Column::Side:
            file->setSide(static_cast<Side>(value.toBool()));
            return true;
        case FileTree::Column::ItemsType:
            file->setItemType(value.toInt());
            emit App::fileModel().dataChanged(this -> index(), this->index(), {Qt::DecorationRole});
            return true;
        default: break;
        }
        break;
    case FileTree::Select:
        for(auto* ig: file->itemGroups())
            ig->setZValue((value.toBool() ? +(file->id() + 1) : -(file->id() + 1)) * 1000);
        return true;
    }
    return {};
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible: return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side            : return itemFlag | Qt::ItemIsEditable;
    case FileTree::Column::ItemsType       : return itemFlag | Qt::ItemIsEditable;
    default                                : return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const {
    switch(FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch(role) {
        case Qt::DisplayRole   : return file->shortName();
        case Qt::ToolTipRole   : return {file->shortName() + u'\n' + file->name()};
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
    case FileTree::Column::ItemsType:
        switch(role) {
        case Qt::DisplayRole: return file->displayedTypes().at(file->itemsType()).shortActName();
        case Qt::ToolTipRole: return file->displayedTypes().at(file->itemsType()).actToolTip;
        case Qt::EditRole   : return file->displayedTypes().at(file->itemsType()).id;
        case FileTree::Id   : return id();
        default             : return {};
        }
    default: break;
    }
    return {};
}

int32_t Node::id() const { return file->id(); }

void Node::menu(QMenu& menu, FileTree::View* tv) {
    menu.addAction(QIcon::fromTheme(u"hint"_s), QObject::tr("&Hide other"), tv, &FileTree::View::hideOther);
    menu.addAction(QIcon::fromTheme(u"document-edit"_s), QObject::tr("&Edit source…"), [this, tv] {
        Editor{file, tv}.exec();
    });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme(u"color-management"_s), QObject::tr("Change color"), [this, tv] {
        QColorDialog cd{tv};
        cd.setCurrentColor(file->color());
        if(cd.exec()) {
            auto color = cd.currentColor();
            color.setAlpha(150);
            file->setColor(color);
            file->setUserColor(true);
        }
    });
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme(u"document-close"_s), QObject::tr("&Close"), tv, &FileTree::View::closeFile);
}

} // namespace Gerber2
