#include "gcnode.h"
#include "project.h"

#include <QFileInfo>
#include <QIcon>

GcodeNode::GcodeNode(int id)
    : AbstractNode(id)
{
    file()->itemGroup()->addToScene();
}

bool GcodeNode::setData(const QModelIndex& index, const QVariant& value, int role)
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::CheckStateRole:
            file()->itemGroup()->setVisible(value.value<Qt::CheckState>() == Qt::Checked);
            return true;
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

Qt::ItemFlags GcodeNode::flags(const QModelIndex& index) const
{
    Qt::ItemFlags itemFlag = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable /*| Qt::ItemIsDragEnabled*/;
    switch (index.column()) {
    case 0:
        return itemFlag | Qt::ItemIsUserCheckable;
    case 1: {
        if (file()->shortName().contains(".tap"))
            return itemFlag;
        return itemFlag | Qt::ItemIsEditable;
    }
    default:
        return itemFlag;
    }
}

QVariant GcodeNode::data(const QModelIndex& index, int role) const
{
    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole: {
            if (file()->shortName().endsWith("tap"))
                return file()->shortName();
            else
                return file()->shortName() + QStringList({ "_TS", "_BS" })[file()->side()];
        }
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
            return tbStrList[file()->side()];
        case Qt::EditRole:
            return static_cast<bool>(file()->side());
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}
