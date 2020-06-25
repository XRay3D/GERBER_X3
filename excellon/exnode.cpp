#include "exnode.h"
#include "excellondialog.h"
#include "gbrnode.h"
#include "project.h"
#include <QFileInfo>
#include <app.h>
#include <exfile.h>
#include <forms/drillform/drillform.h>
#include <mainwindow.h>

ExcellonNode::ExcellonNode(int id)
    : AbstractNode(id)
{
    file()->itemGroup()->addToScene();
}

bool ExcellonNode::setData(const QModelIndex& index, const QVariant& value, int role)
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

Qt::ItemFlags ExcellonNode::flags(const QModelIndex& index) const
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

QVariant ExcellonNode::data(const QModelIndex& index, int role) const
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
                return tbStrList[file()->side()];
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

void ExcellonNode::menu(QMenu* menu, TreeView* tv) const
{
    menu->addAction(QIcon::fromTheme("hint"), tr("&Hide other"), tv, &TreeView::hideOther);
    if (!m_exFormatDialog) {
        menu->addAction(QIcon::fromTheme("configure-shortcuts"), tr("&Edit Format"), [this] {
            if (App::drillForm())
                App::drillForm()->on_pbClose_clicked();
            m_exFormatDialog = new ExcellonDialog(App::project()->file<Excellon::File>(m_id));
            connect(m_exFormatDialog, &ExcellonDialog::destroyed, [&] { m_exFormatDialog = nullptr; });
            m_exFormatDialog->show();
        });
    }
    menu->addAction(QIcon::fromTheme("document-close"), tr("&Close"), tv, &TreeView::closeFile);
}
