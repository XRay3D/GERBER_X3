// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gc_thermalmodel.h"
#include "gc_thermalnode.h"

#include <QIcon>

QIcon ThermalModel::repaint(QColor color, const QIcon& icon) const {
    QImage image(icon.pixmap(24, 24).toImage());
    for (int x = 0; x < 24; ++x)
        for (int y = 0; y < 24; ++y) {
            color.setAlpha(image.pixelColor(x, y).alpha());
            image.setPixelColor(x, y, color);
        }
    return QIcon(QPixmap::fromImage(image));
}

ThermalModel::ThermalModel(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new ThermalNode(this)) {
}

ThermalModel::~ThermalModel() { delete rootItem; }

ThermalNode* ThermalModel::appendRow(const QIcon& icon, const QString& name, const ThParam& par) {
    m_data.push_back(new ThermalNode(icon, name, par, this));
    rootItem->append(m_data.back());
    return m_data.back();
}

int ThermalModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0)
        return 0;
    return getItem(parent)->childCount();
}

int ThermalModel::columnCount(const QModelIndex& /*parent*/) const { return 5; }

QModelIndex ThermalModel::index(int row, int column, const QModelIndex& parent) const {
    ThermalNode* childItem = getItem(parent)->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex ThermalModel::parent(const QModelIndex& index) const {

    if (!index.isValid())
        return QModelIndex();

    ThermalNode* parentItem = getItem(index)->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant ThermalModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();
    ThermalNode* item = getItem(index);
    return item->data(index, role);
}

bool ThermalModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid())
        return false;
    const bool result = getItem(index)->setData(index, value, role);
    return result;
}

QVariant ThermalModel::headerData(int section, Qt::Orientation orientation, int role) const {
    static const QStringList horizontalLabel {tr("     Name|Pos (X:Y)|Angle|Tickness|Count").split('|')};
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal)
            return horizontalLabel[section];
        else
            return section + 1;
    default:
        return QVariant();
    }
}

Qt::ItemFlags ThermalModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    ThermalNode* item = getItem(index);
    return item->flags(index);
}

bool ThermalModel::removeRows(int row, int count, const QModelIndex& parent) {
    ThermalNode* item = nullptr;
    if (parent.isValid())
        item = static_cast<ThermalNode*>(parent.internalPointer());
    else
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    while (count--)
        item->remove(row);
    endRemoveRows();
    resetInternalData();
    return true;
}

ThParam ThermalModel::thParam() { return m_data.front()->getPar(); }

ThermalNode* ThermalModel::getItem(const QModelIndex& index) const {
    if (index.isValid()) {
        auto* item = static_cast<ThermalNode*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

#include "moc_gc_thermalmodel.cpp"
