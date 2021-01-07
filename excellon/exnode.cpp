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

#include "forms/drillform/drillform.h"
#include "treeview.h"

#include <QBoxLayout>
#include <QIcon>
#include <QMenu>
#include <QTextBrowser>

#include "leakdetector.h"

namespace Excellon {
Node::Node(int id)
    : NodeInterface(id)
{
    file()->itemGroup()->addToScene();
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (Column(index.column())) {
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
    default:
        return false;
    }
}

Qt::ItemFlags Node::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
    switch (Column(index.column())) {
    case Column::NameColorVisible:
        return itemFlag | Qt::ItemIsUserCheckable;
    case Column::SideType:
        return itemFlag | Qt::ItemIsEditable;
    default:
        return itemFlag;
    }
}

QVariant Node::data(const QModelIndex& index, int role) const
{
    if (file())
        switch (Column(index.column())) {
        case Column::NameColorVisible:
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
        default:
            return QVariant();
        }
    return QVariant();
}

void Node::menu(QMenu& menu, FileTreeView* tv) const
{
    menu.addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"), tv, &FileTreeView::hideOther);
    menu.addAction(QIcon(), QObject::tr("&Show source"), [this] {
        QDialog* dialog = new QDialog;
        dialog->setObjectName(QString::fromUtf8("dialog"));
        dialog->resize(600, 600);
        //Dialog->resize(400, 300);
        QVBoxLayout* verticalLayout = new QVBoxLayout(dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        QTextBrowser* textBrowser = new QTextBrowser(dialog);
        textBrowser->setFont(QFont("Consolas"));
        //        /*auto gch =*/new GCH(textBrowser->document());
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        verticalLayout->addWidget(textBrowser);
        for (const QString& str : App::project()->file(m_id)->lines())
            textBrowser->append(str);
        dialog->exec();
        delete dialog;
    });
    menu.addSeparator();
    if (!m_exFormatDialog) {
        menu.addAction(QIcon::fromTheme("configure-shortcuts"), QObject::tr("&Edit Format"), [this] {
            //            if (App::drillForm())
            //                App::drillForm()->on_pbClose_clicked();
            m_exFormatDialog = new ExcellonDialog(App::project()->file<Excellon::File>(m_id));
            QObject::connect(m_exFormatDialog, &ExcellonDialog::destroyed, [&] { m_exFormatDialog = nullptr; });
            m_exFormatDialog->show();
        });
    }
    menu.addSeparator();
    menu.addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close"), tv, &FileTreeView::closeFile);
}
}
