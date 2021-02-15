// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gbrnode.h"
#include "compdialog.h"
#include "ft_view.h"
#include "gbrfile.h"
#include "gbrh.h"
#include "scene.h"

#include <sourcedialog.h>

#include <QAction>
#include <QDialog>
#include <QFileInfo>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

#include "leakdetector.h"

namespace Gerber {

QTimer Node::m_decorationTimer;

Node::Node(File* file, int* id)
    : FileTree::Node(id, FileTree::File)
    , file(file)
{
    connect(&m_decorationTimer, &QTimer::timeout, this, &Node::repaint);
    m_decorationTimer.start(500);
}

Node::~Node() { m_decorationTimer.start(100); }

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        switch (role) {
        case Qt::CheckStateRole:
            file->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case FileTree::Column::Side:
        switch (role) {
        case Qt::EditRole:
            file->setSide(static_cast<Side>(value.toBool()));
            return true;
        default:
            return false;
        }
    case FileTree::Column::ItemsType:
        switch (role) {
        case Qt::EditRole:
            qDebug() << role << value;
            file->setItemType(static_cast<File::ItemsType>(value.toInt()));
            emit App::fileModel()->dataChanged(this->index(), this->index(), { Qt::DecorationRole });
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
    switch (FileTree::Column(index.column())) {
    case FileTree::Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case FileTree::Column::Side:
        return itemFlag | Qt::ItemIsEditable;
    case FileTree::Column::ItemsType:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const
{
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
            //            if (file->color() == QColor())
            //                m_decorationTimer.start(500);
            switch (file->itemsType()) {
            case File::ApPaths:
                return decoration(file->color(), 'A');
            case File::Components:
                return decoration(file->color(), 'C');
            default:
                return decoration(file->color());
            }
        case FileTree::Id:
            return *m_id;
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
        case FileTree::Id:
            return *m_id;
        default:
            return QVariant();
        }
    case FileTree::Column::ItemsType:
        switch (role) {
        case Qt::DisplayRole:
            return file->displayedTypes().at(file->itemsType()).shortActName();
        case Qt::ToolTipRole:
            return file->displayedTypes().at(file->itemsType()).actToolTip;
        case Qt::EditRole:
            return file->displayedTypes().at(file->itemsType()).id;
        case FileTree::Id:
            return *m_id;
        default:
            return QVariant();
        }
    default:
        break;
    }
    return QVariant();
}

QTimer* Node::decorationTimer() { return &m_decorationTimer; }

void Node::repaint() const
{
    if (!m_parent)
        return;
    const int count = m_parent->childCount();
    const int k = static_cast<int>((count > 1) ? (200.0 / (count - 1)) * row() : 0);
    file->setColor(QColor::fromHsv(k, 255, 255, 150));
    emit App::fileModel()->dataChanged(index(0), index(0), { Qt::DecorationRole });
}

void Node::menu(QMenu& menu, FileTree::View* tv) const
{
    menu.addAction(QIcon::fromTheme("hint"), GbrObj::tr("&Hide other"), tv, &FileTree::View::hideOther);
    menu.setToolTipDuration(0);
    menu.setToolTipsVisible(true);
    if (!dialog)
        menu.addAction(QIcon(), GbrObj::tr("&Show source"), [tv, this] {
            dialog = new SourceDialog(file, new SyntaxHighlighter(nullptr));
            dialog->restoreGeometry(tv->saveGeometry());
            dialog->move(tv->mapToGlobal({}));
            dialog->setObjectName(QString::fromUtf8("dialog"));
            if constexpr (true) {
                connect(dialog, &QDialog::finished, [this] {qDebug(""); dialog->deleteLater(), dialog = {}; });
                dialog->show();
            } else {
                dialog->exec();
                delete dialog;
            }
        });
    if (!file->itemGroup(File::Components)->empty()) {
        menu.addAction(QIcon(), GbrObj::tr("Show &Components"), [this, tv] {
            ComponentsDialog dialog(tv);
            dialog.setFile(*m_id);
            dialog.exec();
        });
    }
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("document-close"), GbrObj::tr("&Close"), tv, &FileTree::View::closeFile);
}
}
