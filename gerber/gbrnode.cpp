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
#include "gbrnode.h"
#include "compdialog.h"
#include "project.h"
#include "qboxlayout.h"
#include "scene.h"
#include <QAction>
#include <QDialog>
#include <QFileInfo>
#include <QMenu>
#include <QPainter>
#include <QTextBrowser>
#include <QTimer>
#include <filetree/treeview.h>

#include "leakdetector.h"

namespace Gerber {

QTimer Node::m_decorationTimer;

Node::Node(int id)
    : AbstractNode(id)
{
    App::project()->file<File>(m_id)->addToScene();
    connect(&m_decorationTimer, &QTimer::timeout, this, &Node::repaint);
    m_decorationTimer.setSingleShot(true);
    m_decorationTimer.start(100);
}

Node::~Node()
{
    m_decorationTimer.start(10);
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        switch (role) {
        case Qt::CheckStateRole:
            file()->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case Column::SideType:
        switch (role) {
        case Qt::EditRole:
            file()->setSide(static_cast<Side>(value.toBool()));
            return true;
        default:
            return false;
        }
    case Column::ItemsType:
        switch (role) {
        case Qt::EditRole:
            qDebug() << __FUNCTION__ << role << value;
            App::project()->file<File>(m_id)->setItemType(static_cast<File::ItemsType>(value.toInt()));
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Column::SideType:
        return itemFlag | Qt::ItemIsEditable;
    case Column::ItemsType:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const
{
    switch (static_cast<Column>(index.column())) {
    case Column::NameColorVisible:
        switch (role) {
        case Qt::DisplayRole:
            return file()->shortName();
        case Qt::ToolTipRole:
            return file()->shortName() + "\n" + file()->name();
        case Qt::CheckStateRole:
            return file()->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            switch (App::project()->file<File>(m_id)->itemsType()) {
            case File::ApPaths:
                return decoration(file()->color(), 'A');
            case File::Components:
                return decoration(file()->color(), 'C');
            default:
                return decoration(file()->color());
            }
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case Column::SideType:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return sideStrList[file()->side()];
        case Qt::EditRole:
            return static_cast<bool>(file()->side());
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case Column::ItemsType:
        switch (role) {
        case Qt::DisplayRole:
            return file()->displayedTypes().at(App::project()->file<File>(m_id)->itemsType()).actName;
        case Qt::ToolTipRole:
            return file()->displayedTypes().at(App::project()->file<File>(m_id)->itemsType()).actName;
        case Qt::EditRole:
            return file()->displayedTypes().at(App::project()->file<File>(m_id)->itemsType()).id;
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    default:
        break;
    }
    return QVariant();
}

QTimer* Node::decorationTimer() { return &m_decorationTimer; }

void Node::repaint()
{
    const int count = m_parentItem->childCount();
    const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * row() : 0);
    App::project()->file<File>(m_id)->setColor(QColor::fromHsv(k, 255, 255, 150));
    App::scene()->update();
}

void Node::menu(QMenu* menu, FileTreeView* tv) const
{
    menu->addAction(QIcon::fromTheme("hint"), tr("&Hide other"), tv, &FileTreeView::hideOther);
    menu->setToolTipDuration(0);
    menu->setToolTipsVisible(true);
    //    File* file = App::project()->file<File>(m_id);
    //    { //QActionGroup
    //        menu->addSeparator();
    //        QActionGroup* group = new QActionGroup(menu);
    //        if (file->itemGroup(File::ApPaths)->size()) {
    //            auto action = menu->addAction(tr("&Aperture paths"),
    //                [=](bool checked) { file->setItemType(static_cast<File::ItemsType>(checked * File::ApPaths)); });
    //            action->setCheckable(true);
    //            action->setChecked(file->itemsType() == File::ApPaths);
    //            action->setToolTip(tr("Displays only aperture paths of copper\n"
    //                                  "without width and without contacts."));
    //            action->setActionGroup(group);
    //        }
    //        if (file->itemGroup(File::Components)->size()) {
    //            auto action = menu->addAction(tr("&Components"),
    //                [=](bool checked) { file->setItemType(static_cast<File::ItemsType>(checked * File::Components)); });
    //            action->setCheckable(true);
    //            action->setChecked(file->itemsType() == File::Components);
    //            //            action->setToolTip("Displays only aperture paths of copper\n"
    //            //                               "without width and without contacts.");
    //            action->setActionGroup(group);
    //        }
    //        if (file->itemGroup(File::Normal)->size()) {
    //            auto action = menu->addAction(tr("&Normal"),
    //                [=](bool checked) { file->setItemType(static_cast<File::ItemsType>(checked * File::Normal)); });
    //            action->setCheckable(true);
    //            action->setChecked(file->itemsType() == File::Normal);
    //            //            action->setToolTip("Displays only aperture paths of copper\n"
    //            //                               "without width and without contacts.");
    //            action->setActionGroup(group);
    //        }
    //        menu->addSeparator();
    //    }

    menu->addAction(QIcon(), tr("&Show source"), [this] {
        QDialog* dialog = new QDialog;
        dialog->setObjectName(QString::fromUtf8("dialog"));
        dialog->resize(600, 600);
        QVBoxLayout* verticalLayout = new QVBoxLayout(dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        QTextBrowser* textBrowser = new QTextBrowser(dialog);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        verticalLayout->addWidget(textBrowser);
        for (const QString& str : App::project()->file(m_id)->lines())
            textBrowser->append(str);
        dialog->exec();
        delete dialog;
    });

    if (!App::project()->file<File>(m_id)->itemGroup(File::Components)->isEmpty()) {
        menu->addAction(QIcon(), tr("Show &Components"), [this, tv] {
            ComponentsDialog dialog(tv);
            dialog.setFile(m_id);
            dialog.exec();
        });
    }
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("document-close"), tr("&Close"), tv, &FileTreeView::closeFile);
}
}
