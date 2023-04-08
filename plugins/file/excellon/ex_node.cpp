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
 *******************************************************************************/
#include "ex_node.h"

#include "ex_file.h"
#include "ex_formatdialog.h"
#include "ex_highlighter.h"
#include "ft_view.h"
#include "project.h"

#include <QBoxLayout>
#include <QIcon>
#include <QMenu>
#include <QTextBrowser>

namespace Excellon {

Node::Node(File* file)
    : FileTree::Node(FileTree::File)
    , file(file) {
}

Node::~Node() { App::project()->deleteFile(file->id()); }

bool Node::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch (role) {
    case Qt::CheckStateRole:
        file->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
        return true;
    case Qt::EditRole:
        if (index.column() == FileTree::Column::Side) {
            file->setSide(static_cast<Side>(value.toBool()));
            return true;
        }
        break;
    case FileTree::Select:
        for (auto ig : file->itemGroups())
            ig->setZValue((value.toBool() ? +(file->id() + 1) : -(file->id() + 1)) * 1000);
        return true;
    }
    return {};
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const {
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const {
    if (file)
        switch (FileTree::Column(index.column())) {
        case FileTree::Column::NameColorVisible:
            switch (role) {
            case Qt::DisplayRole:
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
                return {};
            }
        case FileTree::Column::Side:
            switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return sideStrList[file->side()];
            case Qt::EditRole:
                return static_cast<bool>(file->side());
            case FileTree::Id:
                return id();
            default:
                return {};
            }
        default:
            return {};
        }
    return {};
}

void Node::menu(QMenu& menu, FileTree::View* tv) const {
    menu.addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"), tv, &FileTree::View::hideOther);
    menu.addAction(QIcon(), QObject::tr("&Show source"), [this] {
        QDialog* dialog = new QDialog;
        dialog->setObjectName(QString::fromUtf8("dialog"));
        dialog->resize(600, 600);
        // Dialog->resize(400, 300);
        QVBoxLayout* verticalLayout = new QVBoxLayout(dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        QTextBrowser* textBrowser = new QTextBrowser(dialog);
        textBrowser->setFont(QFont("JetBrains Mono"));
        new SyntaxHighlighter(textBrowser->document());
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        verticalLayout->addWidget(textBrowser);
        for (const QString& str : file->lines())
            textBrowser->append(str);
        dialog->exec();
        delete dialog;
    });
    menu.addSeparator();
    if (!FormatDialog::showed()) {
        menu.addAction(QIcon::fromTheme("configure-shortcuts"), QObject::tr("&Edit Format"), [this] {
            (new FormatDialog(file))->show();
        });
    }
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close"), tv, &FileTree::View::closeFile);
}

int Node::id() const { return file->id(); }

} // namespace Excellon
