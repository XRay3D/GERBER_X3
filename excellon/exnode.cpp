#include "exnode.h"
#include "gbrnode.h"
#include "project.h"
#include <QFileInfo>
#include <exfile.h>
#include <mainwindow.h>

ExcellonNode::ExcellonNode(int id)
    : AbstractNode(id)
{
    Project::instance()->file(m_id)->itemGroup()->addToScene();
}

bool ExcellonNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case Name:
        switch (role) {
        case Qt::CheckStateRole:
            Project::instance()->file(m_id)->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
        default:
            return false;
        }
    case Layer:
        switch (role) {
        case Qt::EditRole:
            Project::instance()->file(m_id)->setSide(static_cast<Side>(value.toBool()));
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
    if (Project::instance()->file(m_id))
        switch (index.column()) {
        case Name:
            switch (role) {
            case Qt::DisplayRole:
                return Project::instance()->file(m_id)->shortName();
            case Qt::ToolTipRole:
                return Project::instance()->file(m_id)->shortName() + "\n"
                    + Project::instance()->file(m_id)->name();
            case Qt::CheckStateRole:
                return Project::instance()->file(m_id)->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
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
                return tbStrList[Project::instance()->file(m_id)->side()];
            case Qt::EditRole:
                return static_cast<bool>(Project::instance()->file(m_id)->side());
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
