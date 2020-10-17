// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcnode.h"
#include "gcfile.h"
#include "gch.h"
#include "project.h"
#include "qboxlayout.h"
#include <QDialog>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QTextBrowser>

#include <filetree/treeview.h>

#include "settings.h"

#include "leakdetector.h"

namespace GCode {
Node::Node(int id)
    : AbstractNode(id)
{
    file()->itemGroup()->addToScene();
    file()->itemGroup()->setVisible(true);
}

bool Node::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            file()->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        case Qt::EditRole:
            file()->setFileName(value.toString());
            return true;
            //            if (auto text = dynamic_cast<Text*>(shape()); text)
            //                text->setText(value.toString());
        default:
            return false;
        }
    case 1:
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

QVariant Node::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            if (file()->shortName().endsWith(GlobalSettings::gcFileExtension()))
                return file()->shortName();
            else
                return file()->shortName() + QStringList({ "_TS", "_BS" })[file()->side()];
        case Qt::EditRole:
            return file()->shortName();
        case Qt::ToolTipRole:
            return file()->shortName() + "\n" + file()->name();
        case Qt::CheckStateRole:
            return file()->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            switch (static_cast<int>(App::project()->file<GCode::File>(m_id)->gtype())) {
            case GCode::Profile:
                return QIcon::fromTheme("profile-path");
            case GCode::Pocket:
                return QIcon::fromTheme("pocket-path");
            case GCode::Voronoi:
                return QIcon::fromTheme("voronoi-path");
            case GCode::Thermal:
                return QIcon::fromTheme("thermal-path");
            case GCode::Drill:
                return QIcon::fromTheme("drill-path");
            case GCode::Raster:
            case GCode::LaserHLDI:
                return QIcon::fromTheme("raster-path");
            }
            return QIcon();
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case 1:
        switch (role) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return sideStrList[file()->side()];
        case Qt::EditRole:
            return static_cast<bool>(file()->side());
        default:
            return QVariant();
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
        if (file()->shortName().contains(GlobalSettings::gcFileExtension()))
            return itemFlag | Qt::ItemIsUserCheckable;
        return itemFlag | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    case 1: {
        if (file()->shortName().contains(GlobalSettings::gcFileExtension()))
            return itemFlag;
        return itemFlag | Qt::ItemIsEditable;
    }
    default:
        return itemFlag;
    }
}
void Node::menu(QMenu* menu, TreeView* tv) const
{
    menu->addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"), tv, &TreeView::hideOther);
    menu->addAction(QIcon::fromTheme("document-save"), QObject::tr("&Save Toolpath"), tv, &TreeView::saveGcodeFile);
    menu->addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete Toolpath"), tv, &TreeView::closeFile);
    menu->addAction(QIcon(), QObject::tr("&Show source"), [this] {
        QDialog* dialog = new QDialog;
        dialog->setObjectName(QString::fromUtf8("dialog"));
        dialog->resize(600, 600);
        //Dialog->resize(400, 300);
        QVBoxLayout* verticalLayout = new QVBoxLayout(dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        QTextBrowser* textBrowser = new QTextBrowser(dialog);
        textBrowser->setFont(QFont("Consolas"));
        /*auto gch =*/new GCH(textBrowser->document());
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        verticalLayout->addWidget(textBrowser);
        for (const QString& str : App::project()->file(m_id)->lines())
            textBrowser->append(str);
        dialog->exec();
        delete dialog;
    });
}
}
