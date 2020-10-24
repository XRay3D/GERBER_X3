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
#include "exnode.h"

#include "excellondialog.h"
#include "exfile.h"
#include <QIcon>
#include <QMenu>
#include <filetree/treeview.h>
#include <forms/drillform/drillform.h>

#include "leakdetector.h"

namespace Excellon {
Node::Node(int id)
    : AbstractNode(id)
{
    file()->itemGroup()->addToScene();
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case Name:
        switch (role) {
        case Qt::CheckStateRole:
            file()->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case Layer:
        switch (role) {
        case Qt::EditRole:
            file()->setSide(static_cast<Side>(value.toBool()));
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
    switch (index.column()) {
    case Name:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Layer:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const
{
    if (file())
        switch (index.column()) {
        case Name:
            switch (role) {
            case Qt::DisplayRole:
                return file()->shortName();
            case Qt::ToolTipRole:
                return file()->shortName() + "\n"
                    + file()->name();
            case Qt::CheckStateRole:
                return file()->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
            case Qt::DecorationRole:
                return QIcon::fromTheme("drill-path");
            case Qt::UserRole:
                return m_id;
            default:
                return QVariant();
            }
        case Layer:
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
        default:
            return QVariant();
        }
    return QVariant();
}

void Node::menu(QMenu* menu, TreeView* tv) const
{
    menu->addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"), tv, &TreeView::hideOther);
    if (!m_exFormatDialog) {
        menu->addAction(QIcon::fromTheme("configure-shortcuts"), QObject::tr("&Edit Format"), [this] {
            if (App::drillForm())
                App::drillForm()->on_pbClose_clicked();
            m_exFormatDialog = new ExcellonDialog(App::project()->file<Excellon::File>(m_id));
            QObject::connect(m_exFormatDialog, &ExcellonDialog::destroyed, [&] { m_exFormatDialog = nullptr; });
            m_exFormatDialog->show();
        });
    }
    menu->addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close"), tv, &TreeView::closeFile);
}
}
