// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "shnode.h"
#include "gch.h"
#include "project.h"

#include <QDialog>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QTextBrowser>
#include <qboxlayout.h>

#include <filetree/treeview.h>

ShNode::ShNode(int id)
    : AbstractNode(id, 1)
{
}

bool ShNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            shape()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
        //    case 1:
        //        switch (role) {
        //        case Qt::EditRole:
        //            //            file()->setSide(static_cast<Side>(value.toBool()));
        //            //            return true;
        //        default:
        //            return false;
        //        }
    default:
        return false;
    }
}

Qt::ItemFlags ShNode::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable /*| Qt::ItemIsDragEnabled*/;
    switch (index.column()) {
    case 0:
        return itemFlag | Qt::ItemIsUserCheckable;
    case 1: {
        return itemFlag | Qt::ItemIsEditable;
    }
    default:
        return itemFlag;
    }
}

QVariant ShNode::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            return shape()->name();
            //        case Qt::ToolTipRole:
            //            return file()->shortName() + "\n" + file()->name();
        case Qt::CheckStateRole:
            return shape()->isVisible() ? Qt::Checked : Qt::Unchecked;
        case Qt::DecorationRole:
            switch (shape()->type()) {
            case GiShapeC:
                return QIcon::fromTheme("draw-ellipse");
            case GiShapeR:
                return QIcon::fromTheme("draw-rectangle");
            case GiShapeL:
                return QIcon::fromTheme("draw-line");
            }
            return QIcon();
        case Qt::UserRole:
            return m_id;
        default:
            return QVariant();
        }
    case 1:
        //                switch (role) {
        //                case Qt::DisplayRole:
        //                case Qt::ToolTipRole:
        //                    return tbStrList[file()->side()];
        //                case Qt::EditRole:
        //                    return static_cast<bool>(file()->side());
        //                default:
        //                    return QVariant();
        //                }
    default:
        return QVariant();
    }
}

void ShNode::menu(QMenu* menu, TreeView* tv) const
{
    //    menu->addAction(QIcon::fromTheme("hint"), QObject::tr("&Hide other"), tv, &TreeView::hideOther);
    //    menu->addAction(QIcon::fromTheme("document-save"), QObject::tr("&Save Toolpath"), tv, &TreeView::saveGcodeFile);
    menu->addAction(QIcon::fromTheme("edit-delete"), QObject::tr("&Delete Toolpath"), tv, &TreeView::closeFile);
    //    menu->addAction(QIcon(), QObject::tr("&Show source"), [this] {
    //        QDialog* dialog = new QDialog;
    //        dialog->setObjectName(QString::fromUtf8("dialog"));
    //        dialog->resize(600, 600);
    //        //Dialog->resize(400, 300);
    //        QVBoxLayout* verticalLayout = new QVBoxLayout(dialog);
    //        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    //        QTextBrowser* textBrowser = new QTextBrowser(dialog);
    //        textBrowser->setFont(QFont("Consolas"));
    //        /*auto gch =*/new GCH(textBrowser->document());
    //        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
    //        verticalLayout->addWidget(textBrowser);
    //        for (const QString& str : App::project()->file(m_id)->lines())
    //            textBrowser->append(str);
    //        dialog->exec();
    //        delete dialog;
    //    });
}
