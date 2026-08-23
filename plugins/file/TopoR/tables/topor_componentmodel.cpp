/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "topor_componentmodel.h"

namespace TopoR {

ComponentModel::ComponentModel(const Components& components, QObject* parent)
    : QAbstractTableModel{parent}
    , components_(components) {
}

int ComponentModel::rowCount(const QModelIndex& /*parent*/) const { return static_cast<int>(components_.size()); }

int ComponentModel::columnCount(const QModelIndex& /*parent*/) const { return ColumnCount; }

QVariant ComponentModel::data(const QModelIndex& index, int role) const {
    if(role != Qt::DisplayRole && role != Qt::TextAlignmentRole) return {};
    const Component& c = components_[index.row()];
    if(role == Qt::TextAlignmentRole)
        return index.column() == RefDes || index.column() == Footprint || index.column() == ComponentRef
            ? QVariant{Qt::AlignVCenter | Qt::AlignLeft}
            : QVariant{Qt::AlignCenter};
    switch(index.column()) {
    case RefDes      : return c.refDes;
    case Footprint    : return c.footprint;
    case ComponentRef : return c.componentRef;
    case Side         : return c.side == ::Top ? tr("Top") : tr("Bottom");
    case Angle        : return QString::number(c.angle, 'f', 1);
    case X            : return QString::number(c.pos.x(), 'f', 4);
    case Y            : return QString::number(c.pos.y(), 'f', 4);
    default           : return {};
    }
}

QVariant ComponentModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch(section) {
        case RefDes      : return tr("RefDes");
        case Footprint    : return tr("Footprint");
        case ComponentRef : return tr("Component");
        case Side         : return tr("Side");
        case Angle        : return tr("Angle");
        case X            : return tr("X");
        case Y            : return tr("Y");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

} // namespace TopoR

#include "moc_topor_componentmodel.cpp"
